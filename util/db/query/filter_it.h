#ifndef FILTER_IT_H_
#define FILTER_IT_H_

#include <vector>
#include "filter.h"

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
