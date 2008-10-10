#include "manager.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <signal.h>
#include <assert.h>

static void sighup_handler(int _sig) {
	assert(_sig == SIGHUP);
	try {
		Config::ConfigManager::instance_ptr()->reload();
	}
	catch (std::exception& ex) {
		std::cerr << "ERROR: " << ex.what() << std::endl;
	}
}


int main(int argc, char *argv[]) {

	po::options_description cmd;
	cmd.add_options() 
		("config-test", "Run configuration checker");
	po::options_description file;
	file.add_options() 
		("database.host", po::value<std::string>(), "Database hostname")
		("database.port", po::value<int>(), "Database listening port")
		("database.name", po::value<std::string>(), "Database name to use")
		("database.user", po::value<std::string>(), "Database username")
		("database.pass", po::value<std::string>(), "Database password")
		("nameservice.host", po::value<std::string>(), "CORBA nameservice hostname");

	try {
		Config::Manager &cfm = Config::ConfigManager::instance_ref();
		
		cfm.init(argc, argv);
		cfm.setCmdLineOptions(cmd);
		cfm.setCfgFileOptions(file, CONFIG_FILE);
		cfm.parse();
		signal(SIGHUP, sighup_handler);
		
		if (cfm.isHelp()) {
			std::cout << "Usage: " << argv[0] << " [options]" << std::endl;
			std::cout << "Options:" << std::endl;
			cfm.printAvailableOptions(std::cout);
			return 0;
		}
		if (cfm.isVersion()) {
			std::cout << PACKAGE_STRING 
					  << " (built " << __DATE__ << " " << __TIME__ << ")" 
					  << std::endl;
			return 0;
		}
								
		const Config::Conf &cfg1 = cfm.get();
        if (cfg1.hasUnknown()) {
            std::vector<std::string> un(cfg1.getUnknown());
            std::cout << "Unknown options: " << std::endl;
            for (int i = 0; i < (int)un.size(); i++) {
                std::cout << un[i] << std::endl;
            }
            std::cout << std::endl;
        }
		const std::string &db_host = cfg1.get<std::string>("database.host");
		
		while (1) {
			try {
				// cfg1.print(std::cout);
				// std::cout << cfg1.get<std::string>("database.host") << std::endl;
				std::cout << db_host << std::endl;
			}
			catch (Config::Conf::OptionNotFound& ex) {
				std::cerr << "ERROR: " << ex.what() << std::endl;
			}
			sleep(3);
		}
		
	}
	catch (Config::Manager::ConfigParseError& ex) {
		std::cerr << "ERROR: " << ex.what() << std::endl;
		std::cerr << "Exiting." << std::endl;
	}
	catch (Config::Conf::OptionNotFound& ex) {
		std::cerr << ex.what() << std::endl;
		return 1;
	}
	return 0;
}
