#ifndef __CATEGORY_HPP__
#define __CATEGORY_HPP__

#include <vector>
#include <string>
#include <iostream>
#include "crunchconfig.hpp"
#include "crunchelement.hpp"
#include "product.hpp"
#include "model.hpp"
#include "reference.hpp"
#include "json_spirit/json_spirit.h"
namespace crunch
{
	class Picture;

	class Category
	{

	public:

		static unsigned long m_counter;
		std::string m_id;
		std::string m_name;
		std::string m_treePath;
		std::string m_resource;
		std::string m_fileName;
		Category* m_parent;
		Picture* m_thumbPicture;
		std::vector<Model*> m_models;
		std::vector<Category*> m_categories;
		std::string m_idTree;

		~Category();
		void addModel(Model* m);
		Category(const std::string& path, const std::string& name, const std::string& resource, Category* parent);
		static Category* staticFindCategory(std::vector<Category*> &categories,std::string category_path);
		Category* findCategory(std::string category_path);

		void addSubCategory(Category* c);
		std::string getHash(const unsigned short length);
		int compareName(const std::string& path);
		Category* findOrCreateCategory(const std::string &path, const std::string &separator, bool& isNewCategory, std::string resource, Crunch *c);
		json_spirit::mObject outputJson(CrunchDetailLevel detailLevel, bool oneLevelOnly, const std::string* server_url = NULL,int crunchversion = -1, bool product_listed_in_category = false,int catLevel = 1, CrunchConfig* config = NULL);

		static std::vector<std::string> splitPath(const std::string &path,const std::string &separator);
		void getChildrenList(std::vector<Category*>& result);
		Picture* locateThumbnail();

		std::string getTreeString();
	};
} //namespace crunch
#endif

