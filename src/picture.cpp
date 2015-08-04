#include "picture.hpp"
#include "util.hpp"
#include "json_spirit/json_spirit.h"
#include <boost/filesystem.hpp>
#include <Magick++.h>

using namespace crunch;

unsigned long Picture::m_counter=0;
unsigned long Picture::m_downloadCounter=0;
unsigned long Picture::m_doneCounter=0;
unsigned long Picture::m_errorCounter=0;

Picture::Picture()
{
	m_status=PENDING;
}

Picture::~Picture(){

}

std::pair<std::string, std::string> Picture::getFilePathFromUrl(const std::string& path, const std::string& fileprefix, const std::string &url, const unsigned short hashKeySize)
{
	std::string destFilename;
	//prepare filename
	if(!url.compare(0,8,"https://"))
		destFilename=url.substr(8,5000);
		else if(!url.compare(0,7,"http://"))
			destFilename=url.substr(7,5000);
			else if(!url.compare(0,7,"file://"))
				destFilename=url.substr(7,5000);
					else destFilename=url;

	std::replace(destFilename.begin(), destFilename.end(), '/', '-');
	
	//get MD5 rep
	std::string hash(m_idMd5,hashKeySize);
	std::string folderName(fileprefix);
	folderName+=hash;
	

	//create or select dir
	crunch::Util::selectOrCreateDirectory(path+folderName,false);

	return std::pair<std::string,std::string>(folderName, destFilename);
	

}

int Picture::downloadAndResize(	CURL* handle,
								bool transmitFlag,
								bool transmitThumbFlag,
								bool serverFlag, 	
								std::string transmitpath, 
							 	std::string serverpath,
							 	std::string photofileprefix,
							 	std::string thumbfileprefix,
							 	unsigned short hashKeySize,
							 	unsigned int thumbWidth,
							 	unsigned int thumbHeight,
							 	unsigned int maxWidth,
							 	unsigned int maxHeight
							 	)
{
	Util::log(std::string("Handling photo ")+m_URL+std::string("\n"),logTRACE);
	if(!m_URL.size())
	{
		m_status = ERROR;
		return 0;
	}

	if( (!transmitFlag) && (!serverFlag)) //nothing to do ????
		return 0;

	if(transmitFlag)
	{
		std::pair<std::string, std::string> pathElements = getFilePathFromUrl(transmitpath, photofileprefix, m_URL, hashKeySize);
		m_fileName = pathElements.second; 
		m_resource = pathElements.first + std::string("/");
		m_path = transmitpath+m_resource+m_fileName;
		crunch::Util::selectOrCreateDirectory(transmitpath+m_resource,false);
		
	}

	if(serverFlag)
	{
		std::pair<std::string, std::string> serverPathElements 	= getFilePathFromUrl(serverpath, photofileprefix, m_URL, hashKeySize);
		m_serverFileName = serverPathElements.second; 
		m_resource = serverPathElements.first + std::string("/");
		m_serverPath = serverpath+m_resource + m_serverFileName;
		crunch::Util::selectOrCreateDirectory(serverpath+m_resource,false);
		
	}
	
	
	if(serverFlag)
	{
		if(!download(m_URL,m_serverPath,handle))
			return 0;

		//resize ?
		if(maxWidth>0)
			resize(m_serverPath, maxWidth, maxHeight);

		//copy to transmit dir ?
		if(transmitFlag)
		{
			//delete file if it exists !
			boost::filesystem::wpath _wpath (m_path);
			if(boost::filesystem::exists(_wpath))
				boost::filesystem::remove(_wpath);
			boost::filesystem::copy_file(m_serverPath, m_path);
		}
		//thumbnail ?
		if(thumbWidth>0)
		{
			std::pair<std::string, std::string> thumbPathElements = getFilePathFromUrl(serverpath, thumbfileprefix, m_URL, hashKeySize);
			m_thumbnailResource = thumbPathElements.first + std::string("/");
			m_serverThumbnailFileName = thumbPathElements.second;
			std::string thumbpathServerFolder = serverpath+ m_thumbnailResource;
			std::string destThumbPath = thumbpathServerFolder + m_serverThumbnailFileName;
			crunch::Util::selectOrCreateDirectory(thumbpathServerFolder,false);
			
			createThumbnail(m_serverPath,thumbWidth, thumbHeight,destThumbPath);

			//copy thumbnail to transmit dir ?
			if(transmitThumbFlag)
			{
				std::string thumbpathTransmitFolder = transmitpath + m_thumbnailResource ;
				crunch::Util::selectOrCreateDirectory(thumbpathTransmitFolder,false);
				//delete file if it exists !

				boost::filesystem::wpath _wpath ( thumbpathTransmitFolder +  m_serverThumbnailFileName);
				if(boost::filesystem::exists(_wpath))
					boost::filesystem::remove(_wpath);
				boost::filesystem::copy_file(destThumbPath, thumbpathTransmitFolder +  m_serverThumbnailFileName);

			}
		}

		return 1;
	}

	//if still here, no server storage. download, resize & pictures has to be done in transmit folder directly

	if(!download(m_URL,m_path,handle))
		return 0;

	//resize ?
	if(maxWidth>0)
		resize(m_path, maxWidth, maxHeight);

	//thumbnail ?
	if(thumbWidth>0)
	{
		std::pair<std::string, std::string> thumbPathElements = getFilePathFromUrl(transmitpath, thumbfileprefix, m_URL, hashKeySize);
		m_thumbnailFileName = thumbPathElements.second;
		m_thumbnailResource = thumbPathElements.first + std::string("/");
		std::string thumbpathTransmitFolder = transmitpath + m_thumbnailResource ;
		std::string destThumbPath = thumbpathTransmitFolder + m_thumbnailFileName;
		crunch::Util::selectOrCreateDirectory(thumbpathTransmitFolder,false);
		createThumbnail(m_path,thumbWidth, thumbHeight,destThumbPath);
	}

	return 1;

}


int Picture::download(std::string url, std::string filepath, CURL* handle)
{

	Util::log(std::string("DL photo ")+url+std::string("\n"),logTRACE);

	//if URL starts with file://, it's not a download, it's a copy.
	if( !url.compare(0,7,"file://"))
	{
		//file local
		std::string sourcepath(url,7,std::string::npos);
		boost::filesystem::wpath _wsourcepath (sourcepath);
		if(boost::filesystem::exists(_wsourcepath))
		{
			Util::log(std::string("Copying photo ")+url+std::string("\n"),logTRACE);

			boost::filesystem::wpath _wfilepath(filepath);
			
			if(boost::filesystem::exists(_wfilepath))
				boost::filesystem::remove(_wfilepath);
			boost::filesystem::copy_file(sourcepath, _wfilepath);
			m_status = DONE;

			Util::log("Copying done\n",logTRACE);
			return 1;
		}
		m_status = ERROR;
		return 0;
	}

	//still here ? real download !
	
	FILE* file = fopen(filepath.c_str(), "w");

	if(!file)  {
		Util::log(std::string("Error trying to write to  ")+filepath+std::string("\n"),logERROR);		
		m_status = ERROR;
		return 0;
	}


	curl_easy_setopt(handle, CURLOPT_WRITEDATA, file) ;
	curl_easy_setopt(handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(handle, CURLOPT_FAILONERROR, 1);
	CURLcode code = curl_easy_perform( handle );
	fclose(file);

	if(code == CURLE_OK) {
		m_status = DONE;
		return 1;
	}

	//download error handling
	Util::log(std::string("Download error Curl Code ")
				+std::string(curl_easy_strerror(code))+std::string(" for URL ")
				+m_URL+std::string("\n"),logTRACE);

	m_status = ERROR;

	//delete file if it exists !
	boost::filesystem::wpath _wpath (filepath);
	if(boost::filesystem::exists(_wpath))
		boost::filesystem::remove(_wpath);

	return 0;

}

int Picture::createThumbnail(const std::string& sourcepath,
							int width,
							int height, 
							const std::string& destpath)
{
	Magick::Image thumb(sourcepath); 
	Magick::Geometry newSize(width, height, 0, 0); 
	thumb.resize(newSize);
	thumb.write(destpath);
	return 1;
}

int Picture::resize(const std::string& path, int width, int height)
{
	Magick::Image thumb(path); 
	Magick::Geometry newSize(width, height, 0, 0); 
	thumb.resize(newSize);
	thumb.write(path);
	return 1;
}

json_spirit::mObject  Picture::outputJson( const std::string* server_url, int crunchVersion)
{
	json_spirit::mObject picObj;

	if(m_fileName.size()) //we have a downloaded file transmitted on the device
	{
		picObj[ "url"] =  m_resource + m_fileName ;
	}
	else  if(m_serverFileName.size()) //we have a downloaded file that stays on the server
	{
		picObj[ "url"]= *server_url + m_resource + m_serverFileName;
	}
	else picObj["url"]= m_URL;
	
	if(m_thumbnailFileName.size())
	{
		picObj["thumb"]= m_thumbnailResource + m_thumbnailFileName;
	}
	else if(m_serverThumbnailFileName.size())
	{
		picObj["thumb"]= *server_url + m_thumbnailResource + m_serverThumbnailFileName ;
	}
	return picObj;
}
