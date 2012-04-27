#ifndef UNION_FILTER_H_
#define UNION_FILTER_H_

#include <vector>
#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/vector.hpp>

#include "db/query/query_old.h"
#include "db/query/simple_filter.h"
#include "db/query/simple_filters.h"
#include "db/query/compound_filter.h"
#include "db/query/filter_it.h"
#include "model_filters.h"

#include "log/logger.h"

#include <boost/shared_ptr.hpp>

namespace Database {
namespace Filters {

class Union {
public:
  Union(const Settings *_ptr = 0) : settings_ptr_(_ptr) {
  }

  virtual ~Union() {
    clear();
  }

  virtual void addFilter(Compound *cf) {
    filter_list.push_back(cf);
  }

  virtual Filter* getFilter(unsigned idx) {
    return filter_list.at(idx);
  }

  virtual void addQuery(Database::SelectQuery *_q) {
    query_list.push_back(_q);
  }

  virtual bool empty() const {
    return filter_list.empty();
  }

  void clearFilters();
  void clearQueries();
  void clear();

  // DEPRECATED; should be use std::vector<> drawed out iterator
  Iterator *createIterator() {
    return new Iterator(filter_list);
  }

  typedef std::vector<Filter*>::iterator iterator;
  iterator begin() {
    return filter_list.begin();
  }
  iterator end() {
    return filter_list.end();
  }

  virtual void serialize(Database::SelectQuery& _q);

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    /*
     * Need registering all classes which can be hold in 'filter_list'
     * by Filter* pointer
     * CAUTION: Do not change the class order, append new clases after
     * last otherwise saved filters will be unable to load
     */
    _ar.register_type(static_cast<Simple *>(NULL));
    _ar.register_type(static_cast<Value<ObjectType> *>(NULL));
    _ar.register_type(static_cast<Value<int> *>(NULL));
    _ar.register_type(static_cast<Value<unsigned> *>(NULL));
    _ar.register_type(static_cast<Value<std::string> *>(NULL));
    _ar.register_type(static_cast<Value<Database::ID> *>(NULL));
    _ar.register_type(static_cast<Value<bool> *>(NULL));
    _ar.register_type(static_cast<_BaseDTInterval<Database::DateTimeInterval> *>(NULL));
    _ar.register_type(static_cast<_BaseDTInterval<Database::DateInterval> *>(NULL));
    _ar.register_type(static_cast<Interval<Database::DateTimeInterval> *>(NULL));
    _ar.register_type(static_cast<Interval<Database::DateInterval> *>(NULL));
    _ar.register_type(static_cast<Compound *>(NULL));
    _ar.register_type(static_cast<ObjectRegistryImpl *>(NULL));
    _ar.register_type(static_cast<ObjectImpl *>(NULL));
    _ar.register_type(static_cast<ObjectHistoryImpl *>(NULL));
    _ar.register_type(static_cast<DomainImpl *>(NULL));
    _ar.register_type(static_cast<DomainHistoryImpl *>(NULL));
    _ar.register_type(static_cast<ContactImpl *>(NULL));
    _ar.register_type(static_cast<ContactHistoryImpl *>(NULL));
    _ar.register_type(static_cast<NSSetImpl *>(NULL));
    _ar.register_type(static_cast<NSSetHistoryImpl *>(NULL));
    _ar.register_type(static_cast<RegistrarImpl *>(NULL));
    _ar.register_type(static_cast<PublicRequestImpl *>(NULL));
    _ar.register_type(static_cast<InvoiceImpl *>(NULL));
    _ar.register_type(static_cast<FileImpl *>(NULL));
    _ar.register_type(static_cast<MailImpl *>(NULL));
    _ar.register_type(static_cast<KeySetImpl *>(NULL));
    _ar.register_type(static_cast<KeySetHistoryImpl *>(NULL));
    _ar.register_type(static_cast<ObjectStateImpl *>(NULL));
    _ar.register_type(static_cast<OnlineStatementImpl *>(NULL));
    _ar.register_type(static_cast<BankPaymentImpl *>(NULL));
    // _ar.register_type(static_cast<StatementHeadImpl *>(NULL));
    _ar.register_type(static_cast<RequestImpl *>(NULL));
    _ar.register_type(static_cast<RequestDataImpl *>(NULL));
    _ar.register_type(static_cast<RequestPropertyValueImpl *>(NULL));
    _ar.register_type(static_cast<ServiceType *>(NULL));
    _ar.register_type(static_cast<RequestType *>(NULL));
    _ar.register_type(static_cast<MessageImpl *>(NULL));

    _ar & BOOST_SERIALIZATION_NVP(filter_list);
  }

  void settings(const Settings *_ptr) {
    settings_ptr_ = _ptr;
  }

  const Settings* settings() const {
    return settings_ptr_;
  }

protected:
  std::vector<Filter*> filter_list;
  std::vector<Database::SelectQuery*> query_list;

  const Settings *settings_ptr_;
};//class Union


typedef boost::shared_ptr<Union> UnionPtr;
template < typename DELETER >
class CreateUnionPtrT
{
protected:
    UnionPtr m_ptr;
public:
    CreateUnionPtrT()
    : m_ptr(new Union(),DELETER())
    {
        TRACE(boost::format("[CALL] CreateUnionPtrT::CreateUnionPtrT Union* m_ptr: '%1% ") % m_ptr.get());
    }

    operator UnionPtr() const
    {
        return m_ptr;
    }

};
///deleter functor for Union
struct ClearDeleteUnion
{
    void operator()(Union* u)
    {
        TRACE(boost::format("[CALL] ClearDeleteUnion::operator(Union* u: '%1%' )") % u);
        try
        {
            if(u) u->clear();
        }
        catch(...){}
        delete u;
    }
};
///UnionPtr factory
typedef CreateUnionPtrT<ClearDeleteUnion> CreateClearedUnionPtr;

}
}

#endif /*UNION_FILTER_H_*/
