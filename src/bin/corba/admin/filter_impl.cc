#include <math.h>
#include <memory>
#include <iomanip>
#include "src/bin/corba/Filters.hh"
#include <algorithm>
#include <boost/utility.hpp>

#include "src/bin/corba/admin/common.hh"
#include "src/bin/corba/admin/filter_impl.hh"
#include "src/deprecated/util/log.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/notify.hh"
#include "src/deprecated/libfred/filter.hh"
#include "src/bin/corba/mailer_manager.hh"

#include "src/util/types/null.hh"
#include "util/log/logger.hh"

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

class FilterBoolImpl : virtual public POA_ccReg::Filters::Bool,
  public FilterSimpleImpl {
  Database::Filters::Value<bool>* get() {
    return dynamic_cast<Database::Filters::Value<bool>*>(f);
  }

public:
  FilterBoolImpl(Database::Filters::Value<bool>* f) :
    FilterSimpleImpl(f) {
  }
  CORBA::Boolean value() {
    return get()->getValue().getValue();
  }
  void value(CORBA::Boolean v) {
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

class FilterServiceTypeImpl : virtual public POA_ccReg::Filters::ServiceType, public FilterSimpleImpl {
  Database::Filters::ServiceType* get() {
    return dynamic_cast<Database::Filters::ServiceType*>(f);
  }

public:
  FilterServiceTypeImpl(Database::Filters::ServiceType* f) :
    FilterSimpleImpl(f) {
  }

  ccReg::RequestServiceType value() {
    long int val;
    val = (long int)get()->getValue().getValue();
    return ccReg::RequestServiceType(val);
  }

  void value(ccReg::RequestServiceType v) {
    get()->setValue(Database::Null<long int>(v));
  }
};

class FilterRequestTypeImpl : virtual public POA_ccReg::Filters::RequestType, public FilterSimpleImpl {
  Database::Filters::RequestType* get() {
    return dynamic_cast<Database::Filters::RequestType*>(f);
  }

public:
  FilterRequestTypeImpl(Database::Filters::RequestType* f) :
    FilterSimpleImpl(f) {
  }

  ccReg::RequestType value() {
    long int val;
    val = (long int)get()->getValue().getValue();
    return ccReg::RequestType(val);
  }

  void value(ccReg::RequestType v) {
    get()->setValue(Database::Null<long int>(v));
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
    std::unique_ptr<Database::Filters::Iterator> cit(f->createIterator());
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
    FILTER_ADD(Str, addIco);
    FILTER_ADD(Str, addDic);
    FILTER_ADD(Str, addVarSymbol);
    FILTER_ADD(Bool, addVat);
    FILTER_ADD(Str, addHandle);
    FILTER_ADD(Str, addName);
    FILTER_ADD(Str, addOrganization);
    FILTER_ADD(Str, addStreet);
    FILTER_ADD(Str, addCity);
    FILTER_ADD(Str, addStateOrProvince);
    FILTER_ADD(Str, addPostalCode);
    FILTER_ADD(Str, addCountryCode);
    FILTER_ADD(Str, addTelephone);
    FILTER_ADD(Str, addFax);
    FILTER_ADD(Str, addEmail);
    FILTER_ADD(Str, addUrl);
    FILTER_ADD(Str, addZoneFqdn);
    FILTER_ADD(Id, addGroupId);
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
    FILTER_ADD(Str, addPhoneNumber);
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
    FILTER_ADD(Bool, addPublish)
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
    FILTER_ADD(Registrar, addRegistrar);
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
    FILTER_ADD(DateTime, addCreateTime);
    FILTER_ADD(DateTime, addModifyTime);
    FILTER_ADD(Int, addStatus);
    FILTER_ADD(Int, addAttempt);
    FILTER_ADD(Str, addMessage);
    FILTER_ADD(File, addAttachment);
);

COMPOUND_CLASS(ResultCode, ResultCode, Compound,
    FILTER_ADD(Id, addServiceId);
    FILTER_ADD(Int, addResultCode);
    FILTER_ADD(Str, addName);
);

COMPOUND_CLASS(RequestObjectRef, RequestObjectRef, Compound, 
    FILTER_ADD(Str, addObjectType);
    FILTER_ADD(Id, addObjectId);
);

COMPOUND_CLASS(RequestPropertyValue, RequestPropertyValue, Compound,
    FILTER_ADD(Str, addName);
    FILTER_ADD(Str, addValue);
    FILTER_ADD(Bool, addOutputFlag);
);

COMPOUND_CLASS(Request, Request, Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(DateTime, addTimeBegin);
    FILTER_ADD(DateTime, addTimeEnd);
    FILTER_ADD(Str, addSourceIp);
    FILTER_ADD(Str, addUserName);
    FILTER_ADD(Id, addUserId);
    FILTER_ADD(Bool, addIsMonitoring);
    FILTER_ADD(ServiceType, addServiceType);
    FILTER_ADD(RequestType, addRequestType);
    FILTER_ADD(RequestData, addRequestData);
    FILTER_ADD(RequestPropertyValue, addRequestPropertyValue);
    FILTER_ADD(ResultCode, addResultCode);
    FILTER_ADD(RequestObjectRef, addRequestObjectRef);
);

COMPOUND_CLASS(RequestData, RequestData, Compound,
    FILTER_ADD(Str, addContent);
    FILTER_ADD(Bool, addResponseFlag);
);

COMPOUND_CLASS(Session, Session, Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Str, addUserName);
    FILTER_ADD(Id, addUserId);
    FILTER_ADD(DateTime, addLoginDate);
    FILTER_ADD(DateTime, addLogoutDate);
);

COMPOUND_CLASS(ZoneNs, ZoneNs , Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Id, addZoneId);
    FILTER_ADD(Str, addFqdn);
    FILTER_ADD(Str, addAddrs);
);

COMPOUND_CLASS(Zone, Zone , Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(Str, addFqdn);
    FILTER_ADD(Int, addExPeriodMin);
    FILTER_ADD(Int, addExPeriodMax);
    FILTER_ADD(Int, addValPeriod);
    FILTER_ADD(Int, addDotsMax);
    FILTER_ADD(Bool, addEnumZone);
    FILTER_ADD(ZoneNs, addZoneNs);
);

COMPOUND_CLASS(ZoneSoa, ZoneSoa , Zone,
    FILTER_ADD(Id, addZoneId);
    FILTER_ADD(Int, addTtl);
    FILTER_ADD(Str, addHostmaster);
    FILTER_ADD(Int, addSerial);
    FILTER_ADD(Int, addRefresh);
    FILTER_ADD(Int, addUpdateRetr);
    FILTER_ADD(Int, addExpiry);
    FILTER_ADD(Int, addMinimum);
    FILTER_ADD(Str, addNsFqdn);
);

COMPOUND_CLASS(Message, Message , Compound,
    FILTER_ADD(Id, addId);
    FILTER_ADD(DateTime, addCrDate);
    FILTER_ADD(DateTime, addModDate);
    FILTER_ADD(Int, addAttempt);
    FILTER_ADD(Int, addStatus);
    FILTER_ADD(Int, addCommType);
    FILTER_ADD(Int, addMessageType);
    FILTER_ADD(Str, addSmsPhoneNumber);
    FILTER_ADD(Str, addLetterAddrName);
    FILTER_ADD(Contact, addMessageContact);
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
ITERATOR_ADD_E_METHOD_IMPL(Bool,Value<bool>);
ITERATOR_ADD_E_METHOD_IMPL(Id,Value<Database::ID>);
ITERATOR_ADD_E_METHOD_IMPL(Date,Interval<Database::DateInterval>);
ITERATOR_ADD_E_METHOD_IMPL(DateTime,Interval<Database::DateTimeInterval>);
ITERATOR_ADD_E_METHOD_IMPL(ServiceType,ServiceType);
ITERATOR_ADD_E_METHOD_IMPL(RequestType,RequestType);
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
ITERATOR_ADD_E_METHOD_IMPL(ResultCode,ResultCode);
ITERATOR_ADD_E_METHOD_IMPL(RequestObjectRef,RequestObjectRef);
ITERATOR_ADD_E_METHOD_IMPL(RequestPropertyValue,RequestPropertyValue);
ITERATOR_ADD_E_METHOD_IMPL(RequestData,RequestData);
ITERATOR_ADD_E_METHOD_IMPL(Request,Request);
ITERATOR_ADD_E_METHOD_IMPL(Session,Session);

ITERATOR_ADD_E_METHOD_IMPL(ZoneSoa, ZoneSoa);
ITERATOR_ADD_E_METHOD_IMPL(ZoneNs, ZoneNs);
ITERATOR_ADD_E_METHOD_IMPL(Zone, Zone);

ITERATOR_ADD_E_METHOD_IMPL(Message, Message);


#define ITERATOR_ADD_FILTER_METHOD_IMPL(ct,dt) \
  { Database::Filters::dt *rf = dynamic_cast<Database::Filters::dt *>(f); \
    if (rf) { addF(new Filter##ct##Impl(rf)); return; } }

void FilterIteratorImpl::addFilter(Database::Filters::Filter *f) {
  if (f->getName().empty()) return;
  ITERATOR_ADD_FILTER_METHOD_IMPL(Str,Value<std::string>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Int,Value<unsigned>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Int,Value<int>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Id,Value<Database::ID>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Bool,Value<bool>);
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
  ITERATOR_ADD_FILTER_METHOD_IMPL(ResultCode,ResultCode);
  ITERATOR_ADD_FILTER_METHOD_IMPL(RequestObjectRef,RequestObjectRef);
  ITERATOR_ADD_FILTER_METHOD_IMPL(RequestPropertyValue,RequestPropertyValue);
  ITERATOR_ADD_FILTER_METHOD_IMPL(RequestData,RequestData);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Request,Request);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Session,Session);
  ITERATOR_ADD_FILTER_METHOD_IMPL(IntInterval,Interval<int>);
  ITERATOR_ADD_FILTER_METHOD_IMPL(ServiceType,ServiceType);
  ITERATOR_ADD_FILTER_METHOD_IMPL(RequestType,RequestType);

  ITERATOR_ADD_FILTER_METHOD_IMPL(ZoneSoa, ZoneSoa);
  ITERATOR_ADD_FILTER_METHOD_IMPL(ZoneNs, ZoneNs);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Zone, Zone);
  ITERATOR_ADD_FILTER_METHOD_IMPL(Message, Message);

}

