#include "pagetable_mails.h"

ccReg_Mails_i::ccReg_Mails_i(NameService *ns) : 
  mm(ns), idFilter(0), statusFilter(-1)
{
}

ccReg_Mails_i::~ccReg_Mails_i()
{
  TRACE("[CALL] ccReg_Mails_i::~ccReg_Mails_i()");
}

ccReg::Table::ColumnHeaders* 
ccReg_Mails_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(4);
  COLHEAD(ch,0,"Id",CT_OTHER);
  COLHEAD(ch,1,"CrDate",CT_OTHER);
  COLHEAD(ch,2,"Type",CT_OTHER);
  COLHEAD(ch,3,"Status",CT_OTHER);
  return ch;
}

ccReg::TableRow* 
ccReg_Mails_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  if ((unsigned)row >= mm.getMailList().size()) 
    throw ccReg::Table::INVALID_ROW();
  MailerManager::Detail& md = mm.getMailList()[row];
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(4);
  std::stringstream buf;
  buf << md.id;
  (*tr)[0] = DUPSTRC(buf.str());
  (*tr)[1] = DUPSTRC(md.createTime);
  (*tr)[2] = DUPSTRC(md.typeDesc);
  buf.str("");
  buf << md.status;
  (*tr)[3] = DUPSTRC(buf.str());
  return tr;
}

void 
ccReg_Mails_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

ccReg::TID 
ccReg_Mails_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  if ((unsigned)row >= mm.getMailList().size()) 
    throw ccReg::Table::INVALID_ROW();
  MailerManager::Detail& md = mm.getMailList()[row];
  return md.id;
}

char*
ccReg_Mails_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Mails_i::numRows()
{
  return mm.getMailList().size();
}

CORBA::Short 
ccReg_Mails_i::numColumns()
{
  return 4;
}

void 
ccReg_Mails_i::reload()
{
  MailerManager::Filter mf;
  mf.id = idFilter;
  mf.status = statusFilter;
  mf.content = fulltextFilter;
  mf.handle = handleFilter;
  mf.attachment = attachmentFilter;
  mf.type = typeFilter;
  mf.crTime = setPeriod(createTimeFilter);
  try {
    mm.reload(mf);
  } catch (...) {}
}

FILTER_IMPL_L(ccReg_Mails_i::id,idFilter,);

FILTER_IMPL(ccReg_Mails_i::status,CORBA::Long,CORBA::Long,
            statusFilter,statusFilter,);

FILTER_IMPL(ccReg_Mails_i::type,CORBA::UShort,CORBA::UShort,
            typeFilter,typeFilter,);

FILTER_IMPL_S(ccReg_Mails_i::handle,handleFilter,);

FILTER_IMPL_S(ccReg_Mails_i::fulltext,fulltextFilter,);

FILTER_IMPL_S(ccReg_Mails_i::attachment,attachmentFilter,);

FILTER_IMPL(ccReg_Mails_i::createTime,
            ccReg::DateTimeInterval,
            const ccReg::DateTimeInterval&,
            createTimeFilter,createTimeFilter,);

// ccReg::Filter_ptr
// ccReg_Mails_i::aFilter()
// {
//   return _this();
// }

void
ccReg_Mails_i::clear()
{
  idFilter = 0;
  handleFilter = "";
  typeFilter = 0;
  statusFilter = -1;
  fulltextFilter = "";
  attachmentFilter = "";
  clearPeriod(createTimeFilter);
}

CORBA::ULongLong 
ccReg_Mails_i::resultSize()
{
  return 12345;
}

void
ccReg_Mails_i::loadFilter(ccReg::TID _id) {
}

void
ccReg_Mails_i::saveFilter(const char* _name) {
}
