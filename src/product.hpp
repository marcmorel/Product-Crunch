#ifndef __PRODUCT_HPP__
#define __PRODUCT_HPP__


#include "crunchelement.hpp"
#include "json_spirit/json_spirit.h"

namespace crunch {
    
    class Crunch;
    class Model;
    class Reference;
    class CrunchElement;

    /**
     * class Product derived class from CrunchElement
     */
    class Product : public CrunchElement
    {
        std::vector<Reference*> m_references;//vector of  Reference object belonging to the Product instance

        Model* m_parent;//Model pointer to the model containing the product instance
        
    public:
        static unsigned long m_productCounter;//Product instance counter
        //Json Object Representation of a Product Object depending on detailLevel
        json_spirit::mObject outputJson(CrunchDetailLevel detailLevel = JSON_ALL,const std::string* server_url = NULL, int crunchVersion = -1,bool isRootOutput = false);
        // Json Object Chunked Representation of custom fields of the Product Object
        json_spirit::mObject outputCustomJson(int crunchVersion = -1);
        //Product constructor
        Product(const Item& item, Crunch* crunch,Model* m, Category *c);

        //Product destructor
        ~Product();
        //add a Reference Object to the product instance vector m_references
        Reference*  addReference(const Item&  item, Category *c);
    };

} //namespace crunch

#endif