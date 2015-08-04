#include "crunch.hpp"
#include "version.hpp"
#include "util.hpp"
#include "crunchconfig.hpp"
#include <iostream>
#include <fstream>
#include <boost/program_options.hpp>
#include <boost/program_options/variables_map.hpp>
using namespace std;


/**  crunch entry point **/

/* will handle command line options, create a crunch instance, initialize it and run the "crunch::start" method */

int main (int argc, char *argv[])
{

	/***
	**
	** handle command line parameters
	**
	*/

	namespace po = boost::program_options;
	
	std::string configPath;
	po::options_description desc("Allowed options");
	desc.add_options()
	("help", "produce help message")
    ("version", "identify version of the crunch binary")
    ("verify-config", "verify json conf and die")
	("config-file", po::value<std::string>(&configPath), "absolute or relative path of config file");
	
	po::variables_map vm;
	store(parse_command_line(argc, argv, desc), vm);
	notify(vm);

    if (vm.count("version"))
    {
        crunch_version();
        return 0;
    }

	if (vm.count("help"))
	{
		cout << desc << "\n";
		return 0;
	}

	if (!vm.count("config-file"))
	{
		cout << "Configuration file has to be specified. Try crunch --help" << "\n";
		return 0;
	}
 
	std::ifstream configfile(configPath.c_str());
	if(configfile.fail()) 
	{
		cout << "Config file not found." << "\n";
		return 0;
	}
	configfile.close();

	/***
	**
	** initialize crunch
	**
	*/

	crunch::Crunch crunchInstance;


	//run init method
	try{
		crunchInstance.init(configPath);
	} 
	catch (const std::runtime_error& ex) {
		crunch::Util::log(std::string("Exception raised during init :")+ex.what(),logERROR);
		return 1;
	}
	catch (const std::logic_error& ex) {
		crunch::Util::log(std::string("Exception raised during init :")+ex.what(),logERROR);
		return 1;
	}
	catch (...) {
		cout << "Unkown exception raised during init. Exiting. \n";
		return 1;
	}

    if (vm.count("verify-config"))
    {
        cout << "OK\n";
        return 0;
    }


    if (crunchInstance.getVerifyConfig())
    {
        cout << " verify config activated \n";
        cout << "OK\n";
        return 0; 
    }

	/***
	**
	** start crunch
	**
	*/
	try{
		crunchInstance.start();
	}
	catch (const std::runtime_error& ex) {
		crunch::Util::log(std::string("Exception raised during init :")+ex.what(),logERROR);
		return 1;
	}


	return 0;
}