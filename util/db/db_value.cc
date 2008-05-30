#include "db_value.h"

namespace DBase {

std::ostream& operator<<(std::ostream &_os, const Value& _v) {
	if (_v.isNull()) {
		Null<void> n;
		return _os << n;
	} else {
		return _os << (_v.quoted ? "'" : "") << _v.value << (_v.quoted ? "'" : "");
	}
}

bool operator==(const std::string& _str, const Value& _v) {
  return _str == _v.value;
}

}
