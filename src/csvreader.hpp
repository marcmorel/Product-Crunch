#ifndef __CSVREADER_HPP__
#define __CSVREADER_HPP__

/**
* @author Marc Morel
* @copyright Procheo, 2014
*/

#include "reader.hpp"
#include <iostream>
#include <fstream>  
#include <algorithm>    // copy
#include <iterator>     // ostream_operator
#include <boost/tokenizer.hpp>
/**
* The class CSVCrunch is the main class for the crunch process in case of CSV source file..
*/
namespace crunch
{
	 typedef boost::tokenizer< boost::escaped_list_separator<char> > CSVTokenizer;

	class CSVReader : public Reader
	{

		std::ifstream m_fileStream; //source file
		boost::escaped_list_separator<char>* m_separator; //csv separator (default to ";")
		std::vector<std::string> m_header; //list of csv fields (first line of source file)
		boost::regex m_fieldsRegexp; //regexp to replace all CSV fields in XML fields
		std::string m_currentRegexpReplacementStr; //replacement value for current line
		size_t m_headerSize; //number of csv fields
		unsigned long m_lineNumber; //number of current line (starting at 1)
		

	public:

		static CSVReader* This;
		boost::unordered_map <std::string, std::string > m_currentItem; //list of CSV fields indexed by CSV keys

		/**
		* Default constructor
		* Will create a root category
		*/
		CSVReader(CrunchConfig* config);

        ~CSVReader();
		/**
		* Initialization function
		* @param const std::string& JSON parameter absolute path (in local filesystem)
		*/
		bool openSourceFile();


		/**
		* Iterator function
		* @return true if next item has been found
		*/
		bool hasNextItem();

		std::string getItemValue(const CrunchField* field);
		std::vector<std::string> getItemArray(const CrunchField* field);

		static std::string regexFieldCallback(boost::match_results<std::string::const_iterator> match);

	protected:
		std::string getNextLine();
	};
}



#endif