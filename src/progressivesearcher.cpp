#include "progressivesearcher.hpp"
#include "util.hpp"
#include <boost/algorithm/string.hpp>

using namespace crunch;

std::string ProgressiveSearcher::typeToString()
{
#ifdef __DEBUG__
std::cout << "  crunch::ProgressiveSearcher::typeToString m_type [" << m_type << "]" << '\n';

#endif

    if(m_type == product)
        return "product";

    if(m_type == model)
         return "model";

    if(m_type == reference)
         return "reference";

return "unknown";
}

stringvector ProgressiveSearcher::splitString(std::string &str)
{
    stringvector result;
    boost::split(result, str, boost::is_any_of("\t\n ,;./\\"));
    return result;
}

bool ProgressiveSearcher::addElement(Model* m, Product *p, Reference *r, Crunch *crunch)
 {
 	std::string value;

    for(int i=0;i<m_fieldsNameVector.size();i++)
    {
        value+= crunch->getItemValue(m_crunchFieldsVector[i])+std::string(" ");
    }
   

 	CrunchElement* elt = NULL;

	switch(m_type)
	{
		case model : elt = m; break;
		case product : elt = p; break;
		case reference : elt = r; break;
	}

 	if(!value.size())
 		return false;
    Util::cleanString(value);
    stringvector words = splitString(value);

    for (   stringvectoriterator pos = words.begin();
            pos != words.end();
            ++pos)
    {
        unsigned int length = Util::utf8size(*pos);
        if( length<3) continue;

        //locate tree branch to insert item
        progressiveSearcherTreeItem* item = getTreeItem(*pos);

        //pair to insert :
        std::pair<std::string, CrunchElement*> pair(value, elt);

        //item already exists in this tree branche ?
        
        bool itemfound = false;
        for(int index=0; index < item->m_crunchElements.size(); index++)
            if( (item->m_crunchElements[index].second == elt) && (item->m_crunchElements[index].first == value) )
            {
                itemfound = true;
              break;
            }
       if(!itemfound)
            item->m_crunchElements.push_back(std::pair<std::string, CrunchElement*>(value, elt));
    }
    
 
 }
/**
* getTreeItem get a value and browse the progressive searcher tree 
* to find the right place to insert the value 
*/
 progressiveSearcherTreeItem* ProgressiveSearcher::getTreeItem(std::string& value)
 {

    std::string start = value.substr(0,3);
    unsigned short length = Util::utf8size(value);
    progressiveSearcherTreeItem* startingPoint = m_tree[start];

    if(startingPoint == NULL )
        {
            startingPoint = new progressiveSearcherTreeItem;
            startingPoint->m_value = start;
            m_tree[start] = startingPoint;
        }
    if(length <=3) return startingPoint;
    for(unsigned short i=3;i < PROGRESSIVE_MAX_DEPTH; i++)
    {
        char c = value.at(i);
        progressiveSearcherTreeItem* nextPoint = startingPoint->m_nextLetter[c];
        if(nextPoint == NULL)
        {
            nextPoint = new progressiveSearcherTreeItem;
            nextPoint->m_value = value.substr(0,i+1);
            startingPoint->m_nextLetter[c] = nextPoint;
        }
        startingPoint = nextPoint;
        if(i >= (length-1) ) break;
    }
        
    return startingPoint;
 }

bool ProgressiveSearcher::addCategory(Category *c)
 {
     std::string value = c->m_name;
     Util::cleanString(value);
     stringvector words = splitString(value);


    for (   stringvectoriterator pos = words.begin();
            pos != words.end();
            ++pos)
    {

        unsigned short length = Util::utf8size(*pos);
        if( length<3) continue;
        //add string
        progressiveSearcherTreeItem* item = getTreeItem(*pos);
        item->m_categories.push_back(std::pair<std::string, Category*>(value, c));
    }
    
 
    return false;
 }

 

json_spirit::mObject ProgressiveSearcher::outputJson(std::string base_path, const std::string server_path, const std::string* server_url, unsigned int version)
{
	json_spirit::mObject indexJson;

    std::string path = base_path+std::string(SEARCHPATH)+std::string(m_name)+std::string("-");
    std::string serverpath = server_path+std::string(SEARCHPATH)+std::string(m_name)+std::string("-");

    for (   boost::unordered_map <std::string, progressiveSearcherTreeItem*>::const_iterator pos = m_tree.begin();
            pos != m_tree.end();
            ++pos)
    {
        json_spirit::mObject obj = pos->second->dumpJsonFiles(path,  server_path, server_url, m_transmit, m_type, version); 
        
    }

    indexJson [ "type"]= "progressive";
    indexJson [ "resource_base"]= std::string(SEARCHPATH)+std::string(m_name)+std::string("-") ;
    indexJson [ "resource_hashkey"]= 1;

	return indexJson;

 }

 ProgressiveSearcher::ProgressiveSearcher(std::string name, std::string fieldname, ElementType _type,  bool transmit, short hashsize, short folderhashsize)
 {
    m_transmit = transmit;
 	m_hashSize=hashsize;
 	m_folderHashSize = folderhashsize;
 	m_fieldName =fieldname;
    std::vector<std::string> split = splitString(fieldname);
    for(int i=0;i<split.size();i++)
        if(split[i].length() > 0)
            m_fieldsNameVector.push_back(split[i]);
 	m_type = _type;
 	m_name = name;
 }


bool ProgressiveSearcher::init(Crunch* crunch)
{
    for(int i=0;i<m_fieldsNameVector.size();i++)
    {
        CrunchField* field = crunch->getCrunchFieldByName(m_fieldsNameVector[i]);
        if(field == NULL) return false;
        m_crunchFieldsVector.push_back(field);
    }
    return true;

}
 void ProgressiveSearcher::compute()
 {
 	//compute count of sublevels in progressiveSearcherTreeItem
    for (   boost::unordered_map <std::string, progressiveSearcherTreeItem*>::const_iterator pos = m_tree.begin();
            pos != m_tree.end();
            ++pos)
    {
        pos->second->getSubCount();
    }

 }



 //methods for progressiveSearcherTreeItem struct
/**
* getsubcount is a recursive method
* it will count : 1st, the number of leaves in the sublevels and 2nd, the number of intersections in the sublevels
*/
std::pair<unsigned long, unsigned long> progressiveSearcherTreeItem::getSubCount()
  {
   for (   boost::unordered_map <char, progressiveSearcherTreeItem*>::const_iterator pos = m_nextLetter.begin();
            pos != m_nextLetter.end();
            ++pos)
      {
            std::pair<unsigned long, unsigned long> results = pos->second->getSubCount();

            m_nextLevelLeavesCount += results.first;
            m_nextLevelIntersectionsCount += results.second;
      }
        


    unsigned long size = m_nextLetter.size();
    if(size) size--;


  return std::pair<unsigned long,unsigned long>
                (m_nextLevelLeavesCount + m_categories.size() + m_crunchElements.size(),
                m_nextLevelIntersectionsCount + size
                );
  }




json_spirit::mObject progressiveSearcherTreeItem::dumpJsonFiles(const  std::string basePath, const std::string server_path, const std::string* server_url,  bool transmit, ElementType type, unsigned int version)
{
    json_spirit::mObject treeJson;
    json_spirit::mArray exactArray;

    for(unsigned int crunchElts=0;crunchElts<m_crunchElements.size(); crunchElts++)
        exactArray.push_back( getJsonRepresentation(m_crunchElements[crunchElts], server_url, type) );

    for(unsigned int cat=0;cat<m_categories.size(); cat++)
        exactArray.push_back( getJsonRepresentation(m_categories[cat], server_url) );

    
    treeJson [ "exact"]= exactArray;


    std::string nextLetters;
    std::vector< std::pair< std::string, CrunchElement*> > eltArray;
    std::vector< std::pair< std::string, Category*> > catArray;
    json_spirit::mArray nextArray;
                                            
    if((m_nextLevelLeavesCount > MAX_COUNT_TO_EXPAND) //to much results with completing letters.
        && (m_nextLevelIntersectionsCount > 0) )
    {
        treeJson[ "status"]= "tbc";

        //inserer next letter 
        for (   boost::unordered_map <char, progressiveSearcherTreeItem*>::const_iterator pos = m_nextLetter.begin();
                pos != m_nextLetter.end();
                ++pos)
        {
            char buffer[10]; sprintf(buffer,"%c",pos->first);
            nextLetters += std::string(buffer);
            pos->second->dumpJsonFiles(basePath, server_path, server_url, transmit, type, version);
        }
    }
    else
    {
        //inserer le dump de next_letter
        for (   boost::unordered_map <char, progressiveSearcherTreeItem*>::const_iterator pos = m_nextLetter.begin();
            pos != m_nextLetter.end();
            ++pos)
        {
            char buffer[10]; sprintf(buffer,"%c",pos->first);
            nextLetters += std::string(buffer);
            pos->second->dumpNext(eltArray, catArray, server_url, version);
        }
        treeJson [ "status"] =  "done";
       
        for(unsigned int crunchElts=0;crunchElts<eltArray.size(); crunchElts++)
            nextArray.push_back( getJsonRepresentation(eltArray[crunchElts], server_url,type) );
            
        for(unsigned int cat=0;cat<catArray.size(); cat++)
            nextArray.push_back( getJsonRepresentation(catArray[cat], server_url) );

        treeJson [ "next"] = nextArray ;
    }

    treeJson [ "nextLetters"] = nextLetters ;


    std::string folderName=std::string(m_value,0,1)+"/";
    std::string fileName=m_value+std::string(JSONEXT);

        if(transmit)
            crunch::Util::writeJsonFile(treeJson, basePath + folderName, fileName);
        if(server_path.length())
           crunch::Util::writeJsonFile(treeJson, server_path + folderName, fileName);

    return treeJson;
}

json_spirit::mObject progressiveSearcherTreeItem::getJsonRepresentation(std::pair< std::string, Category*> elt, const std::string* server_url)
{
    json_spirit::mObject json =  elt.second->outputJson(JSON_PATH | JSON_TITLE, true, server_url);
    json [ "type"] = "category" ;
    json [ "textsearch"] =  elt.first ;
    return json;
}

json_spirit::mObject progressiveSearcherTreeItem::getJsonRepresentation(std::pair< std::string, CrunchElement*> elt, const std::string* server_url, ElementType type)
{
    json_spirit::mObject json;
    switch(type)
    {
        case model :  
            json = ((Model*)(elt.second))->outputJson(JSON_PATH | JSON_TITLE | JSON_ID, server_url);
            json [ "type" ] =  "model" ;
            break;

        case product : 
            json = ((Product*)(elt.second))->outputJson(JSON_PATH | JSON_TITLE | JSON_ID, server_url);
            json [  "type"] = "product" ;
            break;

        case reference : 
            json = ((Reference*)(elt.second))->outputJson(JSON_PATH | JSON_TITLE | JSON_ID, server_url);
            json [ "type" ] = "referece" ;
            break;
    }

    json [ "textsearch"] =  elt.first;
    return json;

}



void progressiveSearcherTreeItem::dumpNext( std::vector< std::pair< std::string, CrunchElement*>  >& eltArray, 
                                            std::vector< std::pair< std::string, Category*>  >& catArray,
                                            const std::string* server_url,
                                            unsigned int version)
{

    for(unsigned int crunchElts=0;crunchElts<m_crunchElements.size(); crunchElts++)
    {
        bool itemfound = false;
        for(int index=0; index < eltArray.size(); index++)
            if( (eltArray[index].second == m_crunchElements[crunchElts].second) && (eltArray[index].first == m_crunchElements[crunchElts].first) )
            {
                itemfound = true;
                break;
            }
        if(itemfound)
            continue;
        eltArray.push_back(m_crunchElements[crunchElts]);
    }
    for(unsigned int cat=0;cat<m_categories.size(); cat++)
    {
       bool itemfound = false;
        for(int index=0; index < catArray.size(); index++)
            if( (catArray[index].second == m_categories[cat].second) && (catArray[index].first == m_categories[cat].first) )
            {
                itemfound = true;
                break;
            }
        if(itemfound)
            continue;
        catArray.push_back(m_categories[cat]);
    }

    for (   boost::unordered_map <char, progressiveSearcherTreeItem*>::const_iterator pos = m_nextLetter.begin();
        pos != m_nextLetter.end();
        ++pos)
    {
        pos->second->dumpNext(eltArray,catArray, server_url, version);
    }

}








          