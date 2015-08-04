#include "crunchelement.hpp"
#include "reference.hpp"
#include "util.hpp"

//Reference instance counter
unsigned long crunch::Reference::m_referenceCounter = 0;

crunch::Reference::~Reference(){

}
/**
 * Reference constructor
 */
crunch::Reference::Reference(const Item&  item, Crunch* crunch,Product* p, Category *c)
{
    m_type       = reference; //set the ElementType
    m_filePrefix = "ref/";    // set prefix of json file (local path in resource) : ex prod/ or mod/
    m_data       = item;      // main crunch element values (price, name, description, ... ) 
    m_parent     = p;         //set parent Product
    m_crunch     = crunch;    //pointer to main crunch instance
    m_category = c;
    crunch::Util::md5(item.m_g_id,m_idMd5); //set the md5 hash of the item m_g_id

    //one more Reference instance
    m_referenceCounter++;



}

/**
 * return Json Object Representation of a Reference Object
 */
json_spirit::mObject crunch::Reference::outputJson(CrunchDetailLevel detailLevel,const std::string* server_url , int crunchVersion)
{
    json_spirit::mObject obj;
    obj = CrunchElement::outputJson(detailLevel,server_url, crunchVersion);
    return obj;
}

/**
 * return Json Object Chunked Representation of custom fields of the Reference Object
 */
json_spirit::mObject crunch::Reference::outputCustomJson(int crunchVersion)
{
    json_spirit::mObject obj;
    for (   boost::unordered_map <std::string, std::string>::iterator pos = m_data.m_custom_reference_fields.begin();
            pos != m_data.m_custom_reference_fields.end();
            ++pos)
        obj[pos->first] = pos->second;
    return obj;
}
