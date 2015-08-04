#ifndef __PICTURE_HPP
#define __PICTURE_HPP
#include <string>
#include <iostream>
#include <fstream>
#include <curl/curl.h>				//picture download
#include "json_spirit/json_spirit.h"
#include "util.hpp"

/**
* @author Marc Morel
* @copyright Procheo, 2014
*/


/**
* the class Picture handles picture download and JSON output of picture information.
*
**/


namespace crunch
{



	/** 
	* represents a picture file
	* one class instance is being used in the crunch class per different picture URL.
	* each instance is then pointed at from all model/product/reference where it is being used.
	*/
	class Picture
	{

		public:
			char m_idMd5[33];
			DownloadStatus m_status;
			bool m_mainPicture;
			static unsigned long m_counter;
			static unsigned long m_downloadCounter;
			static unsigned long m_errorCounter;
			static unsigned long m_doneCounter;
			std::string m_URL;
			
			//original file
			std::string m_path;				//at least, one these two paths has length > 0
			std::string m_serverPath; 				
		
			std::string m_fileName;			//at least, one of these two filename has length > 0
			std::string m_serverFileName;	//

			std::string m_resource;
			//thumbnail
			std::string m_thumbnailResource;
			std::string m_thumbnailFileName;
			std::string m_serverThumbnailFileName;

		Picture(); //Default constructor
        ~Picture(); //destructor
		json_spirit::mObject outputJson( const std::string* server_url = NULL, int crunchVersion = -1);
		std::pair<std::string, std::string> getFilePathFromUrl(const std::string& path, const std::string& fileprefix, const std::string &url, const unsigned short hashKeySize);
		int downloadAndResize(CURL* handle,
								bool transmitFlag,
								bool transmitThumbFlag, 
								bool serverFlag, 	
								std::string path, 
							 	std::string serverpath,
							 	std::string photofileprefix,
							 	std::string thumbfileprefix,
							 	unsigned short hashKeySize,
							 	unsigned int thumbWidth,
							 	unsigned int thumbHeight,
							 	unsigned int maxWidth,
							 	unsigned int maxHeight);
		int download(std::string url, std::string filepath, CURL* handle);

		int createThumbnail(const std::string& sourcepath,
							int width,
							int height, 
							const std::string& destpath);
		int resize(const std::string& path, int width, int height);
	};
}
#endif