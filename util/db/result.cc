#include "result.h"

namespace DBase {

Result::~Result() {
}

Result__::Result__(const Connection *_conn, const Query& _query) :
	conn(_conn), query(_query) {
}

Result__::~Result__() {
}

const Query& Result__::getQuery() const {
	return query;
}

}
