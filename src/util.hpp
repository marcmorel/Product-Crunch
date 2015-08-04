#ifndef __CRUNCH_UTIL_HPP
#define __CRUNCH_UTIL_HPP

#include <boost/thread.hpp>
#include <boost/filesystem.hpp>
#include <string>
#include "json_spirit/json_spirit.h"
#include <curl/curl.h>	

/**
* @author Marc Morel
* @copyright Procheo, 2014
*/


/**
* the Util Class contains static miscelleanous methods
*
**/
#include <iostream>
#include <fstream>

#define LOGFILEPATH_PREFIX "/var/log/crunch/log"
#define logERROR	0
#define logINFO		1
#define logTRACE 	2

namespace crunch
{

	/**
	* this enum lists all picture downloading states
	*/
	enum DownloadStatus { PENDING, DONE, ERROR };

	class Util
	{
		static const char stringReplacement[][5];
		static boost::mutex m_directoryMutex;
		static boost::mutex m_logfileMutex;
		
		static std::string m_logFilePath;

		static unsigned short m_logLevelFile;
		static unsigned short m_logLevelDisplay;
		static std::ofstream m_filePointer;

		public:
		static char  curlErrorBuffer[CURL_ERROR_SIZE];
		
		static unsigned int utf8size(const std::string& str);
		static void ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace);
		static void cleanString(std::string &s);
		
		/* compute MD5 hash. CAUTION : outbuffer allocation is not handled by this method. */
		static void md5(const std::string& value, char* outbuffer);

		/* check existence of or create directory. If recursive is true, will create as many directories as need 
		* following the path depth
		*/
		static void selectOrCreateDirectory(const std::string& path, bool recursive=true);

		static void log(std::string msg, unsigned short level = logERROR);
		static bool BothAreSpaces(char lhs, char rhs);
		static std::string to_string(unsigned int i);
		static bool getDirectoryContent(const std::string pathStr, std::vector<std::string>& content);
		static bool compareFiles(const std::string file1, const std::string file2);

		static void writeJsonFile(json_spirit::mObject &obj, const std::string path, const std::string filename);
		static void writeJsonFile(json_spirit::mArray &arr, const std::string path, const std::string filename);
		static std::istream& safeGetline(std::istream& is, std::string& t);

	};
}

#endif