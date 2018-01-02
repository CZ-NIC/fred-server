#ifndef FILTER_IT_HH_AA110E6014DB45A783CF3AF9CD2891F4
#define FILTER_IT_HH_AA110E6014DB45A783CF3AF9CD2891F4

#include <vector>
#include "src/util/db/query/filter.hh"

namespace Database {
namespace Filters {

class Iterator {
public:
	Iterator(std::vector<Filter *>& fl) :
	flist(fl), it(fl.begin()) {
	}
	void first() {
		it = flist.begin();
	}
	void next() {
		if (!isDone())
		it++;
	}
	bool isDone() {
		return it == flist.end();
	}
	Filter* get() {
		return isDone() ? NULL : *it;
	}
private:
	std::vector<Filter *>& flist;
	std::vector<Filter *>::iterator it;
};

}
}

#endif /*FILTER_IT_H_*/
