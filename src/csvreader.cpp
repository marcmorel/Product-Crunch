#include "csvreader.hpp"
#include "util.hpp"

crunch::CSVReader* crunch::CSVReader::This = NULL;


/**
* Default constructor
* Will create a root category
*/
crunch::CSVReader::CSVReader(CrunchConfig* config) : Reader(config)
{
	crunch::CSVReader::This = this;

    m_separator = new boost::escaped_list_separator<char>( '\\', m_crunchConfig->m_CSVdelimiter, '\"' );

}

crunch::CSVReader::~CSVReader()
{
delete m_separator;
m_header.clear();
m_fileStream.close();
 #ifdef __DEBUG__
std::cout << " destructor CSVReader \n";
#endif
}


bool crunch::CSVReader::openSourceFile()
{
	Util::log(std::string("Ouverture de :")+m_sourceFile+std::string("\n"), logTRACE);

    
    m_fileStream .open(m_sourceFile.c_str(),std::ifstream::in);
    if (!m_fileStream.is_open()) return false;


    //get all CSV field names and put them in m_header string vector.
    std::string firstline = getNextLine();
    CSVTokenizer tok(firstline, *m_separator);
    m_header.assign(tok.begin(),tok.end());
    m_lineNumber = 1;
    m_headerSize = m_header.size();

    //prepare regexp to replace CSV fields in XML fields definition
    std::string fieldsRegexpString("\\{\\{([^{}]*)\\}\\}");
    m_fieldsRegexp.assign(fieldsRegexpString);
    
	return true;
}



/** 
* Return a string containing next CSV line from file. 
* Handles correctly fields containing newline car (multiple-line fields)
*/
std::string crunch::CSVReader::getNextLine()
{
	bool inside_quotes(false);
    size_t last_quote(0);
    std::string line;
    std::string buffer;

    while (Util::safeGetline(m_fileStream,buffer))
    {
         #ifdef __DEBUG__
         Util::ReplaceStringInPlace(buffer,std::string("\r"),std::string("\r\n"));
            Util::log(std::string("BUFFER ")+buffer+std::string("\n"), logTRACE);
         #endif
        //ignore empty lines
        
        if(!buffer.length()) continue;
        // --- deal with line breaks in quoted strings

        last_quote = buffer.find_first_of('"');
        while (last_quote != std::string::npos)
        {
            inside_quotes = !inside_quotes;
            last_quote = buffer.find_first_of('"',last_quote+1);
        }

        line.append(buffer);

        if (inside_quotes)
        {
            line.append("\n");
        }
        else break;
    }
    #ifdef __DEBUG__

            Util::log(std::string("CSV LINE ")+line+std::string("\n"), logTRACE);
         #endif    
    return line;

}



/**
* Iterator function
* @return true if next item has been found
*/
bool crunch::CSVReader::hasNextItem()
{
    m_lineNumber++;

    //grab next line of source file
	std::string line = getNextLine();
    std::vector<std::string> currentFields;
	if(!line.size())
		return false;

    //explode string with current separator
	CSVTokenizer tok(line, *m_separator);
    currentFields.assign(tok.begin(),tok.end());
    size_t size = currentFields.size();

    //check if number of  fields found in current line is consistent with CSV header
    if(!size) return false; //empty line found
    if(size != m_headerSize)
    {
        std::stringstream err_str; err_str << "Wrong number of csv fields at line " << m_lineNumber << "\n" << line << "\n";
        throw CrunchWrongKeyException(err_str.str());
    }

    m_currentItem.clear();
     //loop on item values
    for ( unsigned int index = 0; index < m_headerSize; index++ )
    {
        m_currentItem.insert(std::make_pair<std::string,std::string>(m_header[index],currentFields[index]));
    }
    return true;
}

std::string crunch::CSVReader::getItemValue(const CrunchField* crunchfield)
{
#ifdef __DEBUG__
std::cout << "csv reader - getItemValue search for key[" <<  crunchfield->m_replacementName << "] \n";
#endif
    //search for field in CSV field definition
    if(m_crunchConfig->m_CSVfields.count(crunchfield->m_replacementName)==0)
       return std::string("");
   CSVfield csvfield = m_crunchConfig->m_CSVfields [ crunchfield->m_replacementName ];
   //parsing du csvfield et remplacement des {{ }}
    std::string result=boost::regex_replace(
                        csvfield.m_value,
                        m_fieldsRegexp,
                        &crunch::CSVReader::regexFieldCallback);

#ifdef __DEBUG__

  Util::log(std::string("Get item value  ")+ crunchfield->m_replacementName+std::string("-->")+csvfield.m_value+std::string("-->")+result+std::string("\n"), logTRACE);
         #endif   



   return result;

}

/* regexFieldCallback is a callback function (and static for ease of development and runtime speed)
called for each {{ field }}
*/
std::string crunch::CSVReader::regexFieldCallback(boost::match_results<std::string::const_iterator> match)
{
    return crunch::CSVReader::This->m_currentItem[match[1]];
}



std::vector<std::string> crunch::CSVReader::getItemArray(const CrunchField* crunchfield)
{
     //search for field in CSV field definition
    if(m_crunchConfig->m_CSVfields.count(crunchfield->m_replacementName)==0)
       return std::vector<std::string> ();
    CSVfield csvfield = m_crunchConfig->m_CSVfields [ crunchfield->m_replacementName ];
  
    std::vector<std::string> array_result;
    for(unsigned short index=0; index < csvfield.m_arrayValues.size(); index++)
    {
            std::string result=boost::regex_replace(
                        csvfield.m_arrayValues[index],
                        m_fieldsRegexp,
                        &crunch::CSVReader::regexFieldCallback);
            array_result.push_back(result);
    }
	return array_result;
}