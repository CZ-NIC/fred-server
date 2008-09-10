#include <iostream>
#include "conf.h"

namespace Config {

void Conf::print(std::ostream& _os) const {
	const_iterator it = begin();
	
	for (; it != end(); ++it) {
		boost::any val = (it->second).value();
		_os << it->first << " = ";
		
    if (val.type() == typeid(unsigned))
      _os << boost::any_cast<unsigned>(val);
		if (val.type() == typeid(int))
			_os << boost::any_cast<int>(val);
    if (val.type() == typeid(bool))
      _os << boost::any_cast<bool>(val);
		if (val.type() == typeid(std::string))
			_os << boost::any_cast<std::string>(val);
		
		_os << std::endl;
	}
}

}
