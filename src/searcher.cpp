#include "searcher.hpp"
#include "exactsearcher.hpp"
#include "progressivesearcher.hpp"

crunch::Searcher* crunch::Searcher::createSearcher(std::string name, std::string searcherType, std::string fieldname, std::string object_type,  bool transmit, short hashsize, short folderhashsize)
{

	ElementType _type;

	if(object_type.compare("product") == 0) { 
     _type = product;
    }
	else if(object_type.compare("model") == 0){
        _type = model;
    }
	else if(object_type.compare("reference") == 0) 
    {
        _type = reference;
    }
    else if(object_type.compare("categorymodel") == 0) 
    {
        _type = categorymodel;
    }
	else throw std::runtime_error(std::string("unknown crunch type : ")+object_type);

	if(!searcherType.compare("exact"))
		{
			return (Searcher*) new ExactSearcher(name, fieldname, _type, 	transmit, hashsize, folderhashsize);
		}
	if(!searcherType.compare("progressive"))
		{
			return (Searcher*) new ProgressiveSearcher(name, fieldname, _type, transmit, hashsize, folderhashsize);
		}
	return NULL;
}

bool crunch::Searcher::init(Crunch* crunch)
{
	m_crunchField = crunch->getCrunchFieldByName(m_fieldName);
	if(m_crunchField == NULL)
		return false;
	return true;

}
std::string& crunch::Searcher::getFieldName()
{
	return m_fieldName;
}


std::string& crunch::Searcher::getName()
{
	return m_name;
}

