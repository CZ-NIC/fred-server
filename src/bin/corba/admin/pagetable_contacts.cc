/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/bin/corba/admin/pagetable_contacts.hh"

ccReg_Contacts_i::ccReg_Contacts_i(LibFred::Contact::List *_cl, const Settings *_ptr) : cl(_cl) {
  uf.settings(_ptr);
}

ccReg_Contacts_i::~ccReg_Contacts_i() {
  TRACE("[CALL] ccReg_Contacts_i::~ccReg_Contacts_i()");
}

ccReg::Filters::Compound_ptr ccReg_Contacts_i::add() {
  Logging::Context ctx(base_context_);

  TRACE("[CALL] ccReg_Contacts_i::add()");
  Database::Filters::Contact *f = new Database::Filters::ContactHistoryImpl();
  uf.addFilter(f);
  return it.addE(f);
}

Registry::Table::ColumnHeaders* ccReg_Contacts_i::getColumnHeaders() {
  Logging::Context ctx(base_context_);

  Registry::Table::ColumnHeaders *ch = new Registry::Table::ColumnHeaders();
  ch->length(6);
  COLHEAD(ch, 0, "Handle", CT_OID);
  COLHEAD(ch, 1, "Name", CT_OTHER);
  COLHEAD(ch, 2, "Organization", CT_OTHER);
  COLHEAD(ch, 3, "Create date", CT_OTHER);
  COLHEAD(ch, 4, "Delete date", CT_OTHER);
  COLHEAD(ch, 5, "Registrar", CT_OID);
  return ch;
}

Registry::TableRow* ccReg_Contacts_i::getRow(CORBA::UShort row)
{
  Logging::Context ctx(base_context_);

  const LibFred::Contact::Contact *c = cl->getContact(row);
  if (!c) throw Registry::Table::INVALID_ROW();
  Registry::TableRow *tr = new Registry::TableRow;
  tr->length(6);

  MAKE_OID(oid_handle, c->getId(), C_STR(c->getHandle()), FT_CONTACT)
  MAKE_OID(oid_registrar, c->getRegistrarId(), C_STR(c->getRegistrarHandle()), FT_REGISTRAR)

  (*tr)[0] <<= oid_handle;
  (*tr)[1] <<= C_STR(c->getName());
  (*tr)[2] <<= C_STR(c->getOrganization());
  (*tr)[3] <<= C_STR(c->getCreateDate());
  (*tr)[4] <<= C_STR(c->getDeleteDate());
  (*tr)[5] <<= oid_registrar;
  return tr;
}

void ccReg_Contacts_i::sortByColumn(CORBA::Short column, CORBA::Boolean dir) {
  Logging::Context ctx(base_context_);

  TRACE(boost::format("[CALL] ccReg_Contacts_i::sortByColumn(%1%, %2%)") % column % dir);
  /* save sort state */
  ccReg_PageTable_i::sortByColumn(column, dir);
  
  switch (column) {
    case 0:
      cl->sort(LibFred::Contact::MT_HANDLE, dir);
      break;
    case 1:
      cl->sort(LibFred::Contact::MT_NAME, dir);
      break;
    case 2:
      cl->sort(LibFred::Contact::MT_ORGANIZATION, dir);
      break;
    case 3:
      cl->sort(LibFred::Contact::MT_CRDATE, dir);
      break;
    case 4:
      cl->sort(LibFred::Contact::MT_ERDATE, dir);
      break;
    case 5:
      cl->sort(LibFred::Contact::MT_REGISTRAR_HANDLE, dir);
      break;
  }
}

ccReg::TID ccReg_Contacts_i::getRowId(CORBA::UShort row)
{
  Logging::Context ctx(base_context_);

  const LibFred::Contact::Contact *c = cl->getContact(row);
  if (!c) throw Registry::Table::INVALID_ROW();
  return c->getId();
}

char* ccReg_Contacts_i::outputCSV() {
  return CORBA::string_dup("1,1,1");
}

CORBA::Short ccReg_Contacts_i::numRows() {
  Logging::Context ctx(base_context_);

  return cl->getCount();
}

CORBA::Short ccReg_Contacts_i::numColumns() {
  Logging::Context ctx(base_context_);

  return 6;
}

void ccReg_Contacts_i::reload_worker() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

//  cl->makeRealCount();
  cl->setTimeout(query_timeout);
  cl->setLimit(limit_);
  cl->reload(uf);
  cl->deleteDuplicatesId();
}

void ccReg_Contacts_i::clear() {
  Logging::Context ctx(base_context_);

  cl->clearFilter();
  
  ccReg_PageTable_i::clear();
  cl->clear();
}

CORBA::ULongLong ccReg_Contacts_i::resultSize() {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE("[CALL] ccReg_Contacts_i::resultSize()");
  return cl->getRealCount(uf);
}

void ccReg_Contacts_i::loadFilter(ccReg::TID _id) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Contacts_i::loadFilter(%1%)") % _id);
  ccReg_PageTable_i::loadFilter(_id);

  Database::Filters::Union::iterator uit = uf.begin();
  for (; uit != uf.end(); ++uit) {
    Database::Filters::Contact *tmp = dynamic_cast<Database::Filters::Contact*>(*uit);
    if (tmp) {
      it.addE(tmp);
      TRACE(boost::format("[IN] ccReg_Contacts_i::loadFilter(%1%): loaded filter content = %2%") % _id % tmp->getContent());
    }
  }
}

void ccReg_Contacts_i::saveFilter(const char* _name) {
  Logging::Context ctx(base_context_);
  ConnectionReleaser releaser;

  TRACE(boost::format("[CALL] ccReg_Contacts_i::saveFilter('%1%')") % _name);

  std::unique_ptr<LibFred::Filter::Manager>
  tmp_filter_manager(LibFred::Filter::Manager::create());
  tmp_filter_manager->save(LibFred::Filter::FT_CONTACT, _name, uf);
}

LibFred::Contact::Contact* ccReg_Contacts_i::findId(ccReg::TID _id) {
  Logging::Context ctx(base_context_);

  try {
    LibFred::Contact::Contact *contact =
        dynamic_cast<LibFred::Contact::Contact*> (cl->findId(_id));
    if (contact) {
      return contact;
    }
    return 0;
  }
  catch (LibFred::NOT_FOUND) {
    return 0;
  }
}

CORBA::Boolean ccReg_Contacts_i::numRowsOverLimit() {
  Logging::Context ctx(base_context_);

  return cl->isLimited(); 
}

