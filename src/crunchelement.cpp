#include "crunchelement.hpp"
#include "category.hpp"
#include "crunch.hpp"
#include "util.hpp"
#include <vector>
#include <string>
#include <iostream>

using namespace crunch;




crunch::CrunchElement::CrunchElement()
{
}

crunch::CrunchElement::~CrunchElement()
{
    m_picturesURL.clear();
    m_pictures.clear();
}

void crunch::CrunchElement::setHashKeySize(unsigned short resourcekeysize, unsigned short filekeysize)
{
	m_resourceHashKeySize = resourcekeysize;
	m_fileHashKeySize = filekeysize;
	constructFilePath();

}


std::string crunch::CrunchElement::getHash(const unsigned short length)
{
	return std::string(m_idMd5,length);
}

/**
* constructFilePath will prepare the resource name and json file name in which this crunch element is to get stored 
* hashkeysize gives the length (in car) of the resource group key.
**/
void crunch::CrunchElement::constructFilePath()
{

	std::string file_hashkey = getHash(m_fileHashKeySize);
	m_resource = std::string(CATALOGPREFIX) + getHash(m_resourceHashKeySize)+std::string("/");
	m_filePath = m_filePrefix+file_hashkey+std::string(JSONEXT);
}

void crunch::CrunchElement::addPicture(Picture* pic)
{
	if(std::find(m_picturesURL.begin(), m_picturesURL.end(), pic->m_URL)!=m_picturesURL.end())
		return; //picture already in crunch element
	
	m_picturesURL.push_back(pic->m_URL);
	m_pictures.push_back(pic);
}


void crunch::CrunchElement::addFile(std::string fileName, CrunchFile* file)
{
	Util::log(std::string("Add file ")+fileName+std::string(" ")+file->m_URL,logINFO);
	m_files.insert(std::pair<std::string,CrunchFile*>(fileName,file));
}

std::vector<Picture*>* crunch::CrunchElement::getPictures()
{
	return &m_pictures;
}
const Item* crunch::CrunchElement::getItem()
{
	return &m_data;
}
/**
 *  build a json resource path object
 */
json_spirit::mObject crunch::CrunchElement::buildJsonResourcePathPair(){
    json_spirit::mObject path_obj;

    path_obj ["resource"] = m_resource;
    path_obj ["file"] = m_filePath;

    return path_obj;
}

/**
 * build a search index resource object
 */
void crunch::CrunchElement::appendJsonResourceSearchObject(json_spirit::mObject& obj)
{
    obj["resource"] = m_resource;
    obj["file"] = m_filePath;
    obj["search-value"] = m_searchMatchValue;
}
/**
* will construct a json_spirit::mObject from crunch element
* @param CrunchDetailLevel bit mask of information to include in json output
* @param CrunchVersion JSON syntax to use for ouput
*/
json_spirit::mObject crunch::CrunchElement::outputJson(CrunchDetailLevel detailLevel,const std::string* server_url, int crunchVersion,bool isRootOutput)
{
	json_spirit::mObject obj;//main representation of crunch element as a json object

    //inject price data section into json
	if(detailLevel & JSON_PRICE)
	{
		json_spirit::mObject price_obj;
		price_obj[ "current"]  = m_data.m_g_sale_price ;
		price_obj[ "initial"]  = m_data.m_g_price ;
	
		obj[ "price" ] =  price_obj ;
	}
    //add ids and ean infos
	if(detailLevel & JSON_ID)
	{
		obj[ "id"  ]= m_data.m_g_id ;
		obj[ "ean" ]= m_data.m_g_gtin ;
	}
    //add description and title keys
	if( (detailLevel & JSON_DESCRIPTION) ||  (detailLevel & JSON_TITLE))
	{
        //in crunch v1 description and title are nested under 
		if( crunchVersion == CRUNCH_VERSION_1 )
		{	
			json_spirit::mObject description_obj;
			json_spirit::mObject description_fr_obj;

			if( detailLevel & JSON_TITLE)
			{
				description_fr_obj[ "title"]= m_data.m_title ;
				description_fr_obj[ "brand"]= m_data.m_g_brand ;
			}

			if( detailLevel & JSON_DESCRIPTION)
				description_fr_obj[ "description"]= m_data.m_description ;

			description_obj[ "fr_fr"]= description_fr_obj ;
			
            obj[ "description"]= description_obj ;
		}
		else //other wise directly attached to  the root of the object
		{
			if( detailLevel & JSON_DESCRIPTION )
				obj[ "description"]= m_data.m_description ;
			
            if( detailLevel & JSON_TITLE)
            {
				obj[ "title" ]= m_data.m_title  ;
				obj[ "brand"]= m_data.m_g_brand  ;
				if(m_category)
				{
					obj[ "tree"]= m_category->getTreeString() ;
				}
			}
		}
	}

	//output images
	if( detailLevel & JSON_PICTURES)
	{	
		json_spirit::mArray picturesJson;
		for(unsigned short picIndex=0; picIndex< m_pictures.size(); picIndex++)
		{
			picturesJson.push_back(m_pictures[picIndex]->outputJson(server_url, crunchVersion));
		}
		obj[ "photo" ] = picturesJson;
	}

	//output custom fields
		if( detailLevel & JSON_CUSTOM)
			obj [ "custom"] = outputCustomJson(crunchVersion);

	//output custom fields
		if( detailLevel & JSON_FILES)
			obj [ "files"] = outputFilesJson(server_url,crunchVersion);

	return obj;
}

/**
 * return Json Object Chunked Representation of files of the  Object
 */
json_spirit::mObject crunch::CrunchElement::outputFilesJson(const std::string* server_url, int crunchVersion)
{
    json_spirit::mObject obj;
    for (   boost::unordered_map <std::string, CrunchFile*>::iterator pos = m_files.begin();
            pos != m_files.end();
            ++pos)
    {
    	obj [pos->first] = (pos->second)->outputJson(server_url);
    }
       
    return obj;
}

