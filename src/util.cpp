#include "util.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <boost/filesystem.hpp>		//directory creation
#include <boost/algorithm/string.hpp>
#include <openssl/md5.h>			//OPENSSL needed for MD5 support
#include <time.h>       /* time_t, struct tm, time, localtime, strftime */
	

boost::mutex crunch::Util::m_directoryMutex;
boost::mutex crunch::Util::m_logfileMutex;
std::string crunch::Util::m_logFilePath("");
#ifdef __DEBUG__
unsigned short crunch::Util::m_logLevelFile = logTRACE;
unsigned short crunch::Util::m_logLevelDisplay = logTRACE;
#else
unsigned short crunch::Util::m_logLevelFile = logINFO;
unsigned short crunch::Util::m_logLevelDisplay = logERROR;
#endif
std::ofstream crunch::Util::m_filePointer;

char crunch::Util::curlErrorBuffer[CURL_ERROR_SIZE] = "\0";
 const char crunch::Util::stringReplacement[][5] = 
#include "charconversion.txt"

/**
* Computes MD5 hash value using the openssl library
* returns an HEX-formatted string
*/
void crunch::Util::md5(const std::string& value, char* outbuffer)
{
	unsigned char result[MD5_DIGEST_LENGTH];
	MD5((unsigned char*) value.c_str(), value.length(), result);
	for(unsigned short i=0; i <MD5_DIGEST_LENGTH; i++)
		sprintf(outbuffer+2*i,"%02x",result[i]);
}

bool  crunch::Util::BothAreSpaces(char lhs, char rhs) 
{ return (lhs == rhs) && (lhs == ' '); }


void crunch::Util::cleanString(std::string& source)
{
	unsigned int len = source.length();

	bool pb =false;
	
	for(unsigned int c=0; c<len; c++)
	{ 
		char testcar = source[c];
		//normal uppercase char, ok let's convert to lowercase
		if((testcar >='A') && (testcar <='Z'))
		{
			source.replace(c, 1, 1, testcar + 32);
			continue;
		}
		//normal char, continue
		if(  ((testcar >='a') && (testcar <='z')) || ((testcar >='0') && (testcar <='9')) || (testcar == ' '))
			continue;

		bool char_handled = false;
		//strange car. what to do with it ?
		for(unsigned int index = 0; index < CHAR_CONVERSION_NUMBER; index++)
		{	
			if(testcar == stringReplacement [index][2]) //probably char to replace
			{
				if(stringReplacement [index][3] == '\0') //not extended UTF8 char
					{
						source.replace(c, 1, 1, stringReplacement [index][0]);
						char_handled = true;
						break;
					}
				else //utf8 car
				{
					if(source[c+1] == stringReplacement[index][3] ) //a UTF8 (2bytes) char to replace
					{
						if(stringReplacement [index][1] != '\0')
							//2 cars in replacement (ex æ -> a & e)
							{
								source.replace(c, 1, 1, stringReplacement [index][0]);
								source.replace(c+1, 1, 1, stringReplacement [index][1]);
							}
						else // 1 car only to replace an UTF8 char on 2 bytes. Second car has to be removed
						{
							source.replace(c, 2,1, stringReplacement [index][0]);
							len = source.length();
						}
						char_handled = true;
						break;
					}
				}
			}
		}

		if(char_handled) continue;
		//still here ? remove the char
		if((source[c]&0xC0)==0xC0) //first UtF8 sequence. erase !
		{
			source.erase(c,1);
			if((source[c]&0xC0)==0x80)
				source.erase(c,1); //second UTF8 car.
			if((source[c]&0xC0)==0x80)
				source.erase(c,1); //third UTF8 car.
			if((source[c]&0xC0)==0x80)
				source.erase(c,1); //fourth UTF8 car.
			len = source.length();
			c--; //car has been deleted, next caracter to study is now pointed by c. Since the loop will c++ --> (c--)++ = c !
		}
		else
		{
			source.erase(c,1);
			len = source.length();
			c--;
		}
	}

	std::string::iterator new_end = std::unique(source.begin(), source.end(), BothAreSpaces);
	source.erase(new_end, source.end());


}

void crunch::Util::ReplaceStringInPlace(std::string& subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, utf8size(search), replace);
         pos += replace.length();
    }
}

/**
* return a string size, with UTF8 cars taken into account (utf8size("tré") = 3 not 4  )
*
*/
unsigned int crunch::Util::utf8size(const std::string &str)
{
	unsigned int len = 0;
	const char* s = str.c_str();
	while (*s) { if(*s>154) return 0; len += (*s++ & 0xc0) != 0x80;}
	return len;
}

std::string crunch::Util::to_string(unsigned int i)
{
	std::ostringstream ss;
    ss << i;
    return ss.str();
}
/**
* try to open a directory. It it doesn't work, create it
* if recursive = true (dafault), create recursively all directories in the path string
* if recursive = false, only tries to create final directory, assuming the 
* beginning of the path already exists
* thread safe ; use m_directoryMutex
*/
void crunch::Util::selectOrCreateDirectory(const std::string& path, bool recursive)
{

	Util::log(std::string("Select or create DIR ")+path+std::string("\n"),logTRACE);
	boost::lock_guard<boost::mutex> guard(m_directoryMutex);

	const char* buffer=path.c_str();

	if(buffer==NULL)
		throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory Null string as param"));
	if(*buffer=='\0')
		throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory Null string as param"));
	if(*buffer!='/')
		throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory Not an absolute path ") +path);

	if(!recursive) //simple creation 
	{
		if(boost::filesystem::exists(path)) return;
		if(boost::filesystem::create_directory(path)) return;
		std::string errMsg = std::string("crunch::Util::selectOrCreateDirectory Error trying to create directory ")+path;
		Util::log(errMsg,logERROR);
		throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory Error trying to create directory ")+path);
	}
	
	//if here: recursive
	const char* nextCar=buffer; //start at beginning of string
	nextCar++; //first character is /. Let's get to next car.


	if(*nextCar=='/')
		throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory : cannot receive '/' as parameter"));
	
	
	while((nextCar!=NULL)&&(*nextCar!='\0'))
	{
		const char* tempCar=strchr(nextCar,'/');
		if(tempCar == NULL) 
			throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory : string must end with '/'"));
		if(tempCar==nextCar)
			throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory : cannot receive '//' in string"));
		
		std::string tempPath(buffer,tempCar-buffer);
		
		nextCar=tempCar+1;
		if(boost::filesystem::exists(tempPath))
		{
			if(!boost::filesystem::is_directory(tempPath))
				throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory : ")+ tempPath+std::string(" exists and is not a directory"));
		}
		else if(!boost::filesystem::create_directory(tempPath)) 
			throw std::runtime_error(std::string("crunch::Util::selectOrCreateDirectory : error trying to create ")+ tempPath);
	}
}

 void crunch::Util::log(std::string msg, unsigned short level )
{
	if( ( level > m_logLevelDisplay ) && ( level > m_logLevelFile ) )
		return;

	time_t rawtime;
  	struct tm * timeinfo;
  	char buffer [200];
  	time (&rawtime);
  	timeinfo = localtime (&rawtime);
  	strftime (buffer,80,"%H:%M:%S",timeinfo);
  	std::string nowStr(buffer);
	if( level <= m_logLevelDisplay )
		std::cout << nowStr << ( (level == logTRACE )?" TRACE " :((level == logINFO)? " INFO ":" ERROR "))
					<< msg << std::flush;
	if( level <= m_logLevelFile )
	{
		boost::lock_guard<boost::mutex> guard(m_logfileMutex);
		if(m_logFilePath.length() == 0) //log file not initialized yet
		{
			strftime (buffer,80,"%Y%m%d-%H%M%S",timeinfo);
  			std::string dateFileStr(buffer);
			m_logFilePath = std::string(LOGFILEPATH_PREFIX)+dateFileStr;
			m_filePointer.open(m_logFilePath.c_str(),std::ios::out | std::ios::app);
			if(!m_filePointer.is_open())
			{
				std::cout << "UNABLE TO CREATE LOG FILE " << m_logFilePath << ". EXITING\n";
				throw std::runtime_error(std::string("Unable to create log file"));
			}
		}
		m_filePointer <<  nowStr << ( (level == logTRACE )? " TRACE " :((level == logINFO)? " INFO ":" ERROR ")) << msg;
		m_filePointer.flush();
	}

}
using  namespace boost::filesystem;

bool crunch::Util::getDirectoryContent(const std::string pathStr, std::vector<std::string>& content)
{
	
	path _path(pathStr);
	if(!is_directory(_path)) return false;
	for (directory_iterator itr(_path); itr!=directory_iterator(); ++itr)
		content.push_back(itr->path().filename().string());
	return true;

}

bool crunch::Util::compareFiles(const std::string file1, const std::string file2)
{
  int N = 64000;
  char buf1[N];
  char buf2[N];
  FILE* f1=fopen(file1.c_str(),"r");
  if(f1 == NULL) return false;

  FILE* f2=fopen(file2.c_str(),"r");
  if(f2 == NULL) 
  	{	fclose(f1);
  		return false;
  	}

  do {
    size_t r1 = fread(buf1, 1, N, f1);
    size_t r2 = fread(buf2, 1, N, f2);
    if (r1 != r2 ||
        memcmp(buf1, buf2, r1)) {
     fclose(f1);fclose(f2);
 	  return false;
    }
  } while (!feof(f1) || !feof(f2));
 fclose(f1);fclose(f2);
  return true;
}

void crunch::Util::writeJsonFile(json_spirit::mObject &obj, const std::string path, const std::string filename)
{
   	crunch::Util::selectOrCreateDirectory(path,false);
   	std::string filepath = path + filename;
	FILE* f=fopen(filepath.c_str(),"w");
	if(!f)
	{
		std::string exc("Cannot open file for writing :");
		exc+=filepath;
		throw std::runtime_error(exc);
	}
	#ifdef __DEBUG__
		fputs(json_spirit::write(obj,json_spirit::raw_utf8|json_spirit::pretty_print).c_str(),f);
	#else
		fputs(json_spirit::write(obj,json_spirit::raw_utf8).c_str(),f);
	#endif
	fclose(f);
}

void crunch::Util::writeJsonFile(json_spirit::mArray &arr, const std::string path, const std::string filename)
{
   	crunch::Util::selectOrCreateDirectory(path,false);
   	std::string filepath = path + filename;
	FILE* f=fopen(filepath.c_str(),"w");
	if(!f)
	{
		std::string exc("Cannot open file for writing :");
		exc+=filepath;
		throw std::runtime_error(exc);
	}
	#ifdef __DEBUG__
		fputs(json_spirit::write(arr,json_spirit::raw_utf8|json_spirit::pretty_print).c_str(),f);
	#else
		fputs(json_spirit::write(arr,json_spirit::raw_utf8).c_str(),f);
	#endif
	fclose(f);
}

std::istream& crunch::Util::safeGetline(std::istream& is, std::string& t)
{
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf* sb = is.rdbuf();

    for(;;) {
        int c = sb->sbumpc();
        switch (c) {
        case '\n':
            return is;
        case '\r':
            if(sb->sgetc() == '\n')
                sb->sbumpc();
            return is;
        case EOF:
            // Also handle the case when the last line has no line ending
            if(t.empty())
                is.setstate(std::ios::eofbit);
            return is;
        default:
            t += (char)c;
        }
    }
}
