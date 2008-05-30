#ifndef MANAGER_H_
#define MANAGER_H_

#include <vector>
#include <string>
#include <exception>
#include <iostream>

#include "conf.h"
#include "singleton.h"

namespace Config {

class Manager {
public:
	class ConfigParseError : public std::exception {
	public:
		ConfigParseError(const std::string& __what) throw() : _what(__what) { }
		~ConfigParseError() throw() { }
		virtual const char* what() const throw() { return _what.c_str(); }
	protected:
		std::string _what;
	};
	
	Manager(int _argc, char *_argv[]);
	Manager();
	~Manager();
	
	void init(int _argc, char *_argv[]);
	void setCmdLineOptions(const po::options_description& _opts);
	void setCfgFileOptions(const po::options_description& _opts, const std::string& _default_path = "");
	void parse();
	void printAvailableOptions(std::ostream& _ofs);
	void reload();
	
	bool isHelp() const { return data.count("help"); }
	bool isVersion() const { return data.count("version"); }
	
	const Conf& get() const;
		
private:
	bool m_initialized;
	void _init();
	void _parse(Conf &_conf) throw(ConfigParseError);
	void _parseCmdLine(Conf &_conf);
	void _parseCfgFile(const std::string& _name, Conf &_conf) throw(ConfigParseError);
	
	int argc;
	char **argv;
	
	po::options_description basic_opts;
	po::options_description cmd_line_opts;
	po::options_description cfg_file_opts;
	
	Conf data;
};

typedef Singleton<Config::Manager> ConfigManager;

}

#endif /*MANAGER_H_*/
