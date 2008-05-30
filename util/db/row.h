#ifndef ROW_H_
#define ROW_H_

#include <algorithm>
#include <vector>

namespace DBase {

class Row {
public:
	Row() :
		result_ptr(0) {
	}
	Row(const Result__ *_result_ptr) :
		result_ptr(_result_ptr) {
	}
	Row(const Result__ *_result_ptr, std::vector<Value>& _data) : result_ptr(_result_ptr) {
		data.assign(_data.begin(), _data.end());
	}
	
	unsigned size() const {
		return data.size();
	}
	const Value& operator[](unsigned _n) const {
		return data.at(_n);
	}
	
	typedef std::vector<Value>::iterator Iterator;
	typedef std::vector<Value>::const_iterator ConstIterator;
	
	Iterator begin() { return data.begin(); }
	Iterator end() { return data.end(); }
	ConstIterator begin() const { return data.begin(); }
	ConstIterator end() const { return data.end(); }
	
protected:
	const Result__ *result_ptr;
	std::vector<Value> data;
};

}

#endif /*ROW_H_*/
