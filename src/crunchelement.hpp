#ifndef __CRUNCHELEMENT_HPP__
#define __CRUNCHELEMENT_HPP__

#define JSON_ID                         0x01  //0000000000000001
#define JSON_CHILDREN                   0x02  //0000000000000010
#define JSON_PATH                       0x04  //0000000000000100
#define JSON_TITLE                      0x08  //0000000000001000
#define JSON_PICTURE                    0x10  //0000000000010000
#define JSON_PRICE                      0x20  //0000000000100000
#define JSON_DESCRIPTION                0x40  //0000000001000000
#define JSON_PICTURES                   0x80  //0000000010000000
#define JSON_CUSTOM                     0x100 //0000000100000000
#define JSON_SUBCAT                     0x200 //0000001000000000
#define JSON_SEARCHPATH                 0x400 //0000010000000000
#define JSON_CAT_CUSTOM                 0x800 //0000100000000000
#define JSON_FILES		                0x1000//0001000000000000

#define JSON_BASIC                      ( JSON_TITLE | JSON_PICTURE | JSON_PRICE | JSON_DESCRIPTION ) //01111000 //x78

#define JSON_ALL_EXCEPT_PATH            ( 0xFFFF ^ JSON_PATH ) //11111011
#define JSON_ALL_EXCEPT_CHILDREN        ( 0xFFFF ^ (JSON_CHILDREN | JSON_SEARCHPATH))      //11111001 //xFFF9
#define JSON_ALL_EXCEPT_SEARCHPATH      ( 0xFFFF ^ JSON_SEARCHPATH ) //11111011

#define JSON_ALL                        0xFFFF //11111111

/**
* @author Marc Morel
* @copyright Procheo, 2014
*
* CrunchElement is the parent class of Model, Product, Reference.
* It is the main class used to store structured product information 
*/



#include <vector>
#include <string>
#include "crunchstruct.hpp"
#include "picture.hpp"
#include "crunchfile.hpp"
#include "boost/unordered_map.hpp"
#include "json_spirit/json_spirit.h"




namespace crunch {
	
	class Crunch;
    class Model;
	class Product;
	class Reference;
	class Category;

 	typedef unsigned long CrunchDetailLevel;
 	typedef enum { model, product, reference, categorymodel} ElementType;

	class CrunchElement
	{
	protected:
			ElementType m_type; //type of element (product, model, reference)
			Item m_data;  // main crunch element values (price, name, description, ... ) 
			std::string m_filePrefix; // prefix of json file (local path in resource) : ex prod/ or mod/
			std::string m_filePath;   // local path & filename in resource ex prod/5f2.json
			std::string m_resource;   // resource name ex catalogue-5f.json
            std::string m_searchMatchValue;// the value searched in search files 
			unsigned short m_fileHashKeySize; //size of file hahskey ( ex 3 for 5F2.json)
			unsigned short m_resourceHashKeySize; //size of resource hashkey (ex 2 for catalogue-5f.json)
			Crunch* m_crunch; //pointer to main crunch instance
			char m_idMd5[33]; // MD5 of crunchelement ID
			std::vector<std::string> m_picturesURL; // list of pictures URL (used to speed up the unicity test of pictures in m_pictures)
			std::vector<Picture*> m_pictures; //list of pictures
			boost::unordered_map <std::string, CrunchFile*> m_files; //list of pictures
			Category* m_category;	// pointer to category 
			
            
        public:
			virtual json_spirit::mObject outputJson(CrunchDetailLevel detailLevel = JSON_ALL,const std::string* server_url = NULL, int crunchVersion = -1,bool isRootOutput = false);
			json_spirit::mObject  outputFilesJson(const std::string* server_url,int crunchVersion);
			virtual json_spirit::mObject outputCustomJson(int crunchVersion ) = 0;
			json_spirit::mObject buildJsonResourcePathPair();
            void appendJsonResourceSearchObject(json_spirit::mObject& obj);
            std::string getHash(const unsigned short length);
			void constructFilePath();
			void addPicture(Picture*);
			void addFile(std::string fileName, CrunchFile*);
			void setHashKeySize(unsigned short resourcekeysize, unsigned short filekeysize);
            void setMatchValue(std::string matchValue){m_searchMatchValue = matchValue;};
            const Item* getItem();
            std::vector<Picture*>* getPictures();
            CrunchElement(); //Default constructor
            ~CrunchElement();
	};

} //namespace crunch

#include "model.hpp"
#include "product.hpp"
#include "reference.hpp"

#endif