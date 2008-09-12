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
#include "db/manager.h"
#include "model/model_filters.h"

class FilterBaseImpl;
class FilterIteratorImpl : virtual public POA_ccReg::Filters::Iterator
{
public:
  FilterIteratorImpl();
  ~FilterIteratorImpl();
  
  ccReg::Filters::Base_ptr getFilter();
  
  ccReg::Filters::Int_ptr addE(Database::Filters::Value<unsigned>* f);
  ccReg::Filters::Int_ptr addE(Database::Filters::Value<int>* f);
  ccReg::Filters::IntInterval_ptr addE(Database::Filters::Interval<int>* f);
  ccReg::Filters::Id_ptr addE(Database::Filters::Value<Database::ID>* f);
  ccReg::Filters::Str_ptr addE(Database::Filters::Value<std::string>* f);
  ccReg::Filters::Date_ptr addE(Database::Filters::Interval<Database::DateInterval>* f);
  ccReg::Filters::DateTime_ptr addE(Database::Filters::Interval<Database::DateTimeInterval>* f);
  ccReg::Filters::Filter_ptr addE(Database::Filters::FilterFilter* f);
  ccReg::Filters::Obj_ptr addE(Database::Filters::Object* f);
  ccReg::Filters::Contact_ptr addE(Database::Filters::Contact* f);
  ccReg::Filters::Domain_ptr addE(Database::Filters::Domain* f);
  ccReg::Filters::NSSet_ptr addE(Database::Filters::NSSet* f);
  ccReg::Filters::KeySet_ptr addE(Database::Filters::KeySet* f);
  ccReg::Filters::Registrar_ptr addE(Database::Filters::Registrar* f);
  ccReg::Filters::Action_ptr addE(Database::Filters::EppAction* f);
  ccReg::Filters::PublicRequest_ptr addE(Database::Filters::PublicRequest* f);
  ccReg::Filters::Invoice_ptr addE(Database::Filters::Invoice* f);
  ccReg::Filters::Mail_ptr addE(Database::Filters::Mail* f);
  ccReg::Filters::File_ptr addE(Database::Filters::File* f);
  ccReg::Filters::ObjectState_ptr addE(Database::Filters::ObjectState* f);
    
  void addFilter(Database::Filters::Filter* f);
    
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
