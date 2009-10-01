#ifndef _COMMON_NEW_H_
#define _COMMON_NEW_H_

#include "model/model_filters.h"

namespace Register {

class CommonObjectNew {
public:
    virtual ~CommonObjectNew()
    { }
};

template <typename T> struct CheckIdNew
{
	unsigned long long find_id_;
    CheckIdNew(unsigned long long _id)
		  :find_id_(_id)
	{}
    bool operator()(Register::CommonObjectNew* _object)
    {
      return (dynamic_cast<T*>(_object))->getId() == find_id_;
    }
};

class CommonListNew {
public:
    virtual ~CommonListNew()
    { }
    virtual CommonObjectNew *get(unsigned int index) const = 0;
    virtual unsigned int getSize() const = 0;
    virtual unsigned int size() const = 0;
    virtual void clear() = 0;
    virtual bool isEmpty() = 0;
    virtual void appendToList(CommonObjectNew *object) = 0;
    virtual const char *getTempTableName() const = 0;
    virtual unsigned long long getRealCount(Database::Filters::Union &filter) = 0;
    virtual bool isLimited() const = 0;
};



} // namespace Register

#endif  // _COMMON_NEW_H_
