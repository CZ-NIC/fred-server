#include "pagetable_publicrequests.h"

ccReg_PublicRequests_i::ccReg_PublicRequests_i(Register::PublicRequest::List *_list) :
  request_list_(_list) {
}

ccReg_PublicRequests_i::~ccReg_PublicRequests_i() {
  TRACE("[CALL] ccReg_PublicRequests_i::~ccReg_PublicRequests_i()");
}

ccReg::Filters::Compound_ptr ccReg_PublicRequests_i::add() {
  TRACE("[CALL] ccReg_PublicRequests_i::add()");
  it.clearF();
  DBase::Filters::PublicRequest *f = new DBase::Filters::PublicRequestImpl();
  uf.addFilter(f);
  return it.addE(f);
}

void ccReg_PublicRequests_i::reload() {
  TRACE("[CALL] ccReg_PublicRequests_i::reload()");
  request_list_->reload(uf);
}

ccReg::Table::ColumnHeaders* ccReg_PublicRequests_i::getColumnHeaders() {
  TRACE("[CALL] ccReg_PublicRequests_i::getColumnHeaders()");
  ccReg::Table::ColumnHeaders *columns = new ccReg::Table::ColumnHeaders();
  columns->length(4);
  COLHEAD(columns, 0, "Create Time", CT_OTHER);
  COLHEAD(columns, 1, "Resolve Time", CT_OTHER);
  COLHEAD(columns, 2, "Type", CT_OTHER);
  COLHEAD(columns, 3, "Status", CT_OTHER);
  return columns;
}

ccReg::TableRow* ccReg_PublicRequests_i::getRow(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::PublicRequest::PublicRequest *request = request_list_->get(_row);
  if (!request)
    throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *row = new ccReg::TableRow;
  row->length(4);
  (*row)[0] = DUPSTRDATE(request->getCreateTime);
  (*row)[1] = DUPSTRDATE(request->getResolveTime);
  (*row)[2] = DUPSTRC(Register::PublicRequest::Type2Str(request->getType()));
  (*row)[3] = DUPSTRC(Register::PublicRequest::Status2Str(request->getStatus()));
  return row;
}

void ccReg_PublicRequests_i::sortByColumn(CORBA::Short _column, CORBA::Boolean _dir) {
  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::sortByColumn(%1%, %2%)") % _column % _dir);
  switch (_column) {
    case 0:
      request_list_->sort(Register::PublicRequest::MT_CRDATE, _dir);
      sorted_by_ = 0;
      break;
    case 1:
      request_list_->sort(Register::PublicRequest::MT_RDATE, _dir);
      sorted_by_ = 1;
      break;
    case 2:
      request_list_->sort(Register::PublicRequest::MT_TYPE, _dir);
      sorted_by_ = 2;
      break;
    case 3:
      request_list_->sort(Register::PublicRequest::MT_STATUS, _dir);
      sorted_by_ = 3;
      break;
  }
}

ccReg::TID ccReg_PublicRequests_i::getRowId(CORBA::Short _row)
    throw (ccReg::Table::INVALID_ROW) {
  const Register::PublicRequest::PublicRequest *request = request_list_->get(_row);
  if (!request)
    throw ccReg::Table::INVALID_ROW();
  return request->getId();
}

char* ccReg_PublicRequests_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_PublicRequests_i::numRows() {
  return request_list_->getCount();
}

CORBA::Short ccReg_PublicRequests_i::numColumns() {
  return 4;
}

void ccReg_PublicRequests_i::clear() {
  TRACE("[CALL] ccReg_PublicRequests_i::clear()");
  ccReg_PageTable_i::clear();
  request_list_->clear();
}

CORBA::ULongLong ccReg_PublicRequests_i::resultSize() {
  TRACE("ccReg_PublicRequests_i::resultSize()");
  return request_list_->getRealCount(uf);
}

void ccReg_PublicRequests_i::loadFilter(ccReg::TID _id) {
  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  DBase::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    DBase::Filters::PublicRequest *tmp = dynamic_cast<DBase::Filters::PublicRequest* >(*uit);
    it.addE(tmp);
    TRACE(boost::format("[IN] ccReg_PublicRequests_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
  }
}

void ccReg_PublicRequests_i::saveFilter(const char* _name) {
  TRACE(boost::format("[CALL] ccReg_PublicRequests_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create(dbm));
  tmp_filter_manager->save(Register::Filter::FT_PUBLICREQUEST, _name, uf);
}

Register::PublicRequest::PublicRequest* ccReg_PublicRequests_i::findId(ccReg::TID _id) {
  try {
    Register::PublicRequest::PublicRequest *request = dynamic_cast<Register::PublicRequest::PublicRequest* >(request_list_->findId(_id));
    if (request) {
      return request;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}
