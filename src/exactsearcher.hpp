#ifndef __EXACTSEARCHER_HPP
#define __EXACTSEARCHER_HPP
#include "searcher.hpp"
#include <vector>

/**
* @author Marc Morel
* @copyright Procheo, 2014
*/


/**
* the class ExactSearcher derives form Searcher and is used to perform
* an exact search on a specifid field.
* Search results are stored in a hashmap of vectors.
* The key of the hashmap is the MD5 of the search fields truncated to m_hashSize
**/

namespace crunch
{

	class ExactSearcher : Searcher
	{

		/** raw list of elements to index **/
		std::vector<std::pair<std::string,CrunchElement*> > m_crunchElements;

        /** private storage of pointers to searched elements **/
        boost::unordered_map <std::string, std::vector<std::pair<std::string,CrunchElement*> >* > m_crunchElementPairsByHashKeys;
		
		/** size of hashkey used for the map **/
		short m_hashSize;

		/** size of hashkey to group json file in folders **/
		short m_folderHashSize;
	public:
	
        /** get a string representation of m_type **/
        std::string typeToString();

		/** add a new element in the search map **/
		bool addElement(Model* m, Product *p, Reference *r,  Crunch *crunch);
		bool addCategory(Category *c);

		/** compute the search index **/
		void compute();

		/** create a JSON output of the search index and output it to files. **/
		json_spirit::mObject outputJson(std::string basePath, std::string serverBasePath,const std::string* server_url, unsigned int version);
		
		/** constructor **/
		ExactSearcher(std::string name, std::string fieldname, ElementType _type, bool transmit, short hashsize, short folderhashsize);
		
	};
}

#endif