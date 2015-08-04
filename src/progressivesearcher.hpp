#ifndef __PROGRESSIVESEARCHER_HPP
#define __PROGRESSIVESEARCHER_HPP
#include "searcher.hpp"
#include "json_spirit/json_spirit.h"
#include <vector>

/**
* @author Marc Morel
* @copyright Procheo, 2014
*/

#define PROGRESSIVE_MAX_DEPTH 15
#define MAX_COUNT_TO_EXPAND 80

/**
* the class ProgressiveSearcher derives form Searcher and is used to perform
* a progressive search on a specifid field (progressive = tree-based)
* Search results are stored in a hashmap of vectors.
* The key of the hashmap is the MD5 of the search fields truncated to m_hashSize
**/

namespace crunch
{
	//will store all crunch elements for a Nth letter
	struct progressiveSearcherTreeItem
	{
		std::string m_value;
		std::vector< std::pair< std::string, CrunchElement*> > m_crunchElements;
		std::vector<std::pair< std::string, Category*> > m_categories; 
		boost::unordered_map <char,progressiveSearcherTreeItem*>  m_nextLetter;
		unsigned long m_nextLevelLeavesCount;
		unsigned long m_nextLevelIntersectionsCount;
		progressiveSearcherTreeItem(){ m_nextLevelLeavesCount = 0; m_nextLevelIntersectionsCount =0; };
		std::pair<unsigned long, unsigned long> getSubCount();
		json_spirit::mObject dumpJsonFiles(const std::string basePath, const std::string server_path, const std::string* server_url, bool transmit, ElementType type, unsigned int version);
		json_spirit::mObject getJsonRepresentation(std::pair< std::string, CrunchElement*> elt, const std::string* server_url, ElementType type);
		json_spirit::mObject getJsonRepresentation(std::pair< std::string, Category*> elt, const std::string* server_url);
		void dumpNext(std::vector< std::pair<std::string, CrunchElement*>  >& eltArray, 
                                            std::vector< std::pair< std::string, Category*>  >& catArray,
                                            const std::string* server_url,
                                            unsigned int version);
	};

	class ProgressiveSearcher : Searcher
	{

		/** storage **/
        boost::unordered_map <std::string, progressiveSearcherTreeItem*> m_tree;

		/** raw list of elements to index **/
		std::vector<std::pair<std::string,CrunchElement*> > m_crunchElements;

        /** private storage of pointers to searched elements **/
        boost::unordered_map <std::string, std::vector<std::pair<std::string,CrunchElement*> >* > m_crunchElementPairsByHashKeys;
		
		/** size of hashkey used for the map **/
		short m_hashSize;

		/**keys to index */
		std::vector<std::string> m_fieldsNameVector;
		std::vector<CrunchField*> m_crunchFieldsVector;

		/** size of hashkey to group json file in folders **/
		short m_folderHashSize;


	public:
		/* override parent method */
		bool init(Crunch* crunch);
		
		/**split string **/
		stringvector splitString(std::string& str);
		progressiveSearcherTreeItem* getTreeItem(std::string& str);
        /** get a string representation of m_type **/
        std::string typeToString();

		/** add a new element in the search map **/
		bool addElement(Model* m, Product *p, Reference *r,  Crunch *crunch);
		bool addCategory(Category *c);

		/** compute the search index **/
		void compute();

		/** create a JSON output of the search index and output it to files. **/
		json_spirit::mObject outputJson(std::string basePath,std::string serverPath, const std::string* server_url, unsigned int version);
		
		/** constructor **/
		ProgressiveSearcher(std::string name, std::string fieldname, ElementType _type, bool transmit, short hashsize, short folderhashsize);
		
	};
}

#endif