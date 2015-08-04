#ifndef __READER_HPP
#define __READER_HPP
#include "crunchconfig.hpp"
#include "crunchstruct.hpp"


/**
* @author Marc Morel
* @copyright Procheo, 2014
*/


/**
* the class reader is an abstract class. It is used by the Crunch class to manage the source file (download and read)
*
**/


namespace crunch
{
	class Reader
	{

	protected:
		CrunchConfig* m_crunchConfig;
		std::string m_sourceFile;
	public:
		Reader(CrunchConfig* config);
        ~Reader();
		bool locateSourceFile();
		bool downloadSourceFile();
		/** 
		* virtual methods; will be implemented in derived class XMLCrunch or CSVCrunch
		*/
		virtual bool openSourceFile() = 0;
		virtual bool hasNextItem() = 0;
		virtual std::string getItemValue(const CrunchField* field) =0;
		virtual std::vector<std::string> getItemArray(const CrunchField* field) =0;
	};
}

#endif