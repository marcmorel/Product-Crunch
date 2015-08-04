#ifndef __MODEL_HPP__
#define __MODEL_HPP__

#include <vector>
//#include "crunchelement.hpp"
#include "json_spirit/json_spirit.h"

namespace crunch {

    class CrunchElement;
    class Crunch;
    class Product;
    class Reference;
    class Category;

    /**
     * class Model derived class from CrunchElement
     */
    class Model : public CrunchElement
    {
            
    public:
        boost::unordered_map <std::string, Product*> m_products;//unordered_map of  Product object belonging to the Model instance
        static unsigned long m_modelCounter;//Model instance counter
        //Json Object Representation of a Model Object depending on detailLevel
        json_spirit::mObject outputJson(CrunchDetailLevel detailLevel = JSON_ALL,const std::string* server_url = NULL, int crunchVersion = -1,bool isRootOutput = false);
        // Json Object Chunked Representation of custom fields of the Model Object
        json_spirit::mObject outputCustomJson(int crunchVersion = -1);
        // Model constructor
        Model(const Item& item, Crunch* crunch, Category* c);

        //Model Destructor
        ~Model();
        //find the Product from m_products or insert it then return the Product pointer
        Product* findOrAddProduct(const Item& item, bool& isNewProduct, Category *c);
    };

} //namespace crunch

#endif