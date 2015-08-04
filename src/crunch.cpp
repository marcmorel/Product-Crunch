#include "crunch.hpp"
#include "searcher.hpp"
#include <string>
#include <iostream>
#include <fstream>
#include <ctime>
#define BOOST_FILESYSTEM_NO_DEPRECATED

#include "boost/unordered_map.hpp"	//associative arrays
#include <boost/regex/v4/regex.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/locale.hpp>
#include "version.hpp"
#include "category.hpp"
#include "crunchfile.hpp"
#include "util.hpp"
#include "xmlreader.hpp"
#include "csvreader.hpp"
#include "model.hpp"
#include "product.hpp"
#include "reference.hpp"
using namespace crunch;
/**
* Default constructor
* Will create a root category
*/ 
crunch::Crunch::Crunch() : m_rootCategory("root","root",CATPATH, NULL)
{
    curl_global_init(CURL_GLOBAL_ALL );
}

crunch::Crunch::~Crunch()
{
delete crunch::CSVReader::This;

#ifdef __DEBUG__
 std::cout << "debut  destructor Crunch memory freed ]\n"; 
#endif

  /*  m_models.clear();
    m_pictures.clear();*/
#ifdef __DEBUG__
 std::cout << "free memory of m_models ]\n"; 
#endif
	//free memory of m_models
    for( boost::unordered_map <std::string, Model*>::const_iterator m = m_models.begin(); 
			m != m_models.end(); m++)
     		delete m->second;
#ifdef __DEBUG__
 std::cout << " m_models.clear() ]\n"; 
#endif
    m_models.clear();

#ifdef __DEBUG__
 std::cout << " end m_models.clear() ]\n"; 
#endif
    //free memory of m_pictures
    for( boost::unordered_map <std::string, Picture*>::const_iterator p = m_pictures.begin(); 
            p != m_pictures.end(); p++)
            delete p->second;
    m_pictures.clear();

    //free m_modelsByHashKeys
    for( boost::unordered_map <std::string, std::vector<CrunchElement*>* >::iterator h = m_modelsByHashKeys.begin(); 
        h != m_modelsByHashKeys.end(); h++)
    {

    h->second->clear();
    delete h->second;
    }
     m_modelsByHashKeys.clear();


    //free m_productsByHashKeys
    for( boost::unordered_map <std::string, std::vector<CrunchElement*>* >::iterator h = m_productsByHashKeys.begin(); 
        h != m_productsByHashKeys.end(); h++)
    {
    h->second->clear();
    delete h->second;
    }
    
    m_productsByHashKeys.clear();

    //free m_productsByHashKeys
    for( boost::unordered_map <std::string, std::vector<Picture*>* >::iterator h = m_picturesByHashKeys.begin(); 
        h != m_picturesByHashKeys.end(); h++)
    {

    h->second->clear();
    delete h->second;
    }

    m_picturesByHashKeys.clear();

    //free memory of m_fieldList
    for( boost::unordered_map <std::string, CrunchField*>::const_iterator cf = m_fieldList.begin(); 
            cf != m_fieldList.end(); cf++)
            delete cf->second;

    m_fieldList.clear();

    //free memory of m_multipleFieldList
     for( boost::unordered_map <std::string, CrunchField*>::const_iterator cf = m_multipleFieldList.begin(); 
            cf != m_multipleFieldList.end(); cf++)
            delete cf->second;

    m_multipleFieldList.clear();

    //free memory of m_customProductFieldList
     for( boost::unordered_map <std::string, CrunchField*>::const_iterator cf = m_customProductFieldList.begin(); 
            cf != m_customProductFieldList.end(); cf++)
            delete cf->second;
    
    m_customProductFieldList.clear();

    //free memory of m_customModelFieldList
     for( boost::unordered_map <std::string, CrunchField*>::const_iterator cf = m_customModelFieldList.begin(); 
            cf != m_customModelFieldList.end(); cf++)
            delete cf->second;

    m_customModelFieldList.clear();

     //free memory of m_customModelFieldList
     for( boost::unordered_map <std::string, CrunchField*>::const_iterator cf = m_customReferenceFieldList.begin(); 
            cf != m_customReferenceFieldList.end(); cf++)
            delete cf->second;

    m_customReferenceFieldList.clear();
    
        curl_global_cleanup();

     #ifdef __DEBUG__
 std::cout << "End  destructor Crunch memory freed ]\n"; 
#endif
}




/**
* Launch JSON parsing method and prepare fields to retrieve from source file
*/
void crunch::Crunch::init(const std::string& jsonparam)
{

	m_crunchConfig.parseConfigFile(jsonparam);

	Util::log("Creation of output directory\n", logTRACE);

	
	m_crunchOutputDirectory=m_crunchConfig.m_directory;
	crunch::Util::selectOrCreateDirectory(m_crunchOutputDirectory);

	if(m_crunchConfig.m_server_directory.length())
		crunch::Util::selectOrCreateDirectory(m_crunchConfig.m_server_directory);

	Util::log(" Prepare mandatory fields list \n", logTRACE);

	//prepare mandatory fields list
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:id",new CrunchField("g:id","g:id",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("title",new CrunchField("title","title",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("description",new CrunchField("description","description",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:google_product_category",new  CrunchField("g:google_product_category","g:google_product_category",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:product_type",new CrunchField("g:product_type","g:product_type",false)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("link",new CrunchField("link","link",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:image_link",new CrunchField("g:image_link","g:image_link",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:condition",new CrunchField("g:condition","g:condition",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:availability",new CrunchField("g:availability","g:availability",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:price",new CrunchField("g:price","g:price",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:sale_price",new CrunchField("g:sale_price","g:sale_price",false)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:gtin",new CrunchField("g:gtin","g:gtin",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:brand",new CrunchField("g:brand","g:brand",false)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:mpn",new CrunchField("g:mpn","g:mpn",false)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:item_group_id",new CrunchField("g:item_group_id","g:item_group_id",false)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:size",new CrunchField("g:size","g:size",true)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:material",new CrunchField("g:material","g:material",false)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:pattern",new CrunchField("g:pattern","g:pattern",false)));
	m_fieldList.insert(std::make_pair<std::string,CrunchField* >("g:color",new CrunchField("g:color","g:color",false)));
	m_multipleFieldList.insert(std::make_pair<std::string,CrunchField* >("g:additional_image_link",new CrunchField("g:additional_image_link","g:additional_image_link",false)));

	
	Util::log(" Prepare proprietary fields list ", logTRACE);
	

	//add proprietary fields for model
	for (	std::vector <std::string>::iterator pos = m_crunchConfig.m_modelCustomFields.begin();
     		pos != m_crunchConfig.m_modelCustomFields.end();
     		++pos)
	{
	   m_customModelFieldList.insert(std::make_pair<std::string,CrunchField*>(*pos,new CrunchField(*pos,*pos,false)));
	}
	//add proprietary fields for product
	for (	std::vector <std::string>::iterator pos = m_crunchConfig.m_productCustomFields.begin();
     		pos != m_crunchConfig.m_productCustomFields.end();
     		++pos)
	{
	   m_customProductFieldList.insert(std::make_pair<std::string,CrunchField*>(*pos,new CrunchField(*pos,*pos,false)));
	}
	//add proprietary fields for reference
	for (	std::vector <std::string>::iterator pos = m_crunchConfig.m_referenceCustomFields.begin();
     		pos != m_crunchConfig.m_referenceCustomFields.end();
     		++pos)
	{
	   m_customReferenceFieldList.insert(std::make_pair<std::string,CrunchField*>(*pos,new CrunchField(*pos,*pos,false)));
	}

	Util::log(" Prepare download fields list ", logTRACE);

	//add download fields for model
	for (	std::vector <std::string>::iterator pos = m_crunchConfig.m_modelDownloadFields.begin();
     		pos != m_crunchConfig.m_modelDownloadFields.end();
     		++pos)
	{
	   m_downloadModelFieldList.insert(std::make_pair<std::string,CrunchField*>(*pos,new CrunchField(*pos,*pos,false)));
	}
	//add download fields for product
	for (	std::vector <std::string>::iterator pos = m_crunchConfig.m_productDownloadFields.begin();
     		pos != m_crunchConfig.m_productDownloadFields.end();
     		++pos)
	{
	   m_downloadProductFieldList.insert(std::make_pair<std::string,CrunchField*>(*pos,new CrunchField(*pos,*pos,false)));
	}





	//prepare field name replacements and regexp

	// first, replacement names
	std::string specificationName, replacementName;
	for (	boost::unordered_map <std::string, std::string> ::const_iterator pos = m_crunchConfig.m_fieldsNameReplacement.begin();
     		pos != m_crunchConfig.m_fieldsNameReplacement.end();
     		++pos )
	{
		specificationName=pos->first;
		replacementName=pos->second;


		if(m_fieldList.find(specificationName) != m_fieldList.end())
			m_fieldList.find(specificationName)->second->m_replacementName=replacementName;
		if(m_multipleFieldList.find(specificationName) != m_multipleFieldList.end())
			m_multipleFieldList.find(specificationName)->second->m_replacementName=replacementName;
	}

	//regexp 
	std::string fieldName, regexp;
	for (	boost::unordered_map <std::string, std::string> ::const_iterator pos = m_crunchConfig.m_fieldsRegexp.begin();
     		pos != m_crunchConfig.m_fieldsRegexp.end();
     		++pos)
	{
		fieldName=pos->first;
		regexp=pos->second;

		//bind the proper regex to a mandatory field
		if(m_fieldList.find(fieldName) != m_fieldList.end())
		{
			m_fieldList.find(fieldName)->second->m_hasRegexp=true;
			m_fieldList.find(fieldName)->second->m_regexp=boost::regex(regexp);
		}

		//bind the proper regex to a multiple field
		if(m_multipleFieldList.find(fieldName) != m_multipleFieldList.end())
		{
			m_multipleFieldList.find(fieldName)->second->m_hasRegexp=true;
			m_multipleFieldList.find(fieldName)->second->m_regexp=boost::regex(regexp);
		}

		//bind the proper regex to a custom model field
		if(m_customModelFieldList.find(fieldName) != m_customModelFieldList.end())
		{
			m_customModelFieldList.find(fieldName)->second->m_hasRegexp=true;
			m_customModelFieldList.find(fieldName)->second->m_regexp=boost::regex(regexp);
		}

		//bind the proper regex to a custom product field
		if(m_customProductFieldList.find(fieldName) != m_customProductFieldList.end())
		{
			m_customProductFieldList.find(fieldName)->second->m_hasRegexp=true;
			m_customProductFieldList.find(fieldName)->second->m_regexp=boost::regex(regexp);
		}

		//bind the proper regex to a custom reference field
		if(m_customReferenceFieldList.find(fieldName) != m_customReferenceFieldList.end())
		{
			m_customReferenceFieldList.find(fieldName)->second->m_hasRegexp=true;
			m_customReferenceFieldList.find(fieldName)->second->m_regexp=boost::regex(regexp);
		}

		//bind the proper regex to a download model field
		if(m_downloadModelFieldList.find(fieldName) != m_downloadModelFieldList.end())
		{
			m_downloadModelFieldList.find(fieldName)->second->m_hasRegexp=true;
			m_downloadModelFieldList.find(fieldName)->second->m_regexp=boost::regex(regexp);
		}

		//bind the proper regex to a download product field
		if(m_downloadProductFieldList.find(fieldName) != m_downloadProductFieldList.end())
		{
			m_downloadProductFieldList.find(fieldName)->second->m_hasRegexp=true;
			m_downloadProductFieldList.find(fieldName)->second->m_regexp=boost::regex(regexp);
		}

	}

	//add fields whose value should be counted as unique
	for (	std::vector <std::string>::iterator pos = m_crunchConfig.m_enumFields.begin();
     		pos != m_crunchConfig.m_enumFields.end();
     		++pos)
	{
		//first, locate crunchfields and add to list of value to count
		CrunchField* field = getCrunchFieldByName(*pos);
		if(field)
		{
			ValueCountMap* thisvalueCountMap = new ValueCountMap;
			FieldPValueCountMap pair =  FieldPValueCountMap(field,thisvalueCountMap);
			m_itemsDistinctValueCount.push_back(pair);
		}

	}


	//field list ended. Let's init the search indexes for models
	for (	std::vector <Searcher*>::iterator pos = m_crunchConfig.m_searchers.begin();
     		pos != m_crunchConfig.m_searchers.end();
     		++pos)
	{
		if(!(*pos)->init(this))
		{
			throw std::runtime_error(std::string("Search key ")+(*pos)->getFieldName()+std::string(" has not been found in field list.\n"));
		}
	}


}



CrunchField* crunch::Crunch::getCrunchFieldByName(std::string& to_search)
{
	if(m_fieldList.find(to_search) != m_fieldList.end())
		{
			return m_fieldList.find(to_search)->second;
		}
		else if(m_customModelFieldList.find(to_search) != m_customModelFieldList.end())
		{
			return m_customModelFieldList.find(to_search)->second;
		}
		else if(m_customProductFieldList.find(to_search) != m_customProductFieldList.end())
		{
			return m_customProductFieldList.find(to_search)->second;
		}
		else if(m_customReferenceFieldList.find(to_search) != m_customReferenceFieldList.end())
		{
			return m_customReferenceFieldList.find(to_search)->second;
		}
	return NULL;
}

/**
* Itemloop is the main function, used to process each line of source file 
*/

void crunch::Crunch::itemLoop()
{
		//populate item fields from current sourcefile row.
		Item currentItem;
		populateItem(currentItem);

		//choose right id to group models and products
		//current rule for model is : g_item_group_id >> g_id for model
		//current rule for product is :  g_color >> g_id for product

		if(currentItem.m_g_item_group_id.length())
			currentItem.m_modelGroup=currentItem.m_g_item_group_id; 
		else currentItem.m_modelGroup=currentItem.m_g_id; 

		if(currentItem.m_g_color.length())
			currentItem.m_productGroup=currentItem.m_modelGroup + currentItem.m_g_color; 
		else currentItem.m_productGroup=currentItem.m_modelGroup + currentItem.m_g_id; 

		//find or create category
		bool isNewCategory = false;
		Category* c = m_rootCategory.findOrCreateCategory(	currentItem.m_g_google_product_category,
															m_crunchConfig.m_category_separator,
															isNewCategory,
															std::string(CATPATH),
															this);
		
		//find or create model
		bool isNewModel = false;
		Model* m=NULL;
		if(m_models.count(currentItem.m_modelGroup)>0)
			m=m_models[currentItem.m_modelGroup];
		else
			{
				m=new Model(currentItem,this,c);
				m_models.insert(std::make_pair<std::string,Model*>(currentItem.m_modelGroup,m));
				c->addModel(m);
				isNewModel = true;
			}

		//find or create product
		bool isNewProduct = false;
		Product *p=m->findOrAddProduct(currentItem, isNewProduct,c);
	
		//create reference
		Reference *r=p->addReference(currentItem,c);
			
		//handle pictures
		if(currentItem.m_g_image_link.length())
			addPictures(currentItem,m,p,r);

		//handle files
		addFiles(currentItem,m,p,r);

		//insert into search indexes
		for(unsigned int searchers=0;searchers<m_crunchConfig.m_searchers.size(); searchers++)
		{
			if( (m_crunchConfig.m_searchers[searchers]->m_type == model)
				&& ( !isNewModel))
				continue;
			if( (m_crunchConfig.m_searchers[searchers]->m_type == product)
				&& ( !isNewProduct))
				continue;
			m_crunchConfig.m_searchers[searchers]->addElement(m,p,r,this);
		}	

		//count new values if needed
		for(unsigned int valuecount=0;valuecount<m_itemsDistinctValueCount.size(); valuecount++)
		{
			std::string value = getItemValue(m_itemsDistinctValueCount[valuecount].first);
			if(value.length() == 0) continue;
			(*m_itemsDistinctValueCount[valuecount].second)[value]++;
		}


}

void crunch::Crunch::addCategoryToSearchIndex(Category *c)
{
	//insert into search indexes
		for(unsigned int searchers=0;searchers<m_crunchConfig.m_searchers.size(); searchers++)
		{
			m_crunchConfig.m_searchers[searchers]->addCategory(c);
		}	

}


/**
* main function. will parse, compute and then output crunch result
*/ 
void crunch::Crunch::start()
{

	/** 
	* 
	* step 1 : download source file
	*
	**/

	
	switch(m_crunchConfig.m_format)
	{
		case xml: 
        m_reader = (crunch::Reader*) new crunch::XMLReader(&m_crunchConfig);
        Util::log("XMLReader has been instanciated\n", logTRACE); 
        break;
		case csv: m_reader = (crunch::Reader*) new crunch::CSVReader(&m_crunchConfig);
        Util::log("CSVReader has been instanciated\n", logTRACE); 
        break;
	}

    Util::log("source file download started. Let's roll\n", logINFO); 
        

	if(!m_reader->locateSourceFile())
	{
		 Util::log("Download error\n", logERROR);
		throw std::runtime_error("Download error");	
	}
    
    Util::log("Download done\n", logINFO); 
    
	Util::log("Start parsing file\n", logTRACE); 
    if(!m_reader->openSourceFile())
    {
    	Util::log("Wrong format at init\n", logERROR);
		throw std::runtime_error("Wrong format at init");
	}
	Util::log( "Parsing ok. Starting model/product construction\n", logINFO);
	


	/** 
	* 
	* step 2 : main crunch loop
	*
	**/

	unsigned long itemCount=0;
	while(m_reader->hasNextItem())
	{
		itemLoop();

		/** display progress notification **/
		if( !( (itemCount++) %1000) )
			Util::log(std::string(" Item count :")+Util::to_string(itemCount)+std::string("\n"),logINFO); 

        if(m_crunchConfig.iMaxItemsToRead >= 0)
            if (itemCount > m_crunchConfig.iMaxItemsToRead)
                break;
	}

	std::string logmsg = std::string("\n")
					+Util::to_string(Model::m_modelCounter) + std::string(" modèles créés \n")
					+Util::to_string(Product::m_productCounter) + std::string(" produits créés \n")
					+Util::to_string(Reference::m_referenceCounter) + std::string(" reférences créés \n")
					+Util::to_string(Category::m_counter) + std::string(" catégories créés \n")
					+Util::to_string(Picture::m_counter) + std::string(" photos créés \n");
	Util::log("Creating search indexes \n ",logINFO); 
	
	computeHashKeySizes();

	/** group product and models by resourcehashkey and file hashkey **/

	groupItemsByHashKeys();

	/**
	* compute search indexes 
	*/
	for(unsigned int searchers=0;searchers<m_crunchConfig.m_searchers.size(); searchers++)
	{
		m_crunchConfig.m_searchers[searchers]->compute();
		std::string logMsg("Search index ");
		logMsg+=Util::to_string(searchers +1)+std::string(" of ")+Util::to_string(m_crunchConfig.m_searchers.size())
				+std::string(" done\n");
		Util::log(logMsg,logINFO);
	}	


	Util::log( "Start downloading pictures\n", logINFO);
	
	/** 
	* 
	* step 4 : download product pictures (multithread process)
	*
	**/
	downloadPictures();

	Util::log( "Start downloading files\n", logINFO);
	
	downloadFiles();


	Util::log("Starting JSON output\n", logINFO);
	
	/** step 4.5 : locate category thumbnail 
	*/

	locateCategoryThumbnail();
	#ifdef __DEBUG__
		for( boost::unordered_map <std::string, json_spirit::mObject>::iterator h = m_crunchConfig.m_categoryParameter.begin();
				h != m_crunchConfig.m_categoryParameter.end(); h++)
				Util::log(std::string("Category Parameter name ")+h->first+std::string("\n"));
	#endif
	
	/** 
	* 
	* step 5 : output JSON files in TMP directory
	*
	**/	
	outputJson();


	/**
	*
	* step 6 : compute diff with old directory
	*
	*/

	computeDiff();

	/** 
	* step 7 : output json status
	*
	*/

	outputJsonStatus();
}


/** will triger JSON output of the whole crunch. Launch one thread per element type (product and/or model) **/
void crunch::Crunch::outputJson()
{
	
	//create threads for product and model
	boost::thread  modelThread;
	if( m_crunchConfig.m_generate_model_file)
		modelThread= boost::thread(Crunch::outputJsonHashGroup,this,m_modelsByHashKeys,MODFILEPREFIX);

	boost::thread productThread;
	if( m_crunchConfig.m_generate_product_file)
		productThread=boost::thread(Crunch::outputJsonHashGroup,this,m_productsByHashKeys,PRDFILEPREFIX);
	
	boost::thread  categoryThread;
	if( m_crunchConfig.m_generate_category_file)
		categoryThread=boost::thread(Crunch::outputJsonCategory,this);

	boost::thread  searchesThread;
	searchesThread=boost::thread(Crunch::outputJsonSearches,this);

	// launch threads.
	if( m_crunchConfig.m_generate_model_file)
            if(modelThread.joinable())
                modelThread.join();
	if( m_crunchConfig.m_generate_product_file) 
       if(productThread.joinable())
             productThread.join();
	if( m_crunchConfig.m_generate_category_file) 
        if(categoryThread.joinable())
            categoryThread.join();
	
    if(searchesThread.joinable())
        searchesThread.join();

	//generate index file
	outputJsonIndex();
}


json_spirit::mObject crunch::Crunch::getStatJson()
{
	// current date/time based on current system
    time_t now = time(0);
   
    // convert now to string form
    std::string szNow = ctime(&now);

    //add infos
    json_spirit::mObject objInfos;
    json_spirit::mObject objTotals;
    json_spirit::mObject objBuild;

    //infos.build section
    objBuild["version"]=__BINARY_CRUNCH_VERSION;
    objBuild["detail"]= __DETAIL__CRUNCH_VERSION;
    objBuild["generation_date"]= szNow;
    objInfos["build"]=objBuild;

    //infos.totals section
    objTotals["mod"]=Model::m_modelCounter;
    objTotals["prd"]=Product::m_productCounter;
    objTotals["ref"]=Reference::m_referenceCounter;
    objTotals["cat"]=Category::m_counter;
    objTotals["pic"]=Picture::m_counter;

    objInfos["totals"]=objTotals;

    return objInfos;

}

/**
*
* will output the main json index fileS onto the server.
* First will be the index synchronised onto the devices
* Second will the index for the remote version of the crunch (server based)
*/
void crunch::Crunch::outputJsonIndex()
{
	
   	if(!m_crunchConfig.m_transmit_catalog) //catalog files stay on the server
		m_indexJson["api"]=m_crunchConfig.m_server_url;

	if( m_crunchConfig.m_generate_category_file) {
		//add root categories
		json_spirit::mArray catArray;
	
		for(unsigned int catIndex=0; catIndex < m_rootCategory.m_categories.size(); catIndex++)
			 catArray.push_back(m_rootCategory.m_categories[catIndex]->outputJson( JSON_PATH | JSON_TITLE | JSON_CAT_CUSTOM,true, &m_crunchConfig.m_server_url,m_crunchConfig.m_version, m_crunchConfig.m_product_listed_in_category,2,&m_crunchConfig));
		
		if(m_crunchConfig.m_transmit_catalog)
			m_indexJson["cat"]=catArray;	
		if(m_crunchConfig.m_server_directory.length())
			m_serverIndexJson["cat"]=catArray;
	}

	//enum-fields-value section
	json_spirit::mObject enumObject;
	for(unsigned int enumval=0; enumval < m_itemsDistinctValueCount.size(); enumval++)
	{
		CrunchField* field = m_itemsDistinctValueCount[enumval].first;
		ValueCountMap* map = m_itemsDistinctValueCount[enumval].second;
		json_spirit::mObject values;
		//loop on elements to output
		for( ValueCountMap::iterator h = map->begin(); h != map->end(); h++)
				values[h->first]= h->second;
		enumObject[field->m_specificationName] =  values;
	}
	m_indexJson["values"]=enumObject;

    //infos section
    m_indexJson["infos"]=getStatJson();
    m_serverIndexJson["infos"]=getStatJson();

 	//write index.json on disk
	std::string folderpath = m_crunchOutputDirectory+ std::string(INDEXPATH);
	crunch::Util::writeJsonFile( m_indexJson, folderpath, INDEXFILENAME );
	
	//if a server_directory is present, output json also there
	if(m_crunchConfig.m_server_directory.length()) {
		std::string folderpath = m_crunchConfig.m_server_directory + std::string(INDEXPATH);
		crunch::Util::writeJsonFile( m_serverIndexJson, folderpath, INDEXFILENAME );
	}
	
}




void crunch::Crunch::outputJsonCategory( Crunch* crunch )
{
	std::string outputdir = ( crunch->m_crunchConfig.m_transmit_catalog ) ?
										crunch->m_crunchConfig.m_directory : 
										crunch->m_crunchConfig.m_server_directory;

	crunch::Util::selectOrCreateDirectory(outputdir+ std::string(CATPATH));

	//create a plain list of all categories, including children.
	std::vector<Category*> categories;
	crunch->m_rootCategory.getChildrenList(categories);

	for(unsigned int catIndex=0; catIndex < categories.size(); catIndex++)
	{
		Category* cat = categories[catIndex];
		json_spirit::mObject obj = cat->outputJson(JSON_ALL_EXCEPT_SEARCHPATH,
													true,
													&crunch->m_crunchConfig.m_server_url,
													crunch->m_crunchConfig.m_version,
													crunch->m_crunchConfig.m_product_listed_in_category,
													1,
													&crunch->m_crunchConfig);

		if(crunch->m_crunchConfig.m_transmit_catalog) 
			crunch::Util::writeJsonFile(obj, crunch->m_crunchConfig.m_directory + cat->m_resource, cat->m_fileName);
		if(crunch->m_crunchConfig.m_server_directory.length())
			crunch::Util::writeJsonFile(obj, crunch->m_crunchConfig.m_server_directory + cat->m_resource, cat->m_fileName);
		
	}
}

/** 
* Thread function used to output either search indexes JSON FILES
* This is static function (thread), it receives a crunch pointer as a runtime context
**/
void crunch::Crunch::outputJsonSearches( Crunch* crunch )
{
		json_spirit::mObject searchObject;
		json_spirit::mObject serverSearchObject;

		


		//generate search indexes
		for(unsigned int searchers=0;searchers<crunch->m_crunchConfig.m_searchers.size(); searchers++)
		{
			json_spirit::mObject this_searcher = crunch->m_crunchConfig.m_searchers[searchers]->outputJson(crunch->m_crunchOutputDirectory,
																										crunch->m_crunchConfig.m_server_directory,
																										&crunch->m_crunchConfig.m_server_url,
																										crunch->m_crunchConfig.m_version);
			if(crunch->m_crunchConfig.m_searchers[searchers]->m_transmit)																					
				searchObject[crunch->m_crunchConfig.m_searchers[searchers]->getName()]=this_searcher;
			if(crunch->m_crunchConfig.m_server_directory.length())
				serverSearchObject[crunch->m_crunchConfig.m_searchers[searchers]->getName()]=this_searcher;
		}

		(*crunch->getIndexJson())["search"] = searchObject;
		(*crunch->getServerIndexJson()) ["search"]= serverSearchObject ;
}


/** Thread function used to output either product or model JSON files 
* This is static function (thread), it receives a crunch pointer as a runtime context
**/
void crunch::Crunch::outputJsonHashGroup( Crunch* crunch, boost::unordered_map <std::string, std::vector<CrunchElement*>* >& hashgroup,std::string filePrefix)
{

	std::string outputdir = ( crunch->m_crunchConfig.m_transmit_catalog ) ?
										crunch->m_crunchConfig.m_directory : 
										crunch->m_crunchConfig.m_server_directory;

	//loop on elements to output
	for( boost::unordered_map <std::string, std::vector<CrunchElement*>* >::iterator h = hashgroup.begin(); 
		h != hashgroup.end(); h++)
	{
		//iterate on this hashkey
		std::string hashkey=h->first;
		json_spirit::mArray hashArray;

		//loop on elements sharing this hashkey
		for( std::vector<CrunchElement*>::iterator crunchElt = h->second->begin();
			 crunchElt != h->second->end();
			 crunchElt++
			)
		{
			hashArray.push_back((*crunchElt)->outputJson(JSON_ALL_EXCEPT_SEARCHPATH, &crunch->m_crunchConfig.m_server_url, crunch->m_crunchConfig.m_version,true));
		}

		std::string fileName= filePrefix+hashkey+std::string(JSONEXT);
		std::string folderName = std::string(CATALOGPREFIX) +
							std::string(hashkey,0,crunch->m_crunchConfig.m_resource_hashkey) +
							std::string("/");
		if(crunch->m_crunchConfig.m_transmit_catalog) 
			crunch::Util::writeJsonFile(hashArray, crunch->m_crunchConfig.m_directory + folderName, fileName);
		if(crunch->m_crunchConfig.m_server_directory.length())
			crunch::Util::writeJsonFile(hashArray, crunch->m_crunchConfig.m_server_directory + folderName, fileName);
	
	}

}

/** 
* addPictures handle first picture and optional additional pictures by calling addOnePicture
* and reference it in the model/product/reference objects, by calling their corresponding addPicture method.
*/
void crunch::Crunch::addPictures(Item& item, Model* m, Product* p, Reference* r)
{
	//first picture
	addOnePicture(item.m_g_image_link,m,p,r);
	//additionnapictures
	for(unsigned short indexPic=0;indexPic<item.m_g_additional_image_link.size();indexPic++)
		addOnePicture(item.m_g_additional_image_link[indexPic],m,p,r);
}

/** 
* addFiles handle adding files by calling addOneFile
* and reference it in the model/product/reference objects, by calling their corresponding addFile method.
*/
void crunch::Crunch::addFiles(Item& item, Model* m, Product* p, Reference* r)
{
	

	for (   boost::unordered_map <std::string, std::string>::iterator pos = item.m_download_model_fields.begin();
            pos != item.m_download_model_fields.end();
            ++pos)
	{
		addOneFile(pos->first, pos->second, m);
	}
 	for (   boost::unordered_map <std::string, std::string>::iterator pos = item.m_download_product_fields.begin();
            pos != item.m_download_product_fields.end();
            ++pos)
	{
		addOneFile(pos->first, pos->second, p);
	}     

	
}


/** 
* addPictures will create the picture class instance if it does not already exist
* and reference it in the model/product/reference objects, by calling their corresponding addPicture method.
*/
void crunch::Crunch::addOnePicture(const std::string pictureUrl, Model* m, Product* p, Reference* r)
{
	Picture *pic;
	if(!pictureUrl.size()) return;
	if(m_pictures.count(pictureUrl)>0)
	{	//picture already exist (test by unicity of URL)
		pic=m_pictures[pictureUrl];
	}
	else
	{
		pic=new Picture;
		pic->m_URL=pictureUrl;
		pic->m_status=PENDING;
		pic->m_mainPicture=true;
		crunch::Util::md5(pic->m_URL,pic->m_idMd5);

		Picture::m_counter++;
		m_pictures.insert(std::make_pair<std::string,Picture*>(pictureUrl,pic));
	}
	p->addPicture(pic);
	m->addPicture(pic);
	r->addPicture(pic);
}

/** 
* addPictures will create the picture class instance if it does not already exist
* and reference it in the model/product/reference objects, by calling their corresponding addFile method.
*/
void crunch::Crunch::addOneFile(const std::string fileName, const std::string fileUrl, CrunchElement* elt)
{
	CrunchFile *file;
	if(!fileUrl.size()) return;
	if(m_files.count(fileUrl)>0)
	{	//file already exist (test by unicity of URL)
		file=m_files[fileUrl];
	}
	else
	{
		file=new CrunchFile;
		file->m_URL=fileUrl;
		file->m_status=PENDING;
		crunch::Util::md5(file->m_URL,file->m_idMd5);

		CrunchFile::m_counter++;
		m_files.insert(std::make_pair<std::string,CrunchFile*>(fileUrl,file));
	}
	elt->addFile(fileName,file);

}



/**
* populateItem takes an Item as parameter and fills it with
* a row data from the source file (either xml or csv)
* the getItemValue function called is implemented in XmlCrunch and CSVcrunch
*/

void crunch::Crunch::populateItem(Item& item)
{

	item.m_g_id                        = getItemValue(m_fieldList["g:id"]);
	item.m_title                       = getItemValue(m_fieldList["title"]);
	item.m_description                 = getItemValue(m_fieldList["description"]);
	item.m_g_google_product_category   = getItemValue(m_fieldList["g:google_product_category"]);
	item.m_g_product_type              = getItemValue(m_fieldList["g:product_type"]);
	item.m_link                        = getItemValue(m_fieldList["link"]);
	item.m_g_image_link                = getItemValue(m_fieldList["g:image_link"]);
	item.m_g_condition                 = getItemValue(m_fieldList["g:condition"]);
	item.m_g_availability              = getItemValue(m_fieldList["g:availability"]);
	item.m_g_price                     = getItemValue(m_fieldList["g:price"]);
	item.m_g_sale_price                = getItemValue(m_fieldList["g:sale_price"]);
	item.m_g_gtin                      = getItemValue(m_fieldList["g:gtin"]);
	item.m_g_brand                     = getItemValue(m_fieldList["g:brand"]);
	item.m_g_mpn                       = getItemValue(m_fieldList["g:mpn"]);
	item.m_g_item_group_id             = getItemValue(m_fieldList["g:item_group_id"]);
	item.m_g_size                      = getItemValue(m_fieldList["g:size"]);
	item.m_g_material                  = getItemValue(m_fieldList["g:material"]);
	item.m_g_pattern                   = getItemValue(m_fieldList["g:pattern"]);
	item.m_g_color                     = getItemValue(m_fieldList["g:color"]);
	item.m_g_additional_image_link     = getItemArray(m_multipleFieldList["g:additional_image_link"]);

	for(boost::unordered_map <std::string, CrunchField*>::iterator p=m_customProductFieldList.begin();
		p!=m_customProductFieldList.end();p++)
	{
		item.m_custom_product_fields.insert(std::pair<std::string,std::string>(p->first,getItemValue(p->second)));
	}


	for(boost::unordered_map <std::string, CrunchField*>::iterator m=m_customModelFieldList.begin();
		m!=m_customModelFieldList.end();m++)
	{
		item.m_custom_model_fields.insert(std::pair<std::string,std::string>(m->first,getItemValue(m->second)));
	}



	for(boost::unordered_map <std::string, CrunchField*>::iterator r=m_customReferenceFieldList.begin();
		r!=m_customReferenceFieldList.end();r++)
	{
		item.m_custom_reference_fields.insert(std::pair<std::string,std::string>(r->first,getItemValue(r->second)));
	}

	for(boost::unordered_map <std::string, CrunchField*>::iterator p=m_downloadProductFieldList.begin();
		p!=m_downloadProductFieldList.end();p++)
	{
		item.m_download_product_fields.insert(std::pair<std::string,std::string>(p->first,getItemValue(p->second)));
	}


	for(boost::unordered_map <std::string, CrunchField*>::iterator m=m_downloadModelFieldList.begin();
		m!=m_downloadModelFieldList.end();m++)
	{
		item.m_download_model_fields.insert(std::pair<std::string,std::string>(m->first,getItemValue(m->second)));

	}


}

/** 
* transform field value according to field regexp
* TODO possible security breach here : regexp is not sanitized
*/
std::string crunch::Crunch::applyFieldRegexp(const CrunchField* field, std::string value)
{
	if(!field->m_hasRegexp) return value;
	boost::match_results<std::string::const_iterator> what; // match result for regexp operations
	if(boost::regex_match(value, what, field->m_regexp))
		{
			return what[1];
	}
	return std::string("");
}

/**
* retrieve field value from current row in source file and apply optional regexp
*/
std::string crunch::Crunch::getItemValue(const CrunchField* field)
{

	std::string result=m_reader->getItemValue(field);
	
	return applyFieldRegexp(field,result);
}

/**
* retrieve field array from current row in source file and apply optional regexp
*/
std::vector<std::string> crunch::Crunch::getItemArray(const CrunchField* field)
{	
	std::vector<std::string> result=m_reader->getItemArray(field);
	for (	std::vector <std::string>::iterator pos = result.begin();
     		pos != result.end();
     		++pos)
			*pos=applyFieldRegexp(field,*pos);
	return  result;
}

/**
* insertElementIntoHash handles the creation and filling of 
* the hashtables of file-hashkey size (several models or products in a json file)
*/
void crunch::Crunch::insertElementIntoHash(	boost::unordered_map <std::string, std::vector<CrunchElement*>* >& hashTable,
									CrunchElement* elt)
{ 
	//compute hashfile for model or product
	std::string hash = elt->getHash(m_crunchConfig.m_file_hashkey);
	std::vector<CrunchElement*>* v;
	if(!hashTable.count(hash))
	{
		v= new std::vector<CrunchElement*>;
		hashTable.insert(std::make_pair<std::string,std::vector<CrunchElement*>* > (hash,v));
	}
	else v=hashTable[hash];
	v->push_back(elt);
}


/**
* group items (models) by the same hash key.
*/
void crunch::Crunch::groupItemsByHashKeys()
{
	for( boost::unordered_map <std::string, Model*>::const_iterator m = m_models.begin(); 
			m != m_models.end(); m++)
	{

		m->second->setHashKeySize(m_crunchConfig.m_resource_hashkey, m_crunchConfig.m_file_hashkey);
		insertModelIntoHash(m->second);
			
		for( boost::unordered_map <std::string, Product*>::const_iterator p = m->second->m_products.begin(); 
			p != m->second->m_products.end(); p++)
		{
			p->second->setHashKeySize(m_crunchConfig.m_resource_hashkey, m_crunchConfig.m_file_hashkey);
			insertProductIntoHash(p->second);
		}

	}
	
}



/**
* wrapper for insertElementIntoHash, used specifically for Models
*/
void crunch::Crunch::insertModelIntoHash(crunch::Model* m)
{
	insertElementIntoHash(m_modelsByHashKeys,(CrunchElement*) m);
}

/**
* wrapper for insertElementIntoHash, used specifically for Products
*/
void crunch::Crunch::insertProductIntoHash(crunch::Product* p)
{
		insertElementIntoHash(m_productsByHashKeys,(CrunchElement*) p);
}



/**
* getNextPictureToDownload handles a thread-safe access to the
* list of pictures to download. This method is called by each
* download thread and takes care of the picture list access.
*/
Picture* crunch::Crunch::getNextPictureToDownload()
{
	boost::lock_guard<boost::mutex> guard(m_picturesMutex); //one concurrent access only
	if(!m_picturesDownloadInit) //first call, let's create and save iterator.
	{
		m_picturesIterator=m_pictures.begin();
		m_picturesDownloadInit=true;
		if(m_picturesIterator == m_pictures.end()) return NULL; // no picture at all
		Picture::m_downloadCounter++;
		return m_picturesIterator->second; //first picture to download
	}
	if(m_picturesIterator == m_pictures.end())
		return NULL; //no more picture
	m_picturesIterator++;
	Picture::m_downloadCounter++;
	if(! (Picture::m_downloadCounter%100) )
		std::cout << "." << std::flush;
	if(m_picturesIterator == m_pictures.end())
		return NULL; //no more picture
	return m_picturesIterator->second;
  
}

/**
* getNextFileToDownload handles a thread-safe access to the
* list of fiels to download. This method is called by each
* download thread and takes care of the files list access.
*/
CrunchFile* crunch::Crunch::getNextFileToDownload()
{
	boost::lock_guard<boost::mutex> guard(m_filesMutex); //one concurrent access only
	if(!m_filesDownloadInit) //first call, let's create and save iterator.
	{
		m_filesIterator=m_files.begin();
		m_filesDownloadInit=true;
		if(m_filesIterator == m_files.end()) return NULL; // no file at all
		CrunchFile::m_downloadCounter++;
		return m_filesIterator->second; //first file to download
	}
	if(m_filesIterator == m_files.end())
		return NULL; //no more file
	m_filesIterator++;
	CrunchFile::m_downloadCounter++;
	if(! (CrunchFile::m_downloadCounter%100) )
		std::cout << "." << std::flush;
	if(m_filesIterator == m_files.end())
		return NULL; //no more file
	return m_filesIterator->second;
  
}


/**
* downloadPictures launch MAXTHREADS threads to download pictures
* wait for threads to complete before returning (join() method)
*/
void crunch::Crunch::downloadPictures()
{
	
	//check if pictures have to be downloaded
	if(!m_crunchConfig.m_download_pictures) return;

	m_picturesDownloadInit=false;
	boost::thread downloadThreads[10];
	for(int i=0;i<MAXTHREADS;i++)
	{
		Util::log(std::string("Launching picture download thread ")+Util::to_string(i)+std::string("\n"),
					logTRACE);
		
		downloadThreads[i]=boost::thread(Crunch::downloadPictureThread,this,i);
	}
	for(int i=0;i<MAXTHREADS;i++) downloadThreads[i].join();

}

/**
* downloadFiles launch MAXTHREADS threads to download files
* wait for threads to complete before returning (join() method)
*/
void crunch::Crunch::downloadFiles()
{
	
	//check if pictures have to be downloaded
	if(!m_crunchConfig.m_download_files) return;

	m_filesDownloadInit=false;
	boost::thread downloadThreads[10];
	for(int i=0;i<MAXTHREADS;i++)
	{
		Util::log(std::string("Launching files download thread ")+Util::to_string(i)+std::string("\n"),
					logTRACE);
		
		downloadThreads[i]=boost::thread(Crunch::downloadFileThread,this,i);
	}
	for(int i=0;i<MAXTHREADS;i++) downloadThreads[i].join();

}


/**
* static method used to represent a picture download thread.
* @param crunch* : calling crunch instance
* @param int serial : id of thread
*/
void crunch::Crunch::downloadPictureThread(Crunch* crunch, int serial)
{

	Picture* pic;
	CURL* easyhandle = curl_easy_init();
	curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1);
	unsigned int counter=0;
	pic = crunch->getNextPictureToDownload();
	bool serverFlag = crunch->m_crunchConfig.m_server_directory.length()?true:false;
	while( pic != NULL )
	{
		if(pic->downloadAndResize(	easyhandle,
									crunch->m_crunchConfig.m_transmit_pictures,
									crunch->m_crunchConfig.m_transmit_thumbnails,
									serverFlag,
									crunch->m_crunchOutputDirectory,
									crunch->m_crunchConfig.m_server_directory,
									PHOTOPACKNAME,
									THUMBPACKNAME,
									crunch->m_crunchConfig.m_photo_hashkey,
									(crunch->m_crunchConfig.m_create_thumbnails)
										?crunch->m_crunchConfig.m_thumbnail_width:0,
									(crunch->m_crunchConfig.m_create_thumbnails)
										?crunch->m_crunchConfig.m_thumbnail_height:0,
									(crunch->m_crunchConfig.m_resize_pictures)
										?crunch->m_crunchConfig.m_max_width:0,
									(crunch->m_crunchConfig.m_resize_pictures)
										?crunch->m_crunchConfig.m_max_height:0
									))

		{	
			counter++;
		}
		
		pic = crunch->getNextPictureToDownload();

	}
	curl_easy_cleanup(easyhandle);
	
	Util::log(std::string("thread ")+Util::to_string(serial)+std::string(" Job done\n"),
					logTRACE);
}


/**
* static method used to represent a file download thread.
* @param crunch* : calling crunch instance
* @param int serial : id of thread
*/
void crunch::Crunch::downloadFileThread(Crunch* crunch, int serial)
{

	CrunchFile* file;
	CURL* easyhandle = curl_easy_init();
	curl_easy_setopt(easyhandle, CURLOPT_NOSIGNAL, 1);
	curl_easy_setopt(easyhandle, CURLOPT_ERRORBUFFER, Util::curlErrorBuffer);
	unsigned int counter=0;
	file = crunch->getNextFileToDownload();
	bool serverFlag = crunch->m_crunchConfig.m_server_directory.length()?true:false;
	while( file != NULL )
	{
		Util::log(std::string("thread ")+Util::to_string(serial)+std::string(" download ")+file->m_URL+std::string("\n"),
					logTRACE);
		if(file->downloadFile(		easyhandle,
									crunch->m_crunchConfig.m_transmit_files,
									serverFlag,
									crunch->m_crunchOutputDirectory,
									crunch->m_crunchConfig.m_server_directory,
									FILEPACKNAME,
									crunch->m_crunchConfig.m_external_file_hashkey))

		{	
			counter++;
		}
		else if(file->downloadFile(		easyhandle,
									crunch->m_crunchConfig.m_transmit_files,
									serverFlag,
									crunch->m_crunchOutputDirectory,
									crunch->m_crunchConfig.m_server_directory,
									FILEPACKNAME,
									crunch->m_crunchConfig.m_external_file_hashkey))
		{
			counter++;
		}
		
		file = crunch->getNextFileToDownload();

	}
	curl_easy_cleanup(easyhandle);
	
	Util::log(std::string("thread ")+Util::to_string(serial)+std::string(" Job done\n"),
					logTRACE);
}


json_spirit::mObject* crunch::Crunch::getIndexJson()
{
	return &m_indexJson;
}

json_spirit::mObject* crunch::Crunch::getServerIndexJson()
{
	return &m_serverIndexJson;
}

void crunch::Crunch::computeDiff()
{
	Util::log("Starting diff\n", logINFO);
	if(!m_crunchConfig.m_diff_directory.length())
		return;

	std::vector<std::string> old_rep_list;
	std::vector<std::string> new_rep_list;

	crunch::Util::getDirectoryContent(m_crunchConfig.m_diff_directory, 	old_rep_list);
	crunch::Util::getDirectoryContent(m_crunchConfig.m_directory, 		new_rep_list);

	unsigned int size = new_rep_list.size();
	for(unsigned int newIndex=0; newIndex < size; newIndex++)
	{
		//exists in old dir ?
		std::vector<std::string>::iterator search = std::find(old_rep_list.begin(),
												old_rep_list.end(), new_rep_list[newIndex]);
		if(search == old_rep_list.end())
		{
			//new resource, not found in old dir
			m_newResources.push_back(new_rep_list[newIndex]);
			continue;
		} 

		//check rep content
		if(!compareResource(m_crunchConfig.m_diff_directory,
						m_crunchConfig.m_directory,new_rep_list[newIndex]))
		{
			//resource different !
			m_updatedResources.push_back(new_rep_list[newIndex]);
		}
		old_rep_list.erase(search);

	}

	//unused resources are still in old_rep_list
	size = old_rep_list.size();
	for(unsigned int oldIndex=0; oldIndex < size; oldIndex++)
		m_outdatedResources.push_back(old_rep_list[oldIndex]);

	//log results
	unsigned int logsize = m_outdatedResources.size();
	Util::log("Outdated Resources : \n", logINFO);
	std::string newLine("\n");
	
	for(unsigned int ind=0; ind < logsize; ind++)
	{
		Util::log(m_outdatedResources[ind]+newLine, logINFO);
	}

	logsize = m_updatedResources.size();
	Util::log("Updated Resources : \n", logINFO);
	for(unsigned int ind=0; ind < logsize; ind++)
	{
		Util::log(m_updatedResources[ind]+newLine, logINFO);

	}

	logsize = m_newResources.size();
	Util::log("New Resources : \n", logINFO);
	for(unsigned int ind=0; ind < logsize; ind++)
	{
		Util::log(m_newResources[ind]+newLine, logINFO);
	}


}	

/**
* compare crunch output between old path (previous crunch exec) and current one
* to create diff output
**/
bool crunch::Crunch::compareResource(const std::string oldR, const std::string newR, const std::string rscName)
{
	std::vector<std::string> new_rep_content;
	std::vector<std::string> old_rep_content;
	crunch::Util::getDirectoryContent(oldR+rscName, old_rep_content);
	crunch::Util::getDirectoryContent(newR+rscName, new_rep_content);
	unsigned int size = new_rep_content.size();

	for(unsigned int newIndex=0; newIndex < size; newIndex++)
	{
		//exists in old dir ?
		std::vector<std::string>::iterator search = std::find(old_rep_content.begin(),
												old_rep_content.end(), new_rep_content[newIndex]);
		if(search == old_rep_content.end())
		{
			//a file is present in this directory and not in the old one.
			return false;
		}
	}

	//still here ? all files exists in both directory.
	//let's compare the file contents
	for(unsigned int newIndex=0; newIndex < size; newIndex++)
	{
		std::string file1 = newR+rscName+std::string("/")+new_rep_content[newIndex];
		std::string file2 = oldR+rscName+std::string("/")+new_rep_content[newIndex];
		
		if(!crunch::Util::compareFiles(file1,file2))
			return false;
	}
	return true;
}

void crunch::Crunch::outputJsonStatus()
{
	Util::log("Output Json Status\n", logINFO);
	if(!m_crunchConfig.m_status_json_file.length())
		return;

	json_spirit::mObject statusJson;
    statusJson["infos"] =getStatJson();

   //add changes
   	json_spirit::mObject objChanges;
	json_spirit::mArray newResourcesJson; 
	json_spirit::mArray updatedResourcesJson; 
	json_spirit::mArray outdatedResourcesJson; 

	for(unsigned int index=0; index < m_newResources.size(); index++)
		 newResourcesJson.push_back(m_newResources[index]);
	for(unsigned int index=0; index < m_updatedResources.size(); index++)
		 updatedResourcesJson.push_back(m_updatedResources[index]);
	for(unsigned int index=0; index < m_outdatedResources.size(); index++)
		 outdatedResourcesJson.push_back(m_outdatedResources[index]);
	objChanges["new"]=newResourcesJson;
	objChanges["deprecated"]=outdatedResourcesJson;
	objChanges["updated"]=updatedResourcesJson;

	statusJson["changes"]=objChanges;

    //write index.json on disk
	FILE* f=fopen(m_crunchConfig.m_status_json_file.c_str(),"w");
		if(!f)
		{
			std::string exc("Cannot open file for writing :");
			exc+=m_crunchConfig.m_status_json_file;
			throw std::runtime_error(exc);
		}

	fputs(json_spirit::write(statusJson,json_spirit::raw_utf8).c_str(),f);
	fclose(f);

}

void crunch::Crunch::computeHashKeySizes()
{

	/** compute resource hash key size **/
	if(m_crunchConfig.m_resource_hashkey == -1 )
	{
		if(m_crunchConfig.m_generate_product_file)
			m_crunchConfig.m_resource_hashkey = computeHashKeyFolderSizeFromItemCount ( Product::m_productCounter );
		else
			m_crunchConfig.m_resource_hashkey = computeHashKeyFolderSizeFromItemCount ( Model::m_modelCounter );
	}

	/** compute file hash key sizes **/
	if(m_crunchConfig.m_file_hashkey == -1 )
	{
		if(m_crunchConfig.m_generate_product_file)
			m_crunchConfig.m_file_hashkey = computeHashKeyFileSizeFromItemCount ( Product::m_productCounter );
		else
			m_crunchConfig.m_file_hashkey = computeHashKeyFileSizeFromItemCount ( Model::m_modelCounter );
	}

	/** compute external file hash key sizes **/
	if(m_crunchConfig.m_external_file_hashkey == -1 )
	{
			m_crunchConfig.m_external_file_hashkey = computeHashKeyFolderSizeFromItemCount ( CrunchFile::m_counter );
	}

	/** compute photo hash key sizes **/
	if(m_crunchConfig.m_photo_hashkey == -1 )
	{
		m_crunchConfig.m_photo_hashkey = computeHashKeyPictureSizeFromItemCount ( Picture::m_counter );
	}

}

unsigned short crunch::Crunch::computeHashKeyFileSizeFromItemCount(unsigned long itemcount)
{
	return computeHashKeyFolderSizeFromItemCount(itemcount)+1;

}

unsigned short crunch::Crunch::computeHashKeyPictureSizeFromItemCount(unsigned long itemcount)
{
	unsigned long foldercount = itemcount / MAX_PICTURE_PER_FOLDER;
	unsigned short folderkey=0;
	if(foldercount <=1) 
		 folderkey = 0;
	else if(foldercount <=16)
		folderkey = 1;
		else if(foldercount <= 256)  
			folderkey = 2;
		else if(foldercount <= 4096)
			folderkey = 3;
		else folderkey = 4;
	return folderkey;

}

unsigned short crunch::Crunch::computeHashKeyFolderSizeFromItemCount(unsigned long itemcount)
{
	unsigned long filecount = itemcount / MAX_ITEM_PER_JSON_FILE;
	unsigned long foldercount = filecount / MAX_JSON_FILE_PER_FOLDER;
	unsigned short folderkey=0;
	if(foldercount <1) 
		 folderkey = 0;
	else if(foldercount <16)
		folderkey = 1;
		else if(foldercount < 256)  
			folderkey = 2;
		else if(foldercount < 4096)
			folderkey = 3;
		else folderkey = 4;
	return folderkey;
		
}

 bool crunch::Crunch::getVerifyConfig(){
    return m_crunchConfig.bCmdVerifyConfig;
 };

void crunch::Crunch::locateCategoryThumbnail()
{
	if(!m_crunchConfig.m_category_thumbnail)
		return;

	std::vector<Category*> categories;
	m_rootCategory.getChildrenList(categories);

	for(unsigned int catIndex=0; catIndex < categories.size(); catIndex++)
	{
		categories[catIndex]->locateThumbnail();
		
	}

}



