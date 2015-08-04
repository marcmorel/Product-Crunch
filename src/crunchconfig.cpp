#include "crunchconfig.hpp"
#include <iostream>
#include <fstream>
#include "searcher.hpp"

crunch::CrunchConfig::~CrunchConfig(){
m_modelCustomFields.clear();
m_productCustomFields.clear();
m_referenceCustomFields.clear();
m_searchers.clear();
m_fieldsNameReplacement.clear();
m_fieldsRegexp.clear();
m_CSVfields.erase(m_CSVfields.begin(),m_CSVfields.end());
}

/**
* Read JSON config file indicated by jsonparam and 
* prepare the crunch class, by retrieving all parameter values.
*/
void crunch::CrunchConfig::parseConfigFile(const std::string& jsonparam)
{
	//opening config file
	std::ifstream is( jsonparam.c_str());

	//parsing config file
	json_spirit::mValue value;
	try{
		json_spirit::read_or_throw( is, value );
	}
	catch(const json_spirit::Error_position& pos) { 
		std::cout << "error " << pos.line_ << " " << pos.column_ << " " << pos.reason_ << "\n";	
		throw std::logic_error("Bad json format ");
	}
	json_spirit::mObject jsonObject = value.get_obj();

	//two mandatory parameters
	m_source_file = getJSONstring("source-file",jsonObject );
	m_directory   = getJSONstring("directory",jsonObject );

    //command line equivalent call from the json conf
    bCmdVerifyConfig = getJSONbool("verify-config",jsonObject,false, bCmdVerifyConfig );

	//all these parameters are optional. Let's catch the exception when they aren't found
	m_version                    = getJSONint    ("crunch-version"         ,jsonObject,false, m_version ); 	
	m_file_hashkey               = getJSONint    ("file-hashkey"           ,jsonObject,false, m_file_hashkey); 	
	m_resource_hashkey           = getJSONint    ("resource-hashkey"       ,jsonObject,false, m_resource_hashkey ); 	
	m_photo_hashkey              = getJSONint    ("photo-hashkey"          ,jsonObject,false, m_photo_hashkey ); 	
	m_external_file_hashkey      = getJSONint    ("external-file-hashkey"   ,jsonObject,false, m_external_file_hashkey ); 	
	m_search_hashkey             = getJSONint    ("search-hashkey"         ,jsonObject,false , m_search_hashkey);	
	iMaxItemsToRead              = getJSONint    ("max-item-to-read"       ,jsonObject,false , iMaxItemsToRead);  
    
	m_download_files			 =getJSONbool   ("download-files"      ,jsonObject,false, m_download_files );
	m_transmit_files			 = getJSONbool   ("transmit-files"      ,jsonObject,false, m_transmit_files );
	

    m_transmit_pictures          = getJSONbool   ("transmit-pictures"      ,jsonObject,false, m_transmit_pictures );
	m_transmit_thumbnails		 = getJSONbool   ("transmit-thumbnails"    ,jsonObject,false, m_transmit_thumbnails );
	m_transmit_catalog			 = getJSONbool   ("transmit-catalog"       ,jsonObject,false, m_transmit_catalog );
    m_download_pictures          = getJSONbool   ("download-pictures"      ,jsonObject,false, m_download_pictures );
    m_resize_pictures			 = getJSONbool   ("resize-pictures"        ,jsonObject,false, m_resize_pictures );
    m_max_width			 		 = getJSONint    ("max-width"       	   ,jsonObject,false, m_max_width );
 	m_max_height				 = getJSONint    ("max-height"       	   ,jsonObject,false, m_max_height );
    m_create_thumbnails			 = getJSONbool   ("create-thumbnail"       ,jsonObject,false, m_create_thumbnails );
    m_thumbnail_width			 = getJSONint    ("thumbnail-width"        ,jsonObject,false, m_thumbnail_width );
 	m_thumbnail_height			 = getJSONint    ("thumbnail-height"       ,jsonObject,false, m_thumbnail_height );
 	m_category_thumbnail      	 = getJSONbool   ("category-thumbnail"     ,jsonObject,false, m_category_thumbnail);	
	
	m_generate_model_file        = getJSONbool   ("generate-model-file"    ,jsonObject,false, m_generate_model_file );	
    m_generate_product_file      = getJSONbool   ("generate-product-file"  ,jsonObject,false, m_generate_product_file );
	m_generate_category_file     = getJSONbool   ("generate-category-file" ,jsonObject,false, m_generate_category_file );	
	m_category_separator         = getJSONstring ("category-separator"     ,jsonObject,false, m_category_separator);	
	m_source_login               = getJSONstring ("source-login"           ,jsonObject,false, m_source_login );	
	m_source_password            = getJSONstring ("source-password"        ,jsonObject,false, m_source_password );	
	m_product_listed_in_category = getJSONbool   ("product-in-category"    ,jsonObject,false, m_product_listed_in_category );	
	m_server_directory	         = getJSONstring ("server-directory"       ,jsonObject,false, m_server_directory);	
	m_diff_directory			 = getJSONstring ("diff-directory"       	,jsonObject,false, m_diff_directory);	
	m_status_json_file		     = getJSONstring ("status-json-file"       ,jsonObject,false, m_status_json_file);	
	m_server_url      			 = getJSONstring ("server-url"       		,jsonObject,false, m_server_url);	
	
	//array parameters
	getJSONarray ("enum-fields-value"  	   ,jsonObject,m_enumFields, false); 	
	getJSONarray ("custom-product-fields"  ,jsonObject,m_productCustomFields, false); 	
	getJSONarray ("custom-model-fields"    ,jsonObject,m_modelCustomFields, false);	
	getJSONarray ("custom-reference-fields",jsonObject,m_referenceCustomFields, false);	
	getJSONobject("fields-name-replacement",jsonObject,m_fieldsNameReplacement, false);	
	getJSONobject("fields-regexp"          ,jsonObject,m_fieldsRegexp, false);	
	getJSONarray ("download-model-fields"  ,jsonObject,m_modelDownloadFields, false); 	
	getJSONarray ("download-product-fields",jsonObject,m_productDownloadFields, false);	

	//category parameter
	getJSONunparsedObject("category-parameter",jsonObject,m_categoryParameter, false);	
	

	//search config
	if(!jsonObject["search"].is_null())
	{
		json_spirit::mArray tmpArray=jsonObject["search"].get_array();
		for(json_spirit::mArray::const_iterator i = tmpArray.begin();i != tmpArray.end();++i )
		{
			json_spirit::mObject tmpObj=(i->get_obj());
			std::string name = std::string(tmpObj["name"].get_str());
			bool transmit = getJSONbool("transmit",tmpObj,false,true);
			std::string key = std::string(tmpObj["key"].get_str());
			std::string type = std::string(tmpObj["type"].get_str());
			std::string object = std::string(tmpObj["object"].get_str());

			Searcher* s = Searcher::createSearcher(name,type,key,object,transmit, m_search_hashkey, m_resource_hashkey);
			if( s == NULL) throw CrunchWrongKeyException(" unable to create search object at init time");
			m_searchers.push_back(s);
		}
	}
	//CSV config
	if(!jsonObject["csv-convert"].is_null())
	{
		m_format = csv;  
		json_spirit::mObject csvObj=jsonObject["csv-convert"].get_obj();
		std::string delimiterString = getJSONstring("delimiter", csvObj, false, std::string(1,m_CSVdelimiter));
		m_CSVdelimiter = delimiterString.at(0);
		std::string enclosureString = getJSONstring("enclosure", csvObj, false, std::string(1,m_CSVenclosure));
		m_CSVenclosure = enclosureString.at(0);
	
		if(csvObj["keys"].is_null())
			 throw CrunchWrongKeyException("No keys in config file to convert csv to xml");
		json_spirit::mObject keysObj =csvObj["keys"].get_obj();
		for(json_spirit::mObject::const_iterator key = keysObj.begin();key != keysObj.end();++key )
		{
			CSVfield field;
			
			switch( key->second.type())
			{
				case json_spirit::str_type : field.m_value = key->second.get_str(); break;
				case json_spirit::array_type : getJSONarray(key->first.c_str(),keysObj, field.m_arrayValues); break;
				default: throw CrunchWrongKeyException("One CSV key is not array nor string"); 
			}

			m_CSVfields.insert( std::make_pair<std::string,CSVfield>( key->first, field) );
		}
	}

	#ifdef __DEBUG__
		std::cout << " End of parse config file \n";
	#endif

}

/**
* get specific JSON value from config file
* valuetype= INT -> integer
*/
std::string crunch::CrunchConfig::getJSONstring(const char* name, json_spirit::mObject& object, bool mandatory, std::string def)
{
#ifdef __DEBUG__
std::cout << "GetJson string called for " << name <<"\n";
#endif
	if(object[name].is_null())
	{
		if(mandatory)
		{
			std::string error=std::string("Need '")+name+std::string("' parameter in JSON config\n");
			throw CrunchWrongKeyException(error);
		}
		return def;
	}
	return std::string(object[name].get_str());
}


/**
* get specific JSON integer value from config file
*/
int crunch::CrunchConfig::getJSONint(const char* name,  json_spirit::mObject& object, bool mandatory, int def)
{

	if(object[name].is_null())
	{
		if(mandatory)
		{
			std::string error=std::string("Need '")+name+std::string("' parameter in JSON config\n");
			throw CrunchWrongKeyException(error);
		}
		return def;
	}
	return object[name].get_int();
}


/**
* get specific JSON bool value from config file
*/
bool crunch::CrunchConfig::getJSONbool(const char* name,  json_spirit::mObject& object, bool mandatory, bool def)
{

	#ifdef __DEBUG__
	std::cout << "GetJson Bool called for " << name <<"\n";
	#endif
	if(object[name].is_null())
	{
		if(mandatory)
		{
			std::string error=std::string("Need '")+name+std::string("' parameter in JSON config\n");
			throw CrunchWrongKeyException(error);
		}

    #ifdef __DEBUG__
        std::cout << "GetJson Bool returned for " << name << " defaut " << def << "\n";
    #endif

		return def;
	}

    #ifdef __DEBUG__
    std::cout << "GetJson Bool returned for " << name << " non  defaut " << object[name].get_bool() << "\n";
    #endif

	return object[name].get_bool();
}


/**
* get specific JSON ARRAY value from config file
* @param const char* name : key name of the array to parse
* @param const std::vector<std::string>&  = vector to store value in.
*/
void crunch::CrunchConfig::getJSONarray(const char* name,  json_spirit::mObject& object,
 										std::vector <std::string>& vector , bool mandatory)
{
	#ifdef __DEBUG__
	std::cout << "GetJson Array called for " << name <<"\n";
	#endif
	if(object[name].is_null())
	{
		if(mandatory)
			{
				std::string error=std::string("Need '")+name+std::string("' parameter in JSON config\n");
				throw CrunchWrongKeyException(error);
		}
		return;
	}

	json_spirit::mArray tmpArray=object[name].get_array();
	for(json_spirit::mArray::const_iterator i = tmpArray.begin();i != tmpArray.end();++i )
		vector.push_back(i->get_str());
}

/**
* get specific JSON OBJECT value from config file. Object will be of one level only (no tree search)
* @param const char* name : key name of the array to parse
* @param boost::unordered_map <std::string, std::string>& map  = map to store value in.
*/
void crunch::CrunchConfig::getJSONobject(const char* name,  json_spirit::mObject& object, 
										boost::unordered_map <std::string,
										std::string>& map,
										bool mandatory)
{
	#ifdef __DEBUG__
	std::cout << "GetJson Object called for " << name <<"\n";
	#endif
	if(object[name].is_null())
	{
		if(mandatory)
			{
				std::string error=std::string("Need '")+name+std::string("' parameter in JSON config\n");
				throw CrunchWrongKeyException(error);
			}
		return;
	}

	json_spirit::mObject tmpObject=object[name].get_obj();
	for(json_spirit::mObject::const_iterator i = tmpObject.begin();i != tmpObject.end();++i )
	{

		if ( i->second.type() == json_spirit::str_type)
			map.insert( std::make_pair<std::string,std::string>( i->first, i->second.get_str()) );
		if ( i->second.type() == json_spirit::obj_type)
			map.insert( std::make_pair<std::string,std::string>( i->first, json_spirit::write(i->second,json_spirit::raw_utf8)) );
	}
}

/**
* get specific JSON OBJECT value from config file without parsing it. 
* @param const char* name : key name of the array to parse
* @param boost::unordered_map <std::string, std::string>& map  = map to store value in.
*/
void crunch::CrunchConfig::getJSONunparsedObject(const char* name,  json_spirit::mObject& object, 
										boost::unordered_map <std::string,
										json_spirit::mObject>& map,
										bool mandatory)
{
	#ifdef __DEBUG__
	std::cout << "GetJson Object called for " << name <<"\n";
	#endif
	if(object[name].is_null())
	{
		if(mandatory)
			{
				std::string error=std::string("Need '")+name+std::string("' parameter in JSON config\n");
				throw CrunchWrongKeyException(error);
			}
		return;
	}

	json_spirit::mObject tmpObject=object[name].get_obj();
	for(json_spirit::mObject::const_iterator i = tmpObject.begin();i != tmpObject.end();++i )
	{
		if ( i->second.type() == json_spirit::obj_type)
			map.insert( std::make_pair<std::string,json_spirit::mObject>( i->first,i->second.get_obj()) );
	}
}

/*** EXCEPTIONS DEFINED BY THE CRUNCH CLASS *****/

crunch::CrunchWrongKeyException::CrunchWrongKeyException(const std::string& str) : std::runtime_error(str)
{
}
/*** END OF EXCEPTION DEFINE ***/

