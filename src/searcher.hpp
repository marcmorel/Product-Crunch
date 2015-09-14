#ifndef __SEARCHER_HPP
#define __SEARCHER_HPP
#include "crunchstruct.hpp"
#include "crunchelement.hpp"
#include "crunch.hpp"

/**
* @author Marc Morel
* @copyright Marc Morel, 2015
*/


/**
* the class Searcher is an abstract class. It is used by the Crunch class to store and manage all kind of search indexes.
*
**/



namespace crunch
{

	class Searcher
	{

	protected:
		/** field name whose values will be searched for. **/
		std::string m_fieldName;

		/** searcher name. Will be used as a reference in the JSON output **/
		std::string m_name;
	public:
bool	m_transmit; //whether the search indexes will be stored on server (false) or transmitted to device (true)

		
		ElementType m_type;
		CrunchField* m_crunchField;
		virtual bool init(Crunch* crunch);
		virtual bool addElement(Model* m, Product *p, Reference *r, Crunch* crunch) = 0;
		virtual bool addCategory(Category *c) = 0;
		virtual void compute() = 0;
		virtual json_spirit::mObject outputJson(std::string basePath, std::string serverPath, const std::string* server_url, unsigned int version) =0;
		std::string& getFieldName();
		std::string& getName();
        virtual std::string typeToString() = 0;
		static Searcher* createSearcher(std::string name, std::string searcherType, std::string fieldname, std::string object,  bool transmit, short hashSize, short folderhashsize);
	};
}

#endif