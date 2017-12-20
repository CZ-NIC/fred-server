#include "src/util/db/query/sql_helper_objects.hh"

namespace Database {

std::ostream& operator<<(std::ostream& _os, Table& _t) {
	return _os << _t.name << " " << _t.alias;
}

std::ostream& operator<<(std::ostream& _os, Column& _c) {
	bool f = _c.hasFunct();
	return _os << (f ? (_c.funct + "(") : "") << (_c.table)->getAlias() << "." << _c.name << (f ? ")" : "");
}

std::ostream& operator<<(std::ostream& _os, Condition& _c) {
	return _os << _c.left << " " << _c.op << " " << _c.right;
}

}
