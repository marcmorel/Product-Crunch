#ifndef __CRUNCH_CONFIG_HPP
#define __CRUNCH_CONFIG_HPP

#include "json_spirit/json_spirit.h"
#include "boost/unordered_map.hpp"
#include <vector>
#include <sys/time.h>


#define DEFAULT_TMP_ROOT "/tmp/crunch/"


/**
* CrunchConfig is a singleton used in the crunch class to store parameters from the config file 
*/
namespace crunch {

	typedef enum { cdata, element, element_array } CSVfieldType;
	typedef enum { xml, csv } SourceFormat;
	 typedef struct 
	 {
	 	//CSVfieldType m_type;
	 	std::string m_value;
	 	std::vector<std::string> m_arrayValues;
	 } CSVfield;

class Searcher;

class CrunchConfig
{
public:
	SourceFormat	m_format; // csv or xml

	std::string m_source_file;			//url or path of source file
	std::string m_source_login;			//optional login for download
	std::string m_source_password;		//optional password for download
	unsigned int m_version;				//crunch version to output

	std::string m_tmp_directory;		//working directory
	std::string m_directory;			//output directory
	std::string m_server_directory;		//server output directory
	std::string m_server_url;			//server url to access catalog API 
	std::string m_diff_directory;       //previous crunch execution directory, for diff calc		
	std::string m_status_json_file;		//path to JSON status file

	int m_photo_hashkey; 				//number of characters used to group pictures by folders ( 0-> 1 folder, 1 -> 16 folder, 2->255 folders, ...)
	int m_file_hashkey; 				//number of characters used to group prd/mod by FILES
	int m_search_hashkey;				//number of characters used to group search files by FILES 
	int m_resource_hashkey;				//number of characters used to group prd/mod FILES  by folders 
    int m_external_file_hashkey;
    int iMaxItemsToRead;                //max items to read from the xml or csv feed use it to make a preview.
	bool m_generate_model_file;			//flag to generate or not model files
	bool m_generate_product_file;		//flag to generate or not product files
	bool m_generate_category_file;		//flag to generate or not category files
    bool m_product_listed_in_category;	

    bool m_download_files;			//if false, original files URL is kept in json files and pictures are not downloaded
	bool m_transmit_files; 

    bool m_transmit_pictures;
    bool m_transmit_thumbnails;
    bool m_transmit_catalog;
    bool m_category_thumbnail;
	bool m_download_pictures;			//if false, original pictures URL is kept in json files and pictures are not downloaded
	bool m_resize_pictures;				// resize pictures to limit size
	int m_max_height;
	int m_max_width;
	bool m_create_thumbnails;			// resize pictures to create thumbnails
	int m_thumbnail_width;
	int m_thumbnail_height;
	std::string m_category_separator;

	std::vector <std::string> m_enumFields; //list of fields whom value should be counted (unique)
	std::vector <std::string> m_modelCustomFields;
	std::vector <std::string> m_productCustomFields;
	std::vector <std::string> m_referenceCustomFields;
	std::vector <std::string> m_modelDownloadFields; //list of fields which are files to download.
	std::vector <std::string> m_productDownloadFields; //list of fields which are files to download.

	std::vector <Searcher*> m_searchers;
	boost::unordered_map <std::string, std::string> m_fieldsNameReplacement;
	boost::unordered_map <std::string, std::string>m_fieldsRegexp;

	boost::unordered_map <std::string, json_spirit::mObject> m_categoryParameter;

	boost::unordered_map <std::string, CSVfield > m_CSVfields; //used only for CSV files. describes how to construct XML items from CSV fields
	char m_CSVdelimiter;
	char m_CSVenclosure;

    //boolean to stop crunch and operate the equivalent of --verify-config command parameter
    bool bCmdVerifyConfig;


	void parseConfigFile(const std::string& jsonparam);


	/**
	* get specific JSON value from config file
	*/
	std::string getJSONstring(	const char* name,
								json_spirit::mObject& object, bool mandatory = true, std::string def = std::string(""));

	bool getJSONbool(	const char* name,
						json_spirit::mObject& object , bool mandatory = true, bool def = true);

	int getJSONint(	const char* name,
					json_spirit::mObject& object, bool mandatory = true, int def = 0);

	void getJSONarray(	const char* name,
						json_spirit::mObject& object,
						std::vector <std::string>& vector, bool mandatory = true);

	void getJSONobject(const char* name,  json_spirit::mObject& object, boost::unordered_map <std::string, std::string>& map, bool mandatory = true );

	void getJSONunparsedObject(const char* name,  json_spirit::mObject& object, boost::unordered_map <std::string, json_spirit::mObject>& map, bool mandatory = true );

	CrunchConfig()
	{
		m_format            = xml;
		m_version           = 2;
		m_photo_hashkey     = -1; //automatic guess 
		m_file_hashkey      = -1; //automatic guess 
		m_resource_hashkey  = -1; //automatic guess
		m_search_hashkey    = -1; //automatic guess
		m_external_file_hashkey	= -1;
        iMaxItemsToRead     = -1; //no limit
		m_download_pictures = true;
		m_resize_pictures   = false;
		m_create_thumbnails = false;
		m_thumbnail_height	= 100;
		m_thumbnail_width	= 100;
		m_max_height		= 600;
		m_max_width			= 600;

		std::ostringstream tmp_random;

        tmp_random << "/tmp/crunch_" << time(NULL) << "_" << rand << "/";

        m_tmp_directory = std::string(tmp_random.str());

		m_generate_model_file    = true;
		m_generate_product_file  = true;
		m_generate_category_file = true;

		m_category_separator         = std::string("&gt;");
		m_source_login               = std::string("");
		m_source_password            = std::string("");
		m_server_directory			 = std::string("");
		m_diff_directory			 = std::string("");
		m_status_json_file			 = std::string("");
		m_server_url				 = std::string("");
		m_CSVdelimiter               = ';';
		m_CSVenclosure               = '"';
		m_product_listed_in_category = false;
        bCmdVerifyConfig             = false;
        m_transmit_pictures			 = true;
        m_transmit_thumbnails		 = true;
        m_transmit_catalog			 = true;
        m_category_thumbnail		 = true;
        m_transmit_files			 = true;
        m_download_files			 = true;
	}

    ~CrunchConfig();
};


class CrunchWrongKeyException : public std::runtime_error
{
	public:
		CrunchWrongKeyException(const std::string& str);
};
}

#endif