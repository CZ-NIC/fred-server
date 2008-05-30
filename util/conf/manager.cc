#include <fstream>
#include "manager.h"

namespace Config {

Manager::Manager(int _argc, char* _argv[]) : m_initialized(true), 
	argc(_argc), argv(_argv) {
	_init();
}

Manager::Manager() : m_initialized(false) {
}

Manager::~Manager() {
}

void Manager::init(int _argc, char* _argv[]) {
	argc = _argc;
	argv = _argv;
	m_initialized = true;
	_init();
}

void Manager::_init() {
	basic_opts.add_options() ("help,h", "Show this help") ("version,v",
			"Print version and exit");
}

void Manager::_parseCmdLine(Conf &_conf) {
	po::store(po::parse_command_line(argc, argv, basic_opts), _conf);
	po::notify(_conf);
}

void Manager::_parseCfgFile(const std::string& _name, Conf &_conf) throw(ConfigParseError) {
	std::ifstream cfg_file(_name.c_str());
	if (cfg_file.fail()) {
		throw ConfigParseError("config file '" + _name + "' not found");
	}
	try {
		po::store(po::parse_config_file(cfg_file, cfg_file_opts), _conf);
		po::notify(_conf);
		cfg_file.close();
	}
	catch (std::exception& ex) {
		throw ConfigParseError(ex.what());
	}
}

void Manager::setCmdLineOptions(const po::options_description& _opts) {
	cmd_line_opts.add(_opts);
	basic_opts.add(cmd_line_opts);
}

void Manager::setCfgFileOptions(const po::options_description& _opts,
		const std::string& _default) {
	cfg_file_opts.add(_opts);

	if (!_default.empty()) {
		basic_opts.add_options()("conf", po::value<std::string>()->default_value(_default), "Path to configuration file");
	} else {
		basic_opts.add_options()("conf", po::value<std::string>(), "Path to configuration file");
	}
}

void Manager::_parse(Conf &_conf) throw(ConfigParseError) {
	try {
		_parseCmdLine(_conf);
		if (data.count("conf")) {
			std::cout << "parsing given config file: " << _conf["conf"].as<std::string>() << std::endl;
			_parseCfgFile(_conf["conf"].as<std::string>(), _conf);
		}
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
