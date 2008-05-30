#include "pagetable_invoices.h"

ccReg_Invoices_i::ccReg_Invoices_i(
  Register::Invoicing::InvoiceList *_invl
)
  : invl(_invl)
{
}

ccReg_Invoices_i::~ccReg_Invoices_i()
{
  TRACE("[CALL] ccReg_Invoices_i::~ccReg_Invoices_i()");
}

ccReg::Table::ColumnHeaders* 
ccReg_Invoices_i::getColumnHeaders()
{
  ccReg::Table::ColumnHeaders *ch = new ccReg::Table::ColumnHeaders();
  ch->length(10);
  COLHEAD(ch,0,"id",CT_OTHER);
  COLHEAD(ch,1,"CrDate",CT_OTHER);
  COLHEAD(ch,2,"Number",CT_OTHER);
  COLHEAD(ch,3,"Registrar",CT_REGISTRAR_HANDLE);
  COLHEAD(ch,4,"Total",CT_OTHER);
  COLHEAD(ch,5,"Credit",CT_OTHER);
  COLHEAD(ch,6,"Type",CT_OTHER);
  COLHEAD(ch,7,"Zone",CT_OTHER);
  COLHEAD(ch,8,"PDF",CT_FILE_ID);
  COLHEAD(ch,9,"XML",CT_FILE_ID);
  return ch;
}

ccReg::TableRow* 
ccReg_Invoices_i::getRow(CORBA::Short row)
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Invoicing::Invoice *inv = invl->get(row);
  if (!inv) throw ccReg::Table::INVALID_ROW();
  ccReg::TableRow *tr = new ccReg::TableRow;
  tr->length(10);
  std::stringstream buf;
  buf << inv->getId();
  (*tr)[0] = DUPSTRFUN(buf.str);
  (*tr)[1] = DUPSTRDATE(inv->getCrTime);
  buf.str("");
  buf << inv->getNumber();
  (*tr)[2] = DUPSTRFUN(buf.str);
  (*tr)[3] = DUPSTRFUN(inv->getClient()->getHandle);
  (*tr)[4] = DUPSTRC(formatMoney(inv->getPrice()));
  buf.str("");
  if (inv->getType() == Register::Invoicing::IT_DEPOSIT)
    buf << formatMoney(inv->getCredit());
  (*tr)[5] = DUPSTRFUN(buf.str);;
  (*tr)[6] = DUPSTR(
    inv->getType() == Register::Invoicing::IT_DEPOSIT ? "DEPOSIT" : "ACCOUNT"
  );
  (*tr)[7] = DUPSTRC(inv->getZoneName());
  buf.str("");
  buf << inv->getFilePDF();
  (*tr)[8] = DUPSTRFUN(buf.str);
  buf.str("");
  buf << inv->getFileXML();
  (*tr)[9] = DUPSTRFUN(buf.str);
  return tr;
}

void 
ccReg_Invoices_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir)
{
}

ccReg::TID 
ccReg_Invoices_i::getRowId(CORBA::Short row) 
  throw (ccReg::Table::INVALID_ROW)
{
  const Register::Invoicing::Invoice *inv = invl->get(row);
  if (!inv) throw ccReg::Table::INVALID_ROW();
  return inv->getId();
}

char*
ccReg_Invoices_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Invoices_i::numRows()
{
  return invl->getCount();
}

CORBA::Short 
ccReg_Invoices_i::numColumns()
{
  return 6;
}

void 
ccReg_Invoices_i::reload()
{
  invl->setPartialLoad(true);
  invl->reload();
}

FILTER_IMPL_L(ccReg_Invoices_i::id,idFilter,invl->setIdFilter(_v));

FILTER_IMPL_S(ccReg_Invoices_i::number,numberFilter,
              invl->setNumberFilter(_v));

FILTER_IMPL(ccReg_Invoices_i::crDate,
            ccReg::DateInterval,
            const ccReg::DateInterval&,
            crDateFilter,crDateFilter,
            invl->setCrDateFilter(setPeriod(_v)));

FILTER_IMPL_L(ccReg_Invoices_i::registrarId,
              registrarIdFilter,invl->setRegistrarFilter(_v));
            
FILTER_IMPL_S(ccReg_Invoices_i::registrarHandle,registrarHandleFilter,
              invl->setRegistrarHandleFilter(_v));

FILTER_IMPL_L(ccReg_Invoices_i::zone,
              zoneFilter,invl->setZoneFilter(_v));

FILTER_IMPL(ccReg_Invoices_i::type,
            ccReg::Invoicing::InvoiceType,
            ccReg::Invoicing::InvoiceType,
            typeFilter,typeFilter,
            invl->setTypeFilter(
             _v == ccReg::Invoicing::IT_NONE ? 
               0 : 
             _v == ccReg::Invoicing::IT_ADVANCE ? 
               1 : 2 
            ));

FILTER_IMPL_S(ccReg_Invoices_i::varSymbol,varSymbolFilter,
              invl->setVarSymbolFilter(_v));

FILTER_IMPL(ccReg_Invoices_i::taxDate,
            ccReg::DateInterval,
            const ccReg::DateInterval&,
            taxDateFilter,taxDateFilter,
            invl->setTaxDateFilter(setPeriod(_v)));
      
FILTER_IMPL_S(ccReg_Invoices_i::objectName,objectNameFilter,
              invl->setObjectNameFilter(_v));
      
FILTER_IMPL_L(ccReg_Invoices_i::objectId,
              objectIdFilter,invl->setObjectIdFilter(_v));

FILTER_IMPL_S(ccReg_Invoices_i::advanceNumber,advanceNumberFilter,
              invl->setAdvanceNumberFilter(_v));

// ccReg::Filter_ptr
// ccReg_Invoices_i::aFilter()
// {
//   return _this();
// }

void
ccReg_Invoices_i::clear()
{
  idFilter = 0;
  numberFilter = "";
  invl->clearFilter();
}

CORBA::ULongLong 
ccReg_Invoices_i::resultSize()
{
  return 12345;
}

void
ccReg_Invoices_i::loadFilter(ccReg::TID _id) {
}

void
ccReg_Invoices_i::saveFilter(const char* _name) {
}
