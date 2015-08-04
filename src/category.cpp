#include "category.hpp"
#include "crunch.hpp"
#include "crunchelement.hpp"
#include <boost/regex/v4/regex.hpp>
#include <boost/algorithm/string/trim.hpp>
#include "util.hpp"

using namespace crunch;

unsigned long Category::m_counter=0;

Category::Category(const std::string &path, const std::string &category_name, const std::string& resource,  Category* parent)
{
	m_treePath=path;
	m_parent = parent;
	m_name=category_name;
	char md5[33];
	
	Util::md5(path,md5);
	m_counter++;
	m_id=Util::to_string(m_counter); //std::string(md5);
	if(parent)
	{	
		m_idTree =  m_parent->m_idTree+std::string("#")+m_id+std::string("#");
	}
	else
		m_idTree=std::string("");
	m_fileName = std::string(CATFILEPREFIX)+ m_id + std::string(JSONEXT);
	m_resource = resource;
	m_thumbPicture = NULL;
}

/** category destructor. Will delete all subcategories. **/
Category::~Category()
{
	for(unsigned int browse=0;browse<m_categories.size();browse++)
		delete m_categories.at(browse);
}

/** insert model pointer in m_models vector. **/
/** All models of current category will be stored in m_models **/
void Category::addModel(Model* m)
{
	m_models.push_back(m);
}


std::string Category::getHash(const unsigned short length)
{
	return std::string(m_id,length);
}


void Category::addSubCategory(Category* c)
{
	m_categories.push_back(c);
}

/**
* split a full category path using category separator
* @param const std::string &path path to split
* @param const std::string &separator separator to split with
**/
std::vector<std::string> Category::splitPath(const std::string &path, const std::string &separator)
{
	const boost::regex regexp(separator);
	const boost::sregex_token_iterator endOfSequence;
	std::vector<std::string> result;
	boost::sregex_token_iterator token(path.begin(), path.end(), regexp, -1);
	while(token != endOfSequence) 
	{
		if(token->length())
			result.push_back(*token);
		token++;

	}
	return result;
}

int Category::compareName(const std::string &pathElementToFind)
{
	return m_name.compare(pathElementToFind);
}

//modifier pour prendre une cateogry root en paramétrage
Category* Category::findOrCreateCategory(const std::string &path, const std::string &separator, bool& isNewCategory, std::string resource, Crunch* crunch)
{
	// split path using cat separator to construct tree path to reach that category
	std::vector<std::string> split=Category::splitPath(path,separator);
	isNewCategory = false;
	unsigned short maxdepth=split.size(); 
	Category* subCategories=this;
	Category* categoryFound=NULL;
	bool pathFound=false;
	unsigned long browse;
	
	std::string currentPath;
	//move along tree path to find and/or create categories
	for(unsigned short depth=0;depth<maxdepth;++depth)
	{
		//current branch name
		std::string pathElementToFind=split.at(depth);
		currentPath+=(currentPath.length())?separator+pathElementToFind:pathElementToFind;
		pathFound=false;

		//browse current subcategores to find branch
		for(browse=0;browse<subCategories->m_categories.size();browse++)
		{
			if(!(subCategories->m_categories.at(browse)->compareName(pathElementToFind))) // 0 means found
			{
				//branch found, set current position to new branch
				pathFound=true;
				categoryFound=subCategories->m_categories.at(browse);
				subCategories=categoryFound;
				break;
			}
		}

		if(!pathFound)
		{
			//category has not been found for this path element. We will create it
			Category* c=new Category(currentPath,pathElementToFind,resource, this);
			crunch->addCategoryToSearchIndex(c);
			isNewCategory = true;
			subCategories->addSubCategory(c);
			subCategories=c;
		}
	}
	return subCategories;
}

json_spirit::mObject Category::outputJson(CrunchDetailLevel detailLevel, bool oneLevelOnly, const std::string* server_url,int crunchVersion, bool product_listed_in_category,int catLevel, CrunchConfig* config)
{
	json_spirit::mObject obj;
	json_spirit::mArray modArray;
	json_spirit::mArray prdArray;
	json_spirit::mArray catArray;
	//first add subcategories
	unsigned int subCatCount = m_categories.size();
	if((subCatCount) &&  ( detailLevel & JSON_SUBCAT ) )
		{
			//what level of detail for subcats ?
			CrunchDetailLevel sublevel = oneLevelOnly ? (detailLevel & ~JSON_SUBCAT) : detailLevel;
			
			//models and products should not be output for subcategory list
			sublevel &= ~JSON_CHILDREN;
			for(unsigned int browse=0;browse<subCatCount;browse++)
			{
				json_spirit::mObject catObj	= m_categories[browse]->outputJson(sublevel,
																			 oneLevelOnly, server_url,
																			 crunchVersion,
																			 product_listed_in_category,(catLevel+ 1), config);
				catArray.push_back(catObj);
			}

			obj [ "cat" ] =  catArray;
		}

	obj [ "id"] =  m_id;
	

	// add category info
	if( crunchVersion == CRUNCH_VERSION_1 )
	{
		if  ( detailLevel & JSON_DESCRIPTION ) 
		{	
			json_spirit::mObject description_obj;
			json_spirit::mObject description_fr_obj;
			description_fr_obj["title"]=m_name;
			description_fr_obj["description"] =  m_name;
			description_fr_obj["path"] = m_treePath;
			description_obj ["fr_fr"] = description_fr_obj;
			obj ["description"] = description_obj;
		}
	}
	else
	{
		if ( detailLevel & JSON_TITLE )
			{
				obj["name"]=  m_name;
				if(m_thumbPicture != NULL)
				{
					json_spirit::mObject thumb_obj = m_thumbPicture->outputJson(server_url);
					obj [ "picture" ] = thumb_obj;
				}
			}
		//we use a catLevel counter in order not to write the path of the current category in its own file
        //for the root category we force level to 2 in order to write the path because outputJsn is not used properly
        if( detailLevel & JSON_PATH && catLevel > 1)
		{
			json_spirit::mObject path_obj;
			path_obj ["resource"] = m_resource;
			path_obj[ "file"]= m_fileName;
            obj[ "path" ] = path_obj;
		}
	}

	//add categories child : models
	if(detailLevel & JSON_CHILDREN)
	{
		unsigned int modelCount = m_models.size();
		if(modelCount)
		{
			for(unsigned int browse=0;browse<modelCount;browse++)
			{
				json_spirit::mObject modObj=m_models[browse]->outputJson(JSON_ALL_EXCEPT_CHILDREN, server_url, crunchVersion,false);
				modArray.push_back(modObj);
				if(product_listed_in_category)
				{
					Model* model = m_models[browse];
					unsigned int productCount = model->m_products.size();
					if(productCount)
					{

						for( boost::unordered_map <std::string, Product* >::iterator p = model->m_products.begin(); 
							p != model->m_products.end(); p++)
						
						{
							json_spirit::mObject prdObj=p->second->outputJson(JSON_ALL_EXCEPT_CHILDREN, server_url, crunchVersion,false);
							prdArray.push_back(prdObj);
						}
					}

				}
			}
			obj [ "mod" ] =  modArray;
			if(product_listed_in_category)
				obj[ "prd" ]= prdArray;
		}

		
	}

	if(detailLevel & JSON_CAT_CUSTOM) //add category custom info
	{
			//Util::log(std::string("recherche de path pour ")+std::string(m_treePath)+std::string("\n"), logTRACE); 

			if(config->m_categoryParameter.count(m_treePath)>0)
			{
				obj["custom"]=config->m_categoryParameter[m_treePath];
			//	Util::log(std::string("trouvé ")+std::string(m_treePath)+std::string("\n"), logTRACE); 
			}

	}
	
	return obj;
}

void Category::getChildrenList(std::vector<Category*>& result)
{
	for(int browse=0;browse<m_categories.size();browse++)
	{
		result.push_back(m_categories[browse]);
		m_categories[browse]->getChildrenList(result);
	}
}

Picture*  Category::locateThumbnail()
{

		if(m_thumbPicture!=NULL) return m_thumbPicture;

		unsigned int modelCount = m_models.size();
		
		for(unsigned int browse=0;browse<modelCount;browse++)
		{
			std::vector<Picture*>* pictures = m_models[browse]->getPictures();
			unsigned int pictureCount = pictures->size();
			if(!pictureCount) continue;

			for(unsigned int pic = 0; pic < pictureCount;pic++)
			{
				
				if((*pictures)[pic]->m_status != ERROR)
				{
					m_thumbPicture =  (*pictures)[pic]; 
					
					return m_thumbPicture;
				}
			}
		}

		//still here ? find picture in subcat.
		for(int browsecat=0;browsecat<m_categories.size();browsecat++)
		{
			Picture* pic = m_categories[browsecat]->locateThumbnail();
			if(pic!=NULL)
			{
				m_thumbPicture = pic;
				return pic;
			}
		}
	
				
		return NULL;
}


std::string Category::getTreeString()
{
	return m_idTree;
}









