#ifndef FILTER_IMPL_H_
#define FILTER_IMPL_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "corba/mailer_manager.h"
#include "register/register.h"
#include "old_utils/dbsql.h"
#include "old_utils/conf.h"
#include "db/dbs.h"
#include "model/model_filters.h"

class FilterBaseImpl;
class FilterIteratorImpl : virtual public POA_ccReg::Filters::Iterator
{
public:
  FilterIteratorImpl();
  ~FilterIteratorImpl();
  
  ccReg::Filters::Base_ptr getFilter();
  
  ccReg::Filters::Int_ptr addE(DBase::Filters::Value<unsigned>* f);
  ccReg::Filters::Int_ptr addE(DBase::Filters::Value<int>* f);
  ccReg::Filters::Id_ptr addE(DBase::Filters::Value<DBase::ID>* f);
  ccReg::Filters::Str_ptr addE(DBase::Filters::Value<std::string>* f);
  ccReg::Filters::Date_ptr addE(DBase::Filters::Interval<DBase::DateInterval>* f);
  ccReg::Filters::DateTime_ptr addE(DBase::Filters::Interval<DBase::DateTimeInterval>* f);
  ccReg::Filters::Filter_ptr addE(DBase::Filters::FilterFilter* f);
  ccReg::Filters::Obj_ptr addE(DBase::Filters::Object* f);
  ccReg::Filters::Contact_ptr addE(DBase::Filters::Contact* f);
  ccReg::Filters::Domain_ptr addE(DBase::Filters::Domain* f);
  ccReg::Filters::NSSet_ptr addE(DBase::Filters::NSSet* f);
  ccReg::Filters::Registrar_ptr addE(DBase::Filters::Registrar* f);
  ccReg::Filters::Action_ptr addE(DBase::Filters::EppAction* f);
  ccReg::Filters::PublicRequest_ptr addE(DBase::Filters::PublicRequest* f);
  ccReg::Filters::Invoice_ptr addE(DBase::Filters::Invoice* f);
  ccReg::Filters::Mail_ptr addE(DBase::Filters::Mail* f);
  ccReg::Filters::File_ptr addE(DBase::Filters::File* f);
    
  void addFilter(DBase::Filters::Filter* f);
    
  void clearF();
  void reset();
  void setNext();
  bool hasNext();
    
protected:
  void addF(FilterBaseImpl* f);
  
private:
  std::vector<FilterBaseImpl*> flist;
  std::vector<FilterBaseImpl*>::iterator i;
};

#endif /*FILTER_IMPL_H_*/
