#include <math.h>
#include <memory>
#include <iomanip>
#include <corba/ccReg.hh>
#include <algorithm>
#include <boost/utility.hpp>

#include "common.h"
#include "filter_impl.h"
#include "old_utils/log.h"
#include "old_utils/dbsql.h"
#include "register/notify.h"
#include "register/filter.h"
#include "corba/mailer_manager.h"

#include "log/logger.h"

class FilterBaseImpl : virtual public POA_ccReg::Filters::Base {
protected:
  Database::Filters::Filter *f;
public:
  ~FilterBaseImpl() {
  }
  FilterBaseImpl(Database::Filters::Filter* _f) :
    f(_f) {
  }
  char* name() {
    return DUPSTRFUN(f->getName);
  }
  CORBA::Boolean neg() {
    return f->getNOT();
  }
  void neg(CORBA::Boolean newNeg) {
    f->setNOT(newNeg);
  }
  CORBA::Boolean isActive() {
    return f->isActive();
  }
};

class FilterSimpleImpl : virtual public POA_ccReg::Filters::Simple,
  public FilterBaseImpl {
public:
  FilterSimpleImpl(Database::Filters::Simple *f) :
    FilterBaseImpl(f) {
  }
};

class FilterStrImpl : virtual public POA_ccReg::Filters::Str,
  public FilterSimpleImpl {
  Database::Filters::Value<std::string>* get() {
    return dynamic_cast<Database::Filters::Value<std::string>*>(f);
  }
public:
  FilterStrImpl(Database::Filters::Value<std::string>* f) :
    FilterSimpleImpl(f) {
  }
  char* value() {
    return CORBA::string_dup(get()->getValue().c_str());
  }
  void value(const char *v) {
    get()->setValue(v);
  }
};

class FilterIntImpl : virtual public POA_ccReg::Filters::Int,
  public FilterSimpleImpl {
  Database::Filters::Value<int>* get() {
    return dynamic_cast<Database::Filters::Value<int>*>(f);
  }

public:
  FilterIntImpl(Database::Filters::Value<unsigned>* f) :
    FilterSimpleImpl(f) {
  }
  FilterIntImpl(Database::Filters::Value<int>* f) :
    FilterSimpleImpl(f) {
  }
  CORBA::Short value() {
    return get()->getValue().getValue();
  }
  void value(CORBA::Short v) {
    get()->setValue(v);
  }
};

class FilterIdImpl : virtual public POA_ccReg::Filters::Id,
  public FilterSimpleImpl {
  Database::Filters::Value<Database::ID>* get() {
    return dynamic_cast<Database::Filters::Value<Database::ID>*>(f);
  }

public:
  FilterIdImpl(Database::Filters::Value<Database::ID>* f) :
    FilterSimpleImpl(f) {
  }
  ccReg::TID value() {
    return get()->getValue().getValue();
  }
  void value(ccReg::TID v) {
    get()->setValue(Database::ID(v));
  }
};

class FilterIntIntervalImpl : virtual public POA_ccReg::Filters::IntInterval,
                              public FilterSimpleImpl {
  Database::Filters::Interval<int>* get() {
    return dynamic_cast<Database::Filters::Interval<int>*>(f);
  }

public:
  FilterIntIntervalImpl(Database::Filters::Interval<int>* f) : FilterSimpleImpl(f) {
  }
  ccReg::IntInterval value() {
    ccReg::IntInterval v;
    v.from = get()->getValueBeg();
    v.to   = get()->getValueEnd();
    return v;
  }
  void value(const ccReg::IntInterval& v) {
    get()->setValue(v.from, v.to);
  }
};

class FilterDateImpl : virtual public POA_ccReg::Filters::Date,
  public FilterSimpleImpl {
  Database::Filters::Interval<Database::DateInterval>* get() {
    return dynamic_cast<Database::Filters::Interval<Database::DateInterval>*>(f);
  }
public:
  FilterDateImpl(Database::Filters::Interval<Database::DateInterval>* f) :
    FilterSimpleImpl(f) {
  }
#define DCASE1(x) case ccReg::x : d = Database::x; break
#define DCASE2(x) case Database::x : cdi.type = ccReg::x; break
  ccReg::DateInterval value() {
    Database::DateInterval ddi = get()->getValue();
    ccReg::DateInterval cdi;
    switch (ddi.getSpecial()) {
      DCASE2(NONE);
      DCASE2(DAY);
      DCASE2(INTERVAL);
      DCASE2(LAST_HOUR);
      DCASE2(LAST_DAY);
      DCASE2(LAST_WEEK);
      DCASE2(LAST_MONTH);
      DCASE2(LAST_YEAR);
      DCASE2(PAST_HOUR);
      DCASE2(PAST_DAY);
      DCASE2(PAST_WEEK);
      DCASE2(PAST_MONTH);
      DCASE2(PAST_YEAR);
    }
    cdi.offset = ddi.getSpecialOffset();
    cdi.from = makeCorbaDate(ddi.begin().get());
    cdi.to = makeCorbaDate(ddi.end().get());
    return cdi;
  }
  void value(const ccReg::DateInterval& v) {
    Database::DateTimeIntervalSpecial d = Database::INTERVAL;
    switch (v.type) {
      DCASE1(NONE);
      DCASE1(DAY);
      DCASE1(INTERVAL);
      DCASE1(LAST_HOUR);
      DCASE1(LAST_DAY);
      DCASE1(LAST_WEEK);
      DCASE1(LAST_MONTH);
      DCASE1(LAST_YEAR);
      DCASE1(PAST_HOUR);
      DCASE1(PAST_DAY);
      DCASE1(PAST_WEEK);
      DCASE1(PAST_MONTH);
      DCASE1(PAST_YEAR);
    }
    get()->setValue(Database::DateInterval(d, v.offset, makeBoostDate(v.from),
        makeBoostDate(v.to)) );
  }
};

class FilterDateTimeImpl : virtual public POA_ccReg::Filters::DateTime,
  public FilterSimpleImpl {
  Database::Filters::Interval<Database::DateTimeInterval>* get() {
    return dynamic_cast<Database::Filters::Interval<Database::DateTimeInterval>*>(f);
  }
public:
  FilterDateTimeImpl(Database::Filters::Interval<Database::DateTimeInterval>* f) :
    FilterSimpleImpl(f) {
  }
#define DTCASE1(x) case ccReg::x : d = Database::x; break
#define DTCASE2(x) case Database::x : di.type = ccReg::x; break
  ccReg::DateTimeInterval value() {
    Database::DateTimeInterval dti = get()->getValue();
    ccReg::DateTimeInterval di;
    switch (dti.getSpecial()) {
      DTCASE2(NONE);
      DTCASE2(DAY);
      DTCASE2(INTERVAL);
      DTCASE2(LAST_HOUR);
      DTCASE2(LAST_DAY);
      DTCASE2(LAST_WEEK);
      DTCASE2(LAST_MONTH);
      DTCASE2(LAST_YEAR);
      DTCASE2(PAST_HOUR);
      DTCASE2(PAST_DAY);
      DTCASE2(PAST_WEEK);
      DTCASE2(PAST_MONTH);
      DTCASE2(PAST_YEAR);
    }
    di.offset = dti.getSpecialOffset();
    di.from = makeCorbaTime(dti.begin().get());
    di.to = makeCorbaTime(dti.end().get());
    return di;
  }
  void value(const ccReg::DateTimeInterval& v) {
    Database::DateTimeIntervalSpecial d = Database::INTERVAL;
    switch (v.type) {
      DTCASE1(NONE);
      DTCASE1(DAY);
      DTCASE1(INTERVAL);
      DTCASE1(LAST_HOUR);
      DTCASE1(LAST_DAY);
      DTCASE1(LAST_WEEK);
      DTCASE1(LAST_MONTH);
      DTCASE1(LAST_YEAR);
      DTCASE1(PAST_HOUR);
      DTCASE1(PAST_DAY);
      DTCASE1(PAST_WEEK);
      DTCASE1(PAST_MONTH);
      DTCASE1(PAST_YEAR);
    }
    get()->setValue(Database::DateTimeInterval(d, v.offset, makeBoostTime(v.from),
        makeBoostTime(v.to)) );
  }
};

class FilterCompoundImpl : virtual public POA_ccReg::Filters::Compound,
  public FilterBaseImpl {
protected:
  FilterIteratorImpl it;
public:
  FilterCompoundImpl(Database::Filters::Compound* f) :
    FilterBaseImpl(f) {
    std::auto_ptr<Database::Filters::Iterator> cit(f->createIterator());
    for (; !cit->isDone(); cit->next()) {
      it.addFilter(cit->get());
    }
  }
  ccReg::Filters::Iterator_ptr getIterator() {
    return it._this();
  }
};

#define FILTER_ADD(t,fu)                    \
  virtual ccReg::Filters::t##_ptr fu() {    \
  return it.addE(&get()->fu());             \
}

#define FILTER_ADD_N(t,fu,dfu)              \
  virtual ccReg::Filters::t##_ptr fu() {    \
  return it.addE(get()->dfu());             \
}

#define COMPOUND_CLASS(ct,dt,cti,methods)                       \
class Filter##ct##Impl : virtual public POA_ccReg::Filters::ct, \
  public Filter##cti##Impl                                      \
{                                                               \
  Database::Filters::dt* get()                                     \
  {                                                             \
    return dynamic_cast<Database::Filters::dt*>(f);                \
  }                                                             \
public:                                                         \
  Filter##ct##Impl(Database::Filters::dt* f) :                     \
    Filter##cti##Impl(f)                                        \
  {}                                                            \
  methods                                                       \
}

COMPOUND_CLASS(Registrar, Registrar, Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Str, addHandle);
    FILTER_ADD(Str, addName);
    FILTER_ADD(Str, addOrganization);
    FILTER_ADD(Str, addCity);
    FILTER_ADD(Str, addCountry);
);

COMPOUND_CLASS(ObjectState, ObjectState, Compound,
    FILTER_ADD(Id, addStateId);
    FILTER_ADD(DateTime, addValidFrom);
    FILTER_ADD(DateTime, addValidTo);
);

COMPOUND_CLASS(Obj, Object, Compound,
    FILTER_ADD(Str, addHandle);
    FILTER_ADD(DateTime, addCreateTime);
    FILTER_ADD(DateTime, addUpdateTime);
    FILTER_ADD(DateTime, addTransferTime);
    FILTER_ADD(DateTime, addDeleteTime);
    FILTER_ADD(Str, addAuthInfo);
    FILTER_ADD(Registrar, addCreateRegistrar);
    FILTER_ADD(Registrar, addUpdateRegistrar);
    FILTER_ADD(Registrar, addRegistrar);
    FILTER_ADD(ObjectState, addObjectState);
);

COMPOUND_CLASS(Contact, Contact, Obj,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Str, addName);
    FILTER_ADD(Str, addOrganization);
    FILTER_ADD(Str, addCity);
    FILTER_ADD(Str, addEmail);
    FILTER_ADD(Str, addNotifyEmail);
    FILTER_ADD(Str, addVat);
    FILTER_ADD(Str, addSsn);
);

COMPOUND_CLASS(Domain, Domain, Obj,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Date, addExpirationDate);
    FILTER_ADD(Date, addOutZoneDate);
    FILTER_ADD(Date, addCancelDate);
    FILTER_ADD(Contact, addRegistrant);
    FILTER_ADD(Contact, addAdminContact);
    FILTER_ADD(Contact, addTempContact);
    FILTER_ADD(NSSet, addNSSet);
    FILTER_ADD(KeySet, addKeySet);
);

COMPOUND_CLASS(NSSet, NSSet, Obj,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Str, addHostFQDN);
    FILTER_ADD(Str, addHostIP);
    FILTER_ADD(Contact, addTechContact);
);

COMPOUND_CLASS(KeySet, KeySet, Obj,
        FILTER_ADD(Id, addId);
        FILTER_ADD(Contact, addTechContact);
);

COMPOUND_CLASS(Action, EppAction, Compound,
    FILTER_ADD(Registrar, addRegistrar);
    FILTER_ADD(Obj, addObject);    
    FILTER_ADD(Int, addType);
    FILTER_ADD(Int, addResponse);
    FILTER_ADD(DateTime, addTime);
    FILTER_ADD(Str, addClTRID);
    FILTER_ADD(Str, addSvTRID);
    FILTER_ADD(Str, addRequestHandle);
    FILTER_ADD(Int, addEppCodeResponse);
);

COMPOUND_CLASS(Filter, FilterFilter, Compound,
    FILTER_ADD(Id, addUserId);
    FILTER_ADD(Id, addGroupId);
    FILTER_ADD(Int, addType);
);

COMPOUND_CLASS(PublicRequest, PublicRequest, Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Int, addType);
    FILTER_ADD(Int, addStatus);
    FILTER_ADD(DateTime, addCreateTime);
    FILTER_ADD(DateTime, addResolveTime);
    FILTER_ADD(Str, addReason);
    FILTER_ADD(Str, addEmailToAnswer);
    FILTER_ADD(Obj, addObject);
    FILTER_ADD(Action, addEppAction);
);

COMPOUND_CLASS(File, File, Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Str, addName);
    FILTER_ADD(Str, addPath);
    FILTER_ADD(Str, addMimeType);
    FILTER_ADD(DateTime, addCreateTime);
    FILTER_ADD(Int, addSize);
    FILTER_ADD(Int, addType);
);

COMPOUND_CLASS(Invoice, Invoice, Compound,
    FILTER_ADD(Id, addZoneId);
    FILTER_ADD(Int, addType);
    FILTER_ADD(Str, addNumber);
    FILTER_ADD(DateTime, addCreateTime);
    FILTER_ADD(Date, addTaxDate);
    FILTER_ADD(Registrar, addRegistrar);
    FILTER_ADD(Obj, addObject);
    FILTER_ADD(File, addFile);
);

COMPOUND_CLASS(Mail, Mail, Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Int, addType);
    FILTER_ADD(Str, addHandle);
    FILTER_ADD(DateTime, addCreateTime);
    FILTER_ADD(DateTime, addModifyTime);
    FILTER_ADD(Int, addStatus);
    FILTER_ADD(Int, addAttempt);
    FILTER_ADD(Str, addMessage);
    FILTER_ADD(File, addAttachment);
);

FilterIteratorImpl::FilterIteratorImpl() :
  i(flist.end()) {
  TRACE("[CALL] FilterIteratorImpl::FilterIteratorImpl()");
}

FilterIteratorImpl::~FilterIteratorImpl() {
  TRACE("[CALL] FilterIteratorImpl::~FilterIteratorImpl()");
  clearF();
}

void FilterIteratorImpl::addF(FilterBaseImpl* f) {
  flist.push_back(f);
  reset();
}

void FilterIteratorImpl::clearF() {
  std::for_each(flist.begin(), flist.end(), boost::checked_deleter<FilterBaseImpl>());
  flist.clear();
}

void FilterIteratorImpl::reset() {
  i = flist.begin();
}

void FilterIteratorImpl::setNext() {
  if (hasNext())
    i++;
}

bool FilterIteratorImpl::hasNext() {
  return i != flist.end();
}

ccReg::Filters::Base_ptr FilterIteratorImpl::getFilter() {
  TRACE("[CALL] FilterIteratorImpl::getFilter()");
  if (!hasNext()) {
    throw ccReg::Filters::Iterator::INVALID();
  }
  else {
    return (*i)->_this();
  }
  // return !hasNext() ? ccReg::Filters::Base::_nil() : (*i)->_this();
}

#define ITERATOR_ADD_E_METHOD_IMPL(ct, dt) \
  ccReg::Filters::ct##_ptr FilterIteratorImpl::addE(Database::Filters::dt *f) { \
    Filter##ct##Impl *newf = new Filter##ct##Impl(f); \
    addF(newf); \
    return newf->_this(); \
  }

ITERATOR_ADD_E_METHOD_IMPL(Str,Value<std::string>);
ITERATOR_ADD_E_METHOD_IMPL(Int,Value<unsigned>);
ITERATOR_ADD_E_METHOD_IMPL(Int,Value<int>);
ITERATOR_ADD_E_METHOD_IMPL(IntInterval,Interval<int>);
ITERATOR_ADD_E_METHOD_IMPL(Id,Value<Database::ID>);
ITERATOR_ADD_E_METHOD_IMPL(Action,EppAction);
ITERATOR_ADD_E_METHOD_IMPL(Date,Interval<Database::DateInterval>);
ITERATOR_ADD_E_METHOD_IMPL(DateTime,Interval<Database::DateTimeInterval>);
ITERATOR_ADD_E_METHOD_IMPL(Obj,Object);
ITERATOR_ADD_E_METHOD_IMPL(Registrar,Registrar);
ITERATOR_ADD_E_METHOD_IMPL(Filter,FilterFilter);
ITERATOR_ADD_E_METHOD_IMPL(Contact,Contact);
ITERATOR_ADD_E_METHOD_IMPL(Domain,Domain);
ITERATOR_ADD_E_METHOD_IMPL(NSSet,NSSet);
ITERATOR_ADD_E_METHOD_IMPL(KeySet,KeySet);
ITERATOR_ADD_E_METHOD_IMPL(PublicRequest,PublicRequest);
ITERATOR_ADD_E_METHOD_IMPL(File,File);
ITERATOR_ADD_E_METHOD_IMPL(Invoice,Invoice);
ITERATOR_ADD_E_METHOD_IMPL(Mail,Mail);
ITERATOR_ADD_E_METHOD_IMPL(ObjectState,ObjectState);

#define ITERATOR_ADD_FILTER_METHOD_IMPL(ct,dt) \
  { Database::Filters::dt *rf = dynamic_cast<Database::Filters::dt *>(f); \
    if (rf) { addF(new Filter##ct##Impl(rf)); return; } }

void FilterIteratorImpl::addFilter(Database::Filters::Filter *f) {
  if (f->getName().empty()) return;
  ITERATOR_ADD_FILTER_METHOD_IMPL(Str,Value<std::string>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Int,Value<unsigned>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Int,Value<int>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Id,Value<Database::ID>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Action,EppAction);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Date,Interval<Database::DateInterval>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(DateTime,Interval<Database::DateTimeInterval>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Obj,Object);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Registrar,Registrar);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Filter,FilterFilter);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Contact,Contact);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Domain,Domain);
  ITERATOR_ADD_FILTER_METHOD_IMPL(NSSet,NSSet);
  ITERATOR_ADD_FILTER_METHOD_IMPL(KeySet,KeySet);
  ITERATOR_ADD_FILTER_METHOD_IMPL(PublicRequest,PublicRequest);
  ITERATOR_ADD_FILTER_METHOD_IMPL(File,File);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Invoice,Invoice);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Mail,Mail);
  ITERATOR_ADD_FILTER_METHOD_IMPL(ObjectState,ObjectState);
  ITERATOR_ADD_FILTER_METHOD_IMPL(IntInterval,Interval<int>);
}
