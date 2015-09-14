#ifndef __CRUNCH_HPP__
#define __CRUNCH_HPP__

/**
* @author Marc Morel
* @copyright Marc Morel, 2015
*/



#include "reader.hpp"
#include "crunchstruct.hpp"
#include "crunchconfig.hpp"
#include "category.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/regex.hpp>
#include <boost/thread.hpp>

#define PHOTOPACKNAME "photopack-"
#define THUMBPACKNAME "thumb-"
#define FILEPACKNAME "file-"
#define INDEXPATH "catalogue/"
#define INDEXFILENAME "index.json"
#define CATPATH "categories/"
#define CATFILEPREFIX "cat-"
#define SEARCHPATH "search-"
#define CATALOGPREFIX "catalogue-"
#define MODFILEPREFIX "mod-"
#define PRDFILEPREFIX "prd-"
#define JSONEXT ".json"
#define MAXTHREADS 10
#define MAX_ITEM_PER_JSON_FILE 50
#define MAX_JSON_FILE_PER_FOLDER 16
#define MAX_PICTURE_PER_FOLDER 120
#define CRUNCH_VERSION_1 1
/**
* The class crunch is the main class for the crunch process. It will parse the
* data file, constructs the whole model/product/reference tree & the category tree,
* download pictures of products & output everything to JSON files, grouping products
* using hash key group parameters.
* All crunch parameters are set up using a JSON file. 
*/
namespace crunch
{


	class Product;
	class CrunchElement;
	class Model;
	class Reference;
	class Picture;
	class CrunchFile;

	class Reader;
	class Crunch
	{

		Reader* m_reader;

		/**
		* m_models will hold all models in a BOOST provided hash map of models.
		*/
		boost::unordered_map <std::string, Model*> m_models;

		/**
		* m_pictures will hold all pictures in a BOOST provided hash map of Pictures vectors.
		*/
		boost::unordered_map <std::string, Picture*> m_pictures;

		/**
		* m_files will hold all files in a BOOST provided hash map of Files vectors.
		*/
		boost::unordered_map <std::string, CrunchFile*> m_files;



		/** these ByHashKeys arrays will store pointers to vector of models, products, references, pictures. **/
		/** for each of these arrays, the hashkey will be the entry key. **/
		/** All the items with the same type and same hashkey will be stored in the same file **/
		boost::unordered_map <std::string, std::vector<CrunchElement*>* > 		m_modelsByHashKeys;
		boost::unordered_map <std::string, std::vector<CrunchElement*>* > 		m_productsByHashKeys;
		boost::unordered_map <std::string, std::vector<Picture*>* > 			m_picturesByHashKeys;

		/* mutex to protect the access to the picture list (downlad is handled by several threads simultaneously)*/
		boost::mutex m_picturesMutex;
		boost::mutex m_filesMutex;

		/** boolean to check if init procedure of downloads has been done **/
		bool m_picturesDownloadInit;
		bool m_filesDownloadInit;

		/* iterator of pictures to download */
		boost::unordered_map <std::string, Picture*>::const_iterator m_picturesIterator;

		/* iterator of files to download */
		boost::unordered_map <std::string, CrunchFile*>::const_iterator m_filesIterator;



		/**
		* m_rootCategory is the root of the category tree.
		* It is automatically constructed in the Crunch constructor
		*/
		crunch::Category m_rootCategory;

	

		/**
		* m_indexJson is the JSON object to be output in the index.json file.
		*/

		json_spirit::mObject m_indexJson; 

		/**
		* m_serverIndexJson is the JSON object to be output in the index.json file, for the
		* crunch part that will stay on the server.
		*/
		json_spirit::mObject m_serverIndexJson; 

		/**
		* CONFIGURATION MEMBERS
		* will hold different config values from the json config file
		*/
		CrunchConfig m_crunchConfig;

		/**
		* path files
		*/
		std::string m_crunchOutputDirectory; // constructed from crunchparam : tmp_dir/instance_name/

		/**
		* status
		*/
		std::vector<std::string> m_newResources;
		std::vector<std::string> m_updatedResources;
		std::vector<std::string> m_outdatedResources;

	protected:

		/*** FIELDS TO SEARCH FOR IN XML OR CSV FILE **/
		boost::unordered_map <std::string, CrunchField*> m_fieldList; // fields to search for every item
		boost::unordered_map <std::string, CrunchField*> m_multipleFieldList; // fields that may appear more than once for each item
		boost::unordered_map <std::string, CrunchField*> m_customProductFieldList; // custom fields (no google merchant spec) for product
		boost::unordered_map <std::string, CrunchField*> m_customModelFieldList; // custom fields (no google merchant spec) for model
		boost::unordered_map <std::string, CrunchField*> m_customReferenceFieldList; // custom fields (no google merchant spec) for reference
		boost::unordered_map <std::string, CrunchField*> m_downloadProductFieldList; // custom fields (no google merchant spec) for product
		boost::unordered_map <std::string, CrunchField*> m_downloadModelFieldList; // custom fields (no google merchant spec) for model
	
		/**collection to count **/
		std::vector< FieldPValueCountMap > m_itemsDistinctValueCount;
	
		void populateItem(Item& item);
		void addPictures( Item& item, Model* m, Product* p, Reference* r);
		void addOnePicture( const std::string url,Model* m, Product* p, Reference* r);
		void addFiles(Item& item, Model* m, Product* p, Reference* r);
		void addOneFile(const std::string fileName, const std::string fileUrl, CrunchElement* elt);

		/**
		* retrieve field array from current row in source file and apply optional regexp
		*/
		std::vector<std::string> getItemArray(const CrunchField* field);

		json_spirit::mObject getStatJson();

		json_spirit::mObject* getIndexJson();
		json_spirit::mObject* getServerIndexJson();
		bool compareResource(const std::string oldR, const std::string newR, const std::string rscName);

	
	public:

		/**
		* Default constructor
		* Will create a root category
		*/
		Crunch();

		~Crunch();

		/**
		* Initialization function
		* @param const std::string& JSON parameter absolute path (in local filesystem)
		*/
		void init(const std::string& jsonparam);


		/**
		* Main crunch function
		* 
		*/
		void start();
		

		/**
		* processes each source file row.
		* create product, model, references & categories if needed.
		*/
		void itemLoop();


		/**
		* generate all JSON files for the crunch
		*/
		void outputJson();

		/** retrieve crunch field definition by its name **/
		CrunchField* getCrunchFieldByName(std::string& to_search);
		
		/**
		* retrieve field value from current row in source file and apply optional regexp
		*/
		std::string getItemValue(const CrunchField* field);	

		/**
		* after model/product or reference creation, these function are called
		* to break down all items into hash.
		*/
		void groupItemsByHashKeys();

		/** 
		* static function, main function of a thread.
		* will output JSON files for model, product or reference
		*/
		static void outputJsonHashGroup(Crunch* crunch, boost::unordered_map <std::string, std::vector<CrunchElement*>* >& hashgroup,std::string filePrefix);

		/** 
		* static function, main function of a thread.
		* will output JSON files for a category
		*/
		static void outputJsonCategory(Crunch* crunch);

		/** 
		* static function, main function of a thread.
		* will output JSON files for search indexes
		*/
		static void outputJsonSearches(Crunch* crunch);

		/** 
		* will output main JSON file: index.json
		*/
		void outputJsonIndex();

		/** 
		* will output main JSON status file, whose path is in json config
		*/
		void outputJsonStatus();


		/** 
		* will be called for each new category, to be indexed.
		*/
		void addCategoryToSearchIndex(Category* c);
		void insertElementIntoHash(	boost::unordered_map <std::string, std::vector<CrunchElement*>* >& hashTable,crunch::CrunchElement* elt);
		
		/** 
		* add new model into Hashtable
		*/
		void insertModelIntoHash(crunch::Model* m);


		/** 
		* add new product into Hashtable
		*/
		void insertProductIntoHash(crunch::Product* p);


		/** 
		* distribute pictures to a download function.
		* threadsafe function. 
		*/
		Picture* getNextPictureToDownload();	
		CrunchFile* getNextFileToDownload();

		/* 
		* launch pictures  download process
		*/
		void downloadPictures();
		void downloadFiles();

		/*
		* static function, main function of a thread.
		* Download pictures by calling getNextPictureToDownload to get a new URL to process
		* Concurrent downloads are handled by instantiating multiple threads of this function
		*/
		static void downloadPictureThread(Crunch* crunch,int serial);
		static void downloadFileThread(Crunch* crunch,int serial);

		/*
		* apply transformation to product info (value) based on field configuration (field)
		*/
		std::string applyFieldRegexp(const CrunchField* field, std::string value);


		/*
		* compute diff between directory & old-directory
		* output results in diffStatus
		*/

		void computeDiff();

		/*
		* computes size of hashkey to group resources, based on the number of item
		* found in the product feed
		*/
		void computeHashKeySizes();
		static unsigned short computeHashKeyFileSizeFromItemCount(unsigned long itemcount);
		static unsigned short computeHashKeyFolderSizeFromItemCount(unsigned long itemcount);
		static unsigned short computeHashKeyPictureSizeFromItemCount(unsigned long itemcount);

        bool getVerifyConfig();

        void locateCategoryThumbnail();
	};
}




#endif