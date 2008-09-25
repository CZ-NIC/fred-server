#ifndef CONFIG_MANAGER_H_
#define CONFIG_MANAGER_H_

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
  void setCfgFileOptions(const po::options_description& _opts,
                         const std::string& _default_path = "", 
                         bool _override = false);
  void parse();
  void printAvailableOptions(std::ostream& _ofs);
  void reload();
  
  bool isHelp() const { return data.count("help"); }
  bool isVersion() const { return data.count("version"); }
  
  const Conf& get() const;

private:
  bool initialized_;                     ///< flag signalize that manager was initialized by argc and argv variables
  int argc;                              ///< should be passed from main()
  char **argv;                           ///< should be passed from main()
  bool allow_file_opts_override_;        ///< allow to override file options at command-line
  po::options_description basic_opts;    ///< common options as -h (--help) -v (--version)
  po::options_description cmd_line_opts; ///< command-line options
  po::options_description cfg_file_opts; ///< config file options
  Conf data;                             ///< parsed options

  /* internal methods */
  void _init();
  void _parse(Conf &_conf) throw(ConfigParseError);
  void _parseCmdLine(Conf &_conf);
  void _parseCfgFile(const std::string& _name, Conf &_conf) throw(ConfigParseError);
  void _parseCfgFileManual(const std::string& _name, Conf &_conf) throw(ConfigParseError);
  
  
};

typedef Singleton<Config::Manager> ConfigManager;

}

#endif /*CONFIG_MANAGER_H_*/
