#include "reader.hpp"
#include <curl/curl.h>				//picture download
#include "util.hpp"
#include <boost/filesystem.hpp>		//check if file exists


crunch::Reader::~Reader(){
/*delete m_crunchConfig;
*/
 #ifdef __DEBUG__
std::cout << " Reader destructor\n";
#endif
}

crunch::Reader::Reader(crunch::CrunchConfig* c)
{
	m_crunchConfig=c;

}
/** 
* locate source file 
*
*/
bool crunch::Reader::locateSourceFile()
{

	if( (!m_crunchConfig->m_source_file.compare(0,7,"http://"))
		||
		(!m_crunchConfig->m_source_file.compare(0,8,"https://")) )
		return downloadSourceFile();
	Util::log("Protocole non reconnu dans l'URL "+m_crunchConfig->m_source_file +std::string("\n"), logTRACE); 
    Util::log("Recherche dans le filesystem local\n", logTRACE); 
	m_sourceFile = m_crunchConfig->m_source_file;
	if(boost::filesystem::exists(m_sourceFile)) return true;

	Util::log("Fichier non trouvé dans le filesystem local\n", logTRACE); 
	
	return false;
}

/**
* download source file
*/
bool crunch::Reader::downloadSourceFile()
{
	//curl_global_init(CURL_GLOBAL_ALL );
	Util::log("Initialisation de curl\n", logTRACE); 
	CURL* easyhandle = curl_easy_init();
	curl_easy_setopt(easyhandle, CURLOPT_ERRORBUFFER, Util::curlErrorBuffer);
	m_sourceFile=m_crunchConfig->m_tmp_directory+"source";

	//create or select dir
	crunch::Util::selectOrCreateDirectory(m_crunchConfig->m_tmp_directory);	
	FILE* file = fopen(m_sourceFile.c_str(), "w");
	if(!file)
	{
		Util::log("Erreur d'ouverture pour écriture de "+m_sourceFile +std::string("\n"), logERROR); 
    
		return false;
	}
	curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, file) ;
	std::string url=m_crunchConfig->m_source_file;
	if(m_crunchConfig->m_source_password.length())
	{
		unsigned short protocol_size=0;
		if(!url.compare(0,8,"https://")) protocol_size=8;
		else if(!url.compare(0,7,"http://")) protocol_size=7;
			else throw std::runtime_error("Wrong protocol in source URL");	
		url=url.substr(0,protocol_size)
			+m_crunchConfig->m_source_login+std::string(":")
			+m_crunchConfig->m_source_password+std::string("@")+url.substr(protocol_size,std::string::npos);
	}
	Util::log("Paramétrage de curl avec l'URL  "+url +std::string("\n"), logTRACE); 
    
	curl_easy_setopt(easyhandle, CURLOPT_URL, url.c_str());
	if(!curl_easy_perform( easyhandle ))
		{
			fclose(file);
			curl_easy_cleanup(easyhandle);
            return true;
		}
         
	fclose(file);
	std::string errorMsg(Util::curlErrorBuffer);
	Util::log("Erreur CURL "+errorMsg +std::string("\n"), logERROR); 
    
    curl_easy_cleanup(easyhandle);
	return false;
	}

