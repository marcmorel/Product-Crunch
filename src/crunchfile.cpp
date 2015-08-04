#include "crunchfile.hpp"
#include "util.hpp"
#include "json_spirit/json_spirit.h"
#include <boost/filesystem.hpp>

using namespace crunch;

unsigned long CrunchFile::m_counter=0;
unsigned long CrunchFile::m_downloadCounter=0;
unsigned long CrunchFile::m_doneCounter=0;
unsigned long CrunchFile::m_errorCounter=0;

CrunchFile::CrunchFile()
{
	m_status=PENDING;
}

CrunchFile::~CrunchFile(){

}

std::pair<std::string, std::string> CrunchFile::getFilePathFromUrl(const std::string& path, const std::string& fileprefix, const std::string &url, const unsigned short hashKeySize)
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

int CrunchFile::downloadFile(	CURL* handle,
								bool transmitFlag,
								bool serverFlag, 	
								std::string transmitpath, 
							 	std::string serverpath,
							 	std::string fileprefix,
							 	unsigned short hashKeySize
							 	)
{
	Util::log(std::string("Handling file ")+m_URL+std::string("\n"),logTRACE);
	if(!m_URL.size())
	{
		m_status = ERROR;
		return 0;
	}

	if( (!transmitFlag) && (!serverFlag)) //nothing to do ????
		return 0;

	if(transmitFlag) // create local directory
	{
		std::pair<std::string, std::string> pathElements = getFilePathFromUrl(transmitpath, fileprefix, m_URL, hashKeySize);
		m_fileName = pathElements.second; 
		m_resource = pathElements.first + std::string("/");
		m_path = transmitpath+m_resource+m_fileName;
		crunch::Util::selectOrCreateDirectory(transmitpath+m_resource,false);
		
	}

	if(serverFlag) // create directory for server hosting
	{
		std::pair<std::string, std::string> serverPathElements 	= getFilePathFromUrl(serverpath, fileprefix, m_URL, hashKeySize);
		m_serverFileName = serverPathElements.second; 
		m_resource = serverPathElements.first + std::string("/");
		m_serverPath = serverpath+m_resource + m_serverFileName;
		crunch::Util::selectOrCreateDirectory(serverpath+m_resource,false);
		
	}
	
	
	if(serverFlag)
	{
		if(!download(m_URL,m_serverPath,handle))
			return 0;

		//copy to transmit dir ?
		if(transmitFlag)
		{
			//delete file if it exists !
			boost::filesystem::wpath _wpath (m_path);
			if(boost::filesystem::exists(_wpath))
				boost::filesystem::remove(_wpath);
			boost::filesystem::copy_file(m_serverPath, m_path);
		}
		

		return 1;
	}

	//if still here, no server storage. download,  & CrunchFiles has to be done in transmit folder directly

	if(!download(m_URL,m_path,handle))
		return 0;


	return 1;

}


int CrunchFile::download(std::string url, std::string filepath, CURL* handle)
{

	Util::log(std::string("DL file ")+url+std::string(" to path ")+filepath+std::string("\n"),logTRACE);

	//if URL starts with file://, it's not a download, it's a copy.
	if( !url.compare(0,7,"file://"))
	{
		//file local
		std::string sourcepath(url,7,std::string::npos);
		boost::filesystem::wpath _wsourcepath (sourcepath);
		if(boost::filesystem::exists(_wsourcepath))
		{
			Util::log(std::string("Copying file ")+url+std::string("\n"),logTRACE);

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
		Util::log(std::string("DL file OK"),logTRACE);
		m_status = DONE;
		return 1;
	}

	//download error handling
	Util::log(std::string("Download error Curl Code ")
				+std::string(curl_easy_strerror(code))+std::string(" for URL ")
				+m_URL+std::string("[END OF URL]\n"),logERROR);
	Util::log(std::string("additionnal info ")
				+std::string(Util::curlErrorBuffer)+std::string(" for URL ")
				+m_URL+std::string("[END OF URL]\n"),logERROR);

	m_status = ERROR;

	//delete file if it exists !
	boost::filesystem::wpath _wpath (filepath);
	if(boost::filesystem::exists(_wpath))
		boost::filesystem::remove(_wpath);

	return 0;

}



json_spirit::mObject  CrunchFile::outputJson( const std::string* server_url, int crunchVersion)
{
	json_spirit::mObject fileObj;

	if(m_fileName.size()) //we have a downloaded file transmitted on the device
	{
		fileObj[ "url"] =  m_resource + m_fileName ;
		if(m_status == DONE ) {
			fileObj[ "status"] = "ok";
		}
		else {
			fileObj[ "status"] = "error";
		}
	}
	else  if(m_serverFileName.size()) //we have a downloaded file that stays on the server
	{
		fileObj[ "url"]= *server_url + m_resource + m_serverFileName;
	}
	else fileObj["url"]= m_URL;
	
	return fileObj;
}
