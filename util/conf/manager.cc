#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/erase.hpp>

#include <fstream>
#include <vector>
#include "manager.h"

namespace Config {

Manager::Manager(int _argc,
                 char* _argv[]) : initialized_(true), 
                                  argc(_argc), 
                                  argv(_argv) { 
	_init();
}

Manager::Manager() : initialized_(false) {
}

Manager::~Manager() {
}

void Manager::init(int _argc, char* _argv[]) {
	argc = _argc;
	argv = _argv;
	initialized_ = true;
	_init();
}

void Manager::_init() {
	basic_opts.add_options() 
    ("help,h", "Show this help")
    ("version,v", "Print version and exit");
}

void Manager::_parseCmdLine(Conf &_conf) {
  po::options_description tmp;
  /* command-line options should be parsed always */
  tmp.add(basic_opts);
  if (allow_file_opts_override_) {
    /* we want to file options be overriden by command-line ones */
    tmp.add(cfg_file_opts);
  }

  po::parsed_options cmd_parsed = po::command_line_parser(argc, argv).options(tmp).allow_unregistered().run();
  std::vector<std::string> unknown = po::collect_unrecognized(cmd_parsed.options, po::include_positional);
  po::store(cmd_parsed, _conf);
  _conf.setUnknown(unknown);
}

void Manager::_parseCfgFile(const std::string& _name, Conf &_conf) throw(ConfigParseError) {
	std::ifstream cfg_file(_name.c_str());
	if (cfg_file.fail()) {
		throw ConfigParseError("config file '" + _name + "' not found");
	}
	try {
		po::store(po::parse_config_file(cfg_file, cfg_file_opts), _conf);
		cfg_file.close();
	}
	catch (std::exception& ex) {
		throw ConfigParseError(ex.what());
	}
}

void Manager::_parseCfgFileManual(const std::string& _name, Conf &_conf) throw (ConfigParseError) {
  std::ifstream cfg_file(_name.c_str());
  if (cfg_file.fail()) {
    throw ConfigParseError("config file '" + _name + "' not found");
  }

  int resultc = 0;
  std::string line, opt_prefix;
  std::vector<std::string> result;

  while (std::getline(cfg_file, line)) {
    /* strip whitepsace */
    boost::algorithm::trim(line);
    /* ignore empty line and comments */
    if (!line.empty() && line[0] != '#') {
      if (line[0] == '[' && line[line.size() - 1] == ']') {
        /* this is option prefix */
        opt_prefix = line;
        boost::algorithm::erase_first(opt_prefix, "[");
        boost::algorithm::erase_last(opt_prefix,  "]");
      }
      else {
        /* this is normal option */
        std::string::size_type sep = line.find("=");
        if (sep != std::string::npos) {
          /* get name and value couple without any whitespace */
          std::string name  = boost::algorithm::trim_copy(line.substr(0, sep));
          std::string value = boost::algorithm::trim_copy(line.substr(sep + 1, line.size() - 1));

          if (!value.empty()) {
            /* push appropriate commnad-line string */
            result.push_back("--" + opt_prefix + "." + name + "=" + value);
            resultc += 1;
          }
        }
      }
    }
  }

  try {
    po::parsed_options file_parsed = po::command_line_parser(result).options(cfg_file_opts).allow_unregistered().run();
    po::store(file_parsed, _conf);
  }
  catch (std::exception &ex) {
    throw ConfigParseError(ex.what());
  }
}

void Manager::setCmdLineOptions(const po::options_description& _opts) {
	cmd_line_opts.add(_opts);
	basic_opts.add(cmd_line_opts);
}

void Manager::setCfgFileOptions(const po::options_description& _opts,
                                const std::string& _default, 
                                bool _override) {
  allow_file_opts_override_ = _override;
	cfg_file_opts.add(_opts);

	if (!_default.empty()) {
		basic_opts.add_options()("config,C", po::value<std::string>()->default_value(_default), "Path to configuration file");
	} else {
		basic_opts.add_options()("config,C", po::value<std::string>(), "Path to configuration file");
	}
}

void Manager::_parse(Conf &_conf) throw(ConfigParseError) {
	try {
		_parseCmdLine(_conf);
		if (data.count("config")) {
			_parseCfgFileManual(_conf["config"].as<std::string>(), _conf);
		}
    po::notify(_conf);
	}
	catch (std::exception& ex) {
		throw ConfigParseError(ex.what());
	}
}

void Manager::parse() {
	try {
		_parse(data);
	}
	catch (...) {
		throw;
	}
}

void Manager::printAvailableOptions(std::ostream& _ofs) {
	_ofs << basic_opts << std::endl;
}

const Conf& Manager::get() const {
	return data;
}

void Manager::reload() {
	Conf new_data;
	try {
		_parse(new_data);
	}
	catch (...) {
		throw;
	}
	data = new_data;
}

}
