#include "xmlreader.hpp"





/**
* Default constructor
* Will create a root category
*/
crunch::XMLReader::XMLReader(CrunchConfig* config) : Reader(config)
{
	m_itemInit=false;
}

bool crunch::XMLReader::openSourceFile()
{
	std::cout << "Ouverture de " << m_sourceFile <<" \n";
	m_XMLresult = m_XMLdoc.load_file(m_sourceFile.c_str());
	if(!m_XMLresult) 
	{
		std::cout << "Erreur de parsing xml\n";
		return false;
	}
	pugi::xml_node rssItem= m_XMLdoc.child("rss");
	if(rssItem == NULL)
		return false;
	m_items = rssItem.child("channel");
	if(m_items == NULL) return false;
	return true;

}


/**
* iterate to next XML item. Stores it in m_currentItem and return true if new item found
*/
bool crunch::XMLReader::hasNextItem()
{

	if(!m_itemInit) //first call. Has to find first child of itemS
	{
		m_currentItem=m_items.child("item");
		m_itemInit=true;
		if(m_currentItem==NULL)
			return false;
		return true;
	}
	m_currentItem = m_currentItem.next_sibling("item");
	if(m_currentItem==NULL)
			return false;
		return true;
	}


std::string crunch::XMLReader::getItemValue(const CrunchField* field)
{
	
	if(field==NULL) return std::string("");

	//find field in sourcefile
	pugi::xml_node child = m_currentItem.child(field->m_replacementName.c_str());
	if(!child) return std::string("");

	return child.child_value();

}

std::vector<std::string> crunch::XMLReader::getItemArray(const CrunchField* field)
{
	if(field==NULL) return std::vector<std::string>();

	std::vector<std::string> result;
	//find first child & siblings
	for (pugi::xml_node child = m_currentItem.child(field->m_replacementName.c_str());
		child;
		child = child.next_sibling(field->m_replacementName.c_str()))
		{
			std::string value=child.child_value();
			if(value.length())
				result.push_back(value);
		}
	return result;
}