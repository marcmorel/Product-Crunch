#include "crunchelement.hpp"
#include "crunch.hpp"
#include "model.hpp"
#include "util.hpp"

//Model instance counter
unsigned long crunch::Model::m_modelCounter=0;

/**
 * Model constructor
 */
crunch::Model::Model(   const Item&  item, Crunch* crunch, Category* c)
{
    m_type   = model; //set the ElementType
    m_data   = item;  // main crunch element values (price, name, description, ... ) 
    m_crunch = crunch;//pointer to main crunch instance
    m_category = c;
    
    m_filePrefix = MODFILEPREFIX; // set prefix of json file (local path in resource) : ex prod/ or mod/
    crunch::Util::md5(item.m_modelGroup,m_idMd5);//set the md5 hash of the item m_g_id

     //one more Model instance
    m_modelCounter++;

}

/**
 * Model destructor
 */
crunch::Model::~Model(){


//free memory of m_products
 for( boost::unordered_map <std::string, Product*>::const_iterator p = m_products.begin(); 
        p != m_products.end(); p++)
            delete p->second;

m_products.clear();


}
/**
* findOrAddProduct will search if product already exists for this model.
* check is being done using the Id from item
* if so, a pointer to this product is returned
* otherwise, a new product is created, added to the model and returned.
*/
crunch::Product* crunch::Model::findOrAddProduct(const Item& item, bool& isNewProduct, Category* c)
{
    if(m_products.count(item.m_productGroup)==1) // product already existing
    {
        isNewProduct = false;
        return m_products[item.m_productGroup];
    }

    isNewProduct = true;
    Product* p=new Product(item,m_crunch,this,c);
    m_products.insert(std::make_pair<std::string,Product*>(item.m_productGroup,p));
    return p;

}

/**
 * return Json Object Representation of a Model Object depending on detailLevel
 */
json_spirit::mObject crunch::Model::outputJson(CrunchDetailLevel detailLevel,const std::string* server_url ,int crunchVersion,bool isRootOutput)
{
    json_spirit::mObject obj;
    obj = CrunchElement::outputJson(detailLevel, server_url, crunchVersion,isRootOutput);
    
    
    if(detailLevel & JSON_PATH ){
        if(isRootOutput == false)
            obj ["path"] = CrunchElement::buildJsonResourcePathPair();
    }

    if(detailLevel & JSON_SEARCHPATH)
    {
        CrunchElement::appendJsonResourceSearchObject(obj);//obj.push_back(CrunchElement::buildJsonResourceSearchObject());
    }

    if(detailLevel & JSON_CHILDREN)
    {
        json_spirit::mArray prodJson;

        for (   boost::unordered_map <std::string, Product*>::iterator pos = m_products.begin();
                pos != m_products.end();
                ++pos)
        {
            prodJson.push_back(pos->second->outputJson(JSON_PATH | JSON_TITLE,server_url, crunchVersion,false));

        }
        obj [ "prd"] = prodJson;
    }
    return obj;
}

/**
 * return Json Object Chunked Representation of custom fields of the Model Object
 */
json_spirit::mObject crunch::Model::outputCustomJson(int crunchVersion)
{
    json_spirit::mObject obj;
    for (   boost::unordered_map <std::string, std::string>::iterator pos = m_data.m_custom_model_fields.begin();
            pos != m_data.m_custom_model_fields.end();
            ++pos)
        obj [ pos->first ] = pos->second;
    return obj;
}