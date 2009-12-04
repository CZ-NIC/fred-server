#include "pagetable_registrars.h"

ccReg_Registrars_i::ccReg_Registrars_i(Register::Registrar::RegistrarList *_rl
		, Register::Zone::ZoneList * _zl
		)
  : rl(_rl)
  , zl(_zl)
{

}

ccReg_Registrars_i::~ccReg_Registrars_i() {
  TRACE("[CALL] ccReg_Registrars_i::~ccReg_Registrars_i()");
}

void
ccReg_Registrars_i::reload() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] void ccReg_Registrars_i::reload()");
  rl->reload(uf);
  TRACE("[CALL] void ccReg_Registrars_i::reload() end");
}

ccReg::Filters::Compound_ptr 
ccReg_Registrars_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Registrars_i::add()");
  Database::Filters::Registrar *f = new Database::Filters::RegistrarImpl(true);
  uf.addFilter(f);
  return it.addE(f); 
}

Registry::Table::ColumnHeaders*
ccReg_Registrars_i::getColumnHeaders()
{//all columns are in svn rev 9616
  Logging::Context ctx(base_context_);

  unsigned zone_count = zl->getSize();
  TRACE(boost::format("[CALL] ccReg_Registrars_i::getColumnHeaders(), zone_count: %1%") % zone_count);
  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(static_cols + zone_count);

  COLHEAD(ch,0,"Handle",CT_OID);
  COLHEAD(ch,1,"Name",CT_OTHER);
  COLHEAD(ch,2,"Email",CT_OTHER);

  for (int i = 0 ; i < zone_count ; i++)
  {
      std::string zonefqdn
          = (dynamic_cast<Register::Zone::Zone*>(zl->get(i)))->getFqdn();
      COLHEAD(ch,static_cols+i,(zonefqdn + " credit").c_str(),CT_OTHER);
  }//for zone_count
  return ch;
}

Registry::TableRow* 
ccReg_Registrars_i::getRow(CORBA::UShort row)
  throw (ccReg::Table::INVALID_ROW)
{
  Logging::Context ctx(base_context_);
  TRACE(boost::format("[CALL] ccReg_Registrars_i::getRow(%1%)") % row);

  const Register::Registrar::Registrar *r = rl->get(row);
  if (!r) throw ccReg::Table::INVALID_ROW();
  Registry::TableRow *tr = new Registry::TableRow;

  unsigned zone_count = zl->getSize();

  tr->length(static_cols + zone_count);

  MAKE_OID(oid_handle, r->getId(), C_STR(r->getHandle()), FT_REGISTRAR)

  (*tr)[0] <<= oid_handle;
  (*tr)[1] <<= C_STR(r->getName());
  (*tr)[2] <<= C_STR(r->getEmail());
  for (int i = 0 ; i < zone_count ; i++)
  {
      unsigned long long zoneid
          = (dynamic_cast<Register::Zone::Zone*>(zl->get(i)))->getId();

      if(r->isInZone(zoneid))
          (*tr)[static_cols+i] <<= C_STR(r->getCredit(zoneid));
      else
          (*tr)[static_cols+i] <<= C_STR("");//if currently not in zone
  }//for zone_count


  return tr;
}

void 
ccReg_Registrars_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Registrars_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);

  switch (column) {
    case 0:
      rl->sort(Register::Registrar::MT_HANDLE, dir);
      break;
    case 1:
      rl->sort(Register::Registrar::MT_NAME, dir);
      break;
    case 2:
      rl->sort(Register::Registrar::MT_MAIL, dir);
      break;
  }
  if((column > (static_cols-1)) && (column < zl->size()+static_cols))
  {
      rl->sort(Register::Registrar::MT_ZONE, dir
              , (dynamic_cast<Register::Zone::Zone*>(zl->get(column-static_cols)))->getId());
  }//if zone column

}

ccReg::TID 
ccReg_Registrars_i::getRowId(CORBA::UShort row) 
  throw (ccReg::Table::INVALID_ROW)
{
  Logging::Context ctx(base_context_);

  const Register::Registrar::Registrar *r = rl->get(row);
  if (!r) throw ccReg::Table::INVALID_ROW();
  return r->getId();  
}

char* 
ccReg_Registrars_i::outputCSV()
{
  return CORBA::string_dup("1,1,1");
}

CORBA::Short 
ccReg_Registrars_i::numRows()
{
  Logging::Context ctx(base_context_);

  return rl->size();
}

CORBA::Short 
ccReg_Registrars_i::numColumns()
{
  Logging::Context ctx(base_context_);

    unsigned zone_count = zl->getSize();
    TRACE(boost::format("[CALL] ccReg_Registrars_i::numColumns(), zone_count: %1%") % zone_count);
  return static_cols + zone_count;
}

void
ccReg_Registrars_i::clear()
{
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Registrars_i::clear()");
  rl->clearFilter();
  
  ccReg_PageTable_i::clear();
  rl->clear();
}

CORBA::ULongLong 
ccReg_Registrars_i::resultSize()
{
  Logging::Context ctx(base_context_);

  TRACE("ccReg_Registrars_i::resultSize()");
  return rl->getRealCount(uf);
}

void
ccReg_Registrars_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Registrars_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Registrar *tmp = dynamic_cast<Database::Filters::Registrar* >(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Registrars_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void
ccReg_Registrars_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Registrars_i::saveFilter('%1%')") % _name);

  std::auto_ptr<Register::Filter::Manager>
      tmp_filter_manager(Register::Filter::Manager::create());
  tmp_filter_manager->save(Register::Filter::FT_REGISTRAR, _name, uf);
}

Register::Registrar::Registrar* ccReg_Registrars_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    Register::Registrar::Registrar *registrar = dynamic_cast<Register::Registrar::Registrar* >(rl->findId(_id));
    if (registrar) {
      return registrar;
    }
    return 0;
  }
  catch (Register::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Registrars_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return rl->isLimited(); 
}

