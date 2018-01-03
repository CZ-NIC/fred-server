#ifndef COMMON_NEW_HH_A99B05C4BC0242E9A134DB2FC71F6F13
#define COMMON_NEW_HH_A99B05C4BC0242E9A134DB2FC71F6F13

#include "src/deprecated/model/model_filters.hh"

namespace LibFred {

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
    bool operator()(LibFred::CommonObjectNew* _object)
    {
        if(_object == 0) throw std::runtime_error("operator() LibFred::CommonObjectNew* _object is null");

        T* t_ptr = dynamic_cast<T*>(_object);

        if(t_ptr == 0) throw std::runtime_error(
                std::string("operator() LibFred::CommonObjectNew* _object is not ")
                + std::string(typeid(T*).name()));
      return t_ptr->getId() == find_id_;
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
    virtual void setTimeout(unsigned _timeout) = 0;
    virtual void setLimit(unsigned int _limit) = 0;
    virtual unsigned int getLimit() const = 0;
};



} // namespace LibFred

#endif
