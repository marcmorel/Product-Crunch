#ifndef __REFERENCE_HPP__
#define __REFERENCE_HPP__

#include "crunchelement.hpp"
#include "json_spirit/json_spirit.h"

namespace crunch {

    /**
     * class Reference derived class from CrunchElement
     */
    class Reference : public CrunchElement
    {
        //Product pointer to the product containing the reference
        Product* m_parent;
    public:
        //Reference instance counter
        static unsigned long m_referenceCounter;
        // Json Object Representation of a Reference Object
        json_spirit::mObject outputJson(CrunchDetailLevel detailLevel = JSON_ALL, const std::string* server_url = NULL, int crunchVersion = -1);
        // Json Object Chunked Representation of custom fields of the Reference Object
        json_spirit::mObject outputCustomJson( int crunchVersion = -1);
        //Reference constructor
        Reference(const Item& item, Crunch* crunch, Product* p, Category *c);

        ~Reference();
    };

} //namespace crunch

#endif