#ifndef CONF_H_
#define CONF_H_

#include <boost/program_options.hpp>

namespace po = boost::program_options;

namespace Config {

class Conf : public po::variables_map {
public:
	class OptionNotFound : public std::exception {
	public:
		OptionNotFound(const std::string& __what) throw() :
			_what(__what) {
		}
		~OptionNotFound() throw() {
		}
		virtual const char* what() const throw() {
			return _what.c_str();
		}
	protected:
		std::string _what;
	};
	
	Conf() {
	}
	~Conf() {
	}
		
	void print(std::ostream& _os) const;
	
	template<class _Tp> const _Tp& get(const std::string& _what) const {
		try {
			return po::variables_map::operator[](_what).as<_Tp>();
			//return tmp;
		}
		catch (...) {
			throw OptionNotFound("option value '" + _what + "' not found in configuration");
		}
	}

    bool hasOpt(std::string name) const
    {
        return po::variables_map::count(name);
    }
    bool hasUnknown() const
    {
        return !m_unknown.empty();
    }
    std::vector<std::string> getUnknown() const
    {
        return m_unknown;
    }
    void setUnknown(std::vector<std::string> unknown)
    {
        m_unknown = unknown;
    }
private:
    std::vector<std::string> m_unknown;
};

}

#endif /*CONF_H_*/
