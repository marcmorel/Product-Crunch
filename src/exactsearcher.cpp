#include "exactsearcher.hpp"
#include "util.hpp"

using namespace crunch;

std::string ExactSearcher::typeToString()
{


    if(m_type == product)
        return "product";

    if(m_type == model)
         return "model";

    if(m_type == reference)
         return "reference";

return "unknown";
}

bool ExactSearcher::addElement(Model* m, Product *p, Reference *r, Crunch *crunch)
 {
 	std::string value = crunch->getItemValue(m_crunchField);
 	CrunchElement* elt = NULL;

	switch(m_type)
	{
		case model : elt = m; break;
		case product : elt = p; break;
		case reference : elt = r; break;
	}

 	if(!value.size())
 		return false;

 	m_crunchElements.push_back(std::pair<std::string,CrunchElement*>(value,elt));

 
 }
 
 bool ExactSearcher::addCategory(Category *c)
 {
    return false;
 }

json_spirit::mObject ExactSearcher::outputJson(std::string basePath, std::string serverPath,const std::string* server_url, unsigned int version)
{
	json_spirit::mObject indexJson;
	
    for( boost::unordered_map <std::string, std::vector<std::pair<std::string,CrunchElement*> >* >::iterator h = m_crunchElementPairsByHashKeys.begin(); 
        h != m_crunchElementPairsByHashKeys.end(); h++)
    {
        //iterate on this hashkey
        std::string hashkey=h->first;
        json_spirit::mArray hashArray;

        //loop on elements pairs sharing this hashkey
        for( std::vector<std::pair<std::string,CrunchElement*> >::iterator crunchEltPair = h->second->begin();
             crunchEltPair != h->second->end();
             crunchEltPair++
            )
            {

                CrunchElement* crunchElt = crunchEltPair->second;
                //bind the match Value
                crunchElt->setMatchValue(crunchEltPair->first);
                switch(m_type)
                    {
                        case model : hashArray.push_back( ((Model*)(crunchElt))->outputJson(JSON_SEARCHPATH, server_url, version)); break;
                        case product : hashArray.push_back( ((Product*)(crunchElt))->outputJson(JSON_SEARCHPATH, server_url, version));break;
                        case reference : hashArray.push_back( ((Reference*)(crunchElt))->outputJson(JSON_SEARCHPATH, server_url, version)); break;
                    }
              }

        
        std::string folderName=basePath+std::string(SEARCHPATH)+m_name+std::string("-")+std::string(hashkey,0,m_folderHashSize)+"/";
        std::string fileName=m_name+hashkey+std::string(JSONEXT);
        if(m_transmit)
            crunch::Util::writeJsonFile(hashArray, folderName, fileName);
        if(serverPath.length())
        {
            std::string serverFolderName=serverPath+std::string(SEARCHPATH)+m_name+std::string("-")+std::string(hashkey,0,m_folderHashSize)+"/";
            crunch::Util::writeJsonFile(hashArray, serverFolderName, fileName);
        }
    }
	indexJson [ "type" ]= "exact" ;
	indexJson [ "resource_base"]=   std::string(SEARCHPATH)+m_name+std::string("-") ;
	indexJson [ "resource_hashkey"]=  m_folderHashSize ;
	indexJson [ "file_hashkey"]=  m_hashSize;
    indexJson [ "element_type"]=  typeToString();

	return indexJson;

 }

 ExactSearcher::ExactSearcher(std::string name, std::string fieldname, ElementType _type,  bool transmit, short hashsize, short folderhashsize)
 {
 	m_hashSize=hashsize;
 	m_folderHashSize = folderhashsize;
 	m_fieldName =fieldname;
 	m_type = _type;
 	m_name = name;
    m_transmit = transmit;

#ifdef __DEBUG__
std::cout << " ExactSearcher::ExactSearcher m_hashSize [" << m_hashSize << "]" << '\n';
std::cout << " ExactSearcher::ExactSearcher m_folderHashSize [" << m_folderHashSize << "]" << '\n';
std::cout << " ExactSearcher::ExactSearcher fieldname [" << fieldname << "]" << '\n';
std::cout << " ExactSearcher::ExactSearcher m_type [" << m_type << "]" << '\n';
std::cout << " ExactSearcher::ExactSearcher m_hashSize [" << m_hashSize << "]" << '\n';
std::cout << " ExactSearcher::ExactSearcher m_name [" << m_name << "]" << '\n';

#endif
 }

 void ExactSearcher::compute()
 {
 	
 	unsigned long itemcount = m_crunchElements.size();

 	if(m_hashSize == -1)
 	{
 		m_hashSize = Crunch::computeHashKeyFileSizeFromItemCount(itemcount);
 		m_folderHashSize = Crunch::computeHashKeyFolderSizeFromItemCount(itemcount);
 	}

 	
 	for(unsigned long index=0; index < itemcount; index++)
 	{
 		char tmpbuf[33];
      	Util::md5(m_crunchElements[index].first,tmpbuf);
 		std::string hashkey = std::string(tmpbuf, m_hashSize);

        std::vector<std::pair<std::string,CrunchElement*> >* vCrunchElementPair;

        if(!m_crunchElementPairsByHashKeys.count(hashkey))
        {
            vCrunchElementPair= new std::vector<std::pair<std::string,CrunchElement*> >;
            m_crunchElementPairsByHashKeys.insert(std::make_pair<std::string,std::vector<std::pair<std::string,CrunchElement*> >* > (hashkey,vCrunchElementPair));
        }
        else {
            vCrunchElementPair = m_crunchElementPairsByHashKeys[hashkey];

            }
         vCrunchElementPair->push_back(std::make_pair(m_crunchElements[index].first,m_crunchElements[index].second)); 

 	}


 }