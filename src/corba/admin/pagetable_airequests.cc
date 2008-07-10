#include "pagetable_airequests.h"

ccReg_AIRequests_i::ccReg_AIRequests_i(
  Register::AuthInfoRequest::List *_airl
)
  : airl(_airl)
{
}

ccReg_AIRequests_i::~ccReg_AIRequests_i()
{
  TRACE("[CALL] ccReg_AIRequests_i::~ccReg_AIRequests_i");
}

ccReg::Table::ColumnHeaders* 
ccReg_AIRequests_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(5);
  COLHEAD(ch,0,"RequestId",CT_OTHER);
  COLHEAD(ch,1,"CrDate",CT_OTHER);
  COLHEAD(ch,2,"Handle",CT_OTHER);
  COLHEAD(ch,3,"Type",CT_OTHER);
  COLHEAD(ch,4,"Status",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_AIRequests_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::AuthInfoRequest::Detail *aird = airl->get(row);
  if (!aird) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(5);
  std::stringstream id;
  id << aird->getId();
  (*tr)[0] = DUPSTRFUN(id.str);
  (*tr)[1] = DUPSTRDATE(aird->getCreationTime);
  (*tr)[2] = DUPSTRFUN(aird->getObjectHandle);
  std::string type;
  switch (aird->getRequestType()) {
    case Register::AuthInfoRequest::RT_EPP : type = "EPP"; break;
    case Register::AuthInfoRequest::RT_AUTO_PIF : type = "AUTO_PIF"; break;
    case Register::AuthInfoRequest::RT_EMAIL_PIF : type = "MAIL_PIF"; break;
    case Register::AuthInfoRequest::RT_POST_PIF : type = "POST_PIF"; break;
  }
  (*tr)[3] = DUPSTRC(type);
  std::string status;
  switch (aird->getRequestStatus()) {
    case Register::AuthInfoRequest::RS_NEW : status = "NEW"; break;
    case Register::AuthInfoRequest::RS_ANSWERED : status = "CLOSED"; break;
    case Register::AuthInfoRequest::RS_INVALID : status = "INVALID"; break;
  }
  (*tr)[4] = DUPSTRC(status);
  return tr;
}

void 
ccReg_AIRequests_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

ccReg::TID 
ccReg_AIRequests_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::AuthInfoRequest::Detail *aird = airl->get(row);
  if (!aird) throw ccReg::Table::INVALID_ROW();
  return aird->getId();
}

char*
ccReg_AIRequests_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_AIRequests_i::numRows()
{
  return airl->getCount();
}

CORBA::Short 
ccReg_AIRequests_i::numColumns()
{
  return 5;
}

void 
ccReg_AIRequests_i::reload()
{
  airl->reload();
}

FILTER_IMPL_L(ccReg_AIRequests_i::id,idFilter,airl->setIdFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::handle,handleFilter,
              airl->setHandleFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::email,emailFilter,
              airl->setEmailFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::reason,reasonFilter,
              airl->setReasonFilter(_v));

FILTER_IMPL_S(ccReg_AIRequests_i::svTRID,svTRIDFilter,
              airl->setSvTRIDFilter(_v));

FILTER_IMPL(ccReg_AIRequests_i::type,
            ccReg::AuthInfoRequest::RequestType,
            ccReg::AuthInfoRequest::RequestType,
            typeFilter,typeFilter,
            airl->setRequestTypeIgnoreFilter(
              _v == ccReg::AuthInfoRequest::RT_IGNORE
            );
            airl->setRequestTypeFilter(
             _v == ccReg::AuthInfoRequest::RT_EPP ? 
               Register::AuthInfoRequest::RT_EPP : 
             _v == ccReg::AuthInfoRequest::RT_AUTO_PIF ? 
               Register::AuthInfoRequest::RT_AUTO_PIF : 
             _v == ccReg::AuthInfoRequest::RT_EMAIL_PIF ? 
               Register::AuthInfoRequest::RT_EMAIL_PIF : 
               Register::AuthInfoRequest::RT_POST_PIF 
            ));

FILTER_IMPL(ccReg_AIRequests_i::status,
            ccReg::AuthInfoRequest::RequestStatus,
            ccReg::AuthInfoRequest::RequestStatus,
            statusFilter,statusFilter,
            airl->setRequestStatusIgnoreFilter(
              _v == ccReg::AuthInfoRequest::RS_IGNORE
            );
            airl->setRequestStatusFilter(
             _v == ccReg::AuthInfoRequest::RS_NEW ? 
               Register::AuthInfoRequest::RS_NEW : 
             _v == ccReg::AuthInfoRequest::RS_ANSWERED ? 
               Register::AuthInfoRequest::RS_ANSWERED : 
               Register::AuthInfoRequest::RS_INVALID 
            ));
            
FILTER_IMPL(ccReg_AIRequests_i::crTime,
            ccReg::DateTimeInterval,
            const ccReg::DateTimeInterval&,
            crTimeFilter,crTimeFilter,
            airl->setCreationTimeFilter(setPeriod(_v)));

FILTER_IMPL(ccReg_AIRequests_i::closeTime,
            ccReg::DateTimeInterval,
            const ccReg::DateTimeInterval&,
            closeTimeFilter,closeTimeFilter,
            airl->setCloseTimeFilter(setPeriod(_v)));

// ccReg::Filter_ptr
// ccReg_AIRequests_i::aFilter()
// {
//   return _this();
// }

void
ccReg_AIRequests_i::clear()
{
  idFilter = 0;
  handleFilter = "";
  emailFilter = "";
  svTRIDFilter = "";
  reasonFilter = "";
  // TODO CLEAR OTHER 
  airl->clearFilter();
}

CORBA::ULongLong 
ccReg_AIRequests_i::resultSize()
{
  return 12345;
}

void
ccReg_AIRequests_i::loadFilter(ccReg::TID _id) {
}

void
ccReg_AIRequests_i::saveFilter(const char* _name) {
}

CORBA::Boolean ccReg_AIRequests_i::numRowsOverLimit() {
  return false; 
}
