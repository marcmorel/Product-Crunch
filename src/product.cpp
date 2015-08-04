#include "crunchelement.hpp"
#include "crunch.hpp"
#include "product.hpp"
#include "util.hpp"

//Product instance counter
unsigned long crunch::Product::m_productCounter = 0;

/**
 * Product constructor
 */
crunch::Product::Product(const Item&  item, Crunch* crunch, Model* m, Category *c) 
{
    m_type   = product;//set the ElementType
    m_data   = item;   // main crunch element values (price, name, description, ... ) 
    m_parent = m;       //set parent Model
    m_category = c;    //category 

    m_filePrefix = PRDFILEPREFIX;  // set prefix of json file (local path in resource) : ex prod/ or mod/
    crunch::Util::md5(item.m_productGroup,m_idMd5); //set the md5 hash of the item m_g_id

    m_crunch = crunch;  //pointer to main crunch instance
    //one more Product instance
    m_productCounter++;

}

/**
 * Product Desctructor
 */
crunch::Product::~Product () {


/*for (  std::vector<Reference*>::iterator ref = m_references.begin();
            ref != m_references.end();
            ++ref)
    {
        delete (*ref);
        //ref = m_references.erase(ref);
    }
*/
while (!m_references.empty()) {
   delete m_references.back();  m_references.pop_back();
}

//free memory m_references
 m_references.clear();

}

/**
 * return Json Object Representation of a Product Object depending on detailLevel
 */
json_spirit::mObject crunch::Product::outputJson(CrunchDetailLevel detailLevel,const std::string* server_url, int crunchVersion,bool isRootOutput)
{
    json_spirit::mObject obj;
    obj=CrunchElement::outputJson(detailLevel,  server_url, crunchVersion,isRootOutput);

    //obj.push_back(json_spirit::Pair("isRootOutput",isRootOutput));

    if( detailLevel & JSON_PATH )
    {
        if(isRootOutput == false)
            obj ["path"] = CrunchElement::buildJsonResourcePathPair();
    }

    if( detailLevel & JSON_SEARCHPATH )
    {
        CrunchElement::appendJsonResourceSearchObject(obj);
    }

    if(detailLevel & JSON_CHILDREN )
    {
        json_spirit::mArray refJson;
        for (   std::vector<Reference*>::iterator pos = m_references.begin();
                pos != m_references.end();
                ++pos)
            refJson.push_back((*pos)->outputJson(JSON_ALL,server_url, crunchVersion));
        obj ["ref"]  = refJson;
    }
    return obj;
}

/**
 * return Json Object Chunked Representation of custom fields of the Product Object
 */
json_spirit::mObject crunch::Product::outputCustomJson(int crunchVersion)
{
    json_spirit::mObject obj;
    for (   boost::unordered_map <std::string, std::string>::iterator pos = m_data.m_custom_product_fields.begin();
            pos != m_data.m_custom_product_fields.end();
            ++pos)
        obj [ pos->first] =  pos->second;
    return obj;
}



/**
 * add a Reference Object to the product instance vector m_references
 */
crunch::Reference* crunch::Product::addReference(const Item&  item, Category *c)
{
    Reference* r=new Reference(item,m_crunch,this,c);
    m_references.push_back(r);
    return r;
}