#ifndef __XMLREADER_HPP__
#define __XMLREADER_HPP__

/**
* @author Marc Morel
* @copyright Procheo, 2013
*/

#include "reader.hpp"
#include "pugixml-1.2/pugixml.hpp"

/**
* The class crunch is the main class for the crunch process in case of XML source file. 
*/
namespace crunch
{
	class XMLReader : public Reader
	{

		pugi::xml_document m_XMLdoc;
		pugi::xml_parse_result m_XMLresult;
		pugi::xml_node m_items;
		pugi::xml_node m_currentItem;
		bool m_itemInit;

	public:

		/**
		* Default constructor
		* Will create a root category
		*/
		XMLReader(CrunchConfig *config);

		/**
		* Initialization function
		* @param const std::string& JSON parameter absolute path (in local filesystem)
		*/
		bool openSourceFile();

		/**
		* Iterator function
		* @return true if next item has been found
		*/
		bool hasNextItem();

		std::string getItemValue(const CrunchField* field);
		std::vector<std::string> getItemArray(const CrunchField* field);

	};
}




#endif