#ifndef FILTER_IMPL_H_
#define FILTER_IMPL_H_

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "corba/mailer_manager.h"
#include "fredlib/registry.h"
#include "old_utils/dbsql.h"
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
  ccReg::Filters::Bool_ptr addE(Database::Filters::Value<bool>* f);
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
  ccReg::Filters::PublicRequest_ptr addE(Database::Filters::PublicRequest* f);
  ccReg::Filters::Invoice_ptr addE(Database::Filters::Invoice* f);
  ccReg::Filters::Mail_ptr addE(Database::Filters::Mail* f);
  ccReg::Filters::File_ptr addE(Database::Filters::File* f);
  ccReg::Filters::ObjectState_ptr addE(Database::Filters::ObjectState* f);
  ccReg::Filters::Request_ptr addE(Database::Filters::Request* f);
  ccReg::Filters::RequestPropertyValue_ptr addE(Database::Filters::RequestPropertyValue *f);
  ccReg::Filters::RequestData_ptr addE(Database::Filters::RequestData* f);
  ccReg::Filters::Session_ptr addE(Database::Filters::Session* f);
  ccReg::Filters::ResultCode_ptr addE(Database::Filters::ResultCode* f);
  ccReg::Filters::RequestObjectRef_ptr addE(Database::Filters::RequestObjectRef* f);
   

  // ccReg::Filters::ServiceType_ptr addE(Database::Filters::Value<Database::Filters::ServiceType> *f);
  // ccReg::Filters::RequestType_ptr addE(Database::Filters::Value<Database::Filters::RequestType> *f);
  ccReg::Filters::ServiceType_ptr addE(Database::Filters::ServiceType *f);
  ccReg::Filters::RequestType_ptr addE(Database::Filters::RequestType *f);
  ccReg::Filters::StatementItem_ptr addE(Database::Filters::BankPayment *f);
  ccReg::Filters::StatementHead_ptr addE(Database::Filters::BankStatement *f);


  ccReg::Filters::ZoneSoa_ptr addE(Database::Filters::ZoneSoa *f);
  ccReg::Filters::ZoneNs_ptr addE(Database::Filters::ZoneNs *f);
  ccReg::Filters::Zone_ptr addE(Database::Filters::Zone *f);

  ccReg::Filters::Message_ptr addE(Database::Filters::Message *f);



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
