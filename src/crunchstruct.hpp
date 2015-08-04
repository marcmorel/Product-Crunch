#ifndef __CRUNCHSTRUCT_HPP
#define __CRUNCHSTRUCT_HPP

#include <vector>
#include <string>
#include <iostream>
#include "boost/unordered_map.hpp"
#include "boost/regex.hpp"


/**
* @author Marc Morel
* @copyright Procheo, 2014
*
* this files contains different structs used in the crunch app.
*/




namespace crunch
{

	enum CrunchValueType { INT, STRING };

	/** 
	* struct Crunchfield is a type used to represent a single field we should find in the source file
	* it stores :
	* - finalName : the final Name of the field (the one used in the Google Merchant definition and also the
	* one used in the crunch output )
	* - replacementName : the name used in the source file. Should be the same,
	* but some google merchant implementations are different. ReplacementName is found in config file, at init time
	* - regexp : if hasRegexp is true,a regexp has to be applied for this field. Regexp is also found in config file.
	* - hasRegexp : bool
	* - mandatory : bool (if true, an exception will be raised if this field is not found for an item)
	* crunchfield is used in a collection in the Crunch class. This collection is constructed once, at init time, and is used
	* as a list of fields we should find for each item is the source file.
	*/

	struct CrunchField
	{
		std::string m_specificationName;
		std::string m_replacementName;
		boost::regex m_regexp;
		bool m_mandatory;
		bool m_hasRegexp;

		//default constructor, without regexp
		CrunchField(	const std::string& specificationName,
						const std::string& replacementName,
						bool mandatory) :
				m_specificationName(specificationName), m_replacementName(replacementName), m_mandatory(mandatory),m_regexp(""), m_hasRegexp(false) {}
	};

	/**
	* struct Item contains all field for a single row, without custom fields.
	* each line in a csv file or each item in a xml file will be stored (temporary)
	* in an Item struct, before being split in category/model/product/reference
	*/
	struct Item
	{

		std::string m_modelGroup;
		std::string m_productGroup;

		std::string m_g_id;
		std::string m_title;
		std::string m_description;
		std::string m_g_google_product_category;
		std::string m_g_product_type;
		std::string m_link;
		std::string m_g_image_link;
		std::string m_g_condition;
		std::string m_g_availability;
		std::string m_g_price;
		std::string m_g_sale_price;
		std::string m_g_gtin;
		std::string m_g_brand;
		std::string m_g_mpn;
		std::string m_g_item_group_id;
		std::string m_g_size;
		std::string m_g_material;
		std::string m_g_pattern;
		std::string m_g_color;
	 
		std::vector<std::string> m_g_additional_image_link;
		boost::unordered_map <std::string, std::string> m_custom_product_fields;
		boost::unordered_map <std::string, std::string> m_custom_model_fields;
		boost::unordered_map <std::string, std::string> m_custom_reference_fields;
		boost::unordered_map <std::string, std::string> m_download_product_fields;
		boost::unordered_map <std::string, std::string> m_download_model_fields;

	};

	typedef boost::unordered_map <std::string,unsigned long > ValueCountMap;
	typedef std::pair<CrunchField*, ValueCountMap* > FieldPValueCountMap;

	/**
	* struct SearchType list different types of search indexes implemented in the crunch app
	* one type so far : exactsearch
	*/
	typedef enum { exactSearch, progressiveSearch } SearchType;

	/** 
	* searchParam represents the parameters needed to describe a search index. It is used during the parsing 
	* of the crunch config file 
	**/
	struct SearchParam
	{
		std::string searchName;
		std::string searchField;
		SearchType	searchType;
	};

	typedef std::vector<std::string> stringvector;
	typedef std::vector<std::string>::iterator stringvectoriterator;
}

#endif