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
#include "src/deprecated/model/registrar_filter.hh"

namespace Database {
namespace Filters {

RegistrarImpl::RegistrarImpl(bool _set_active) {
  setName("Registrar");
  active = _set_active;
}

RegistrarImpl::~RegistrarImpl() {
}

Table& RegistrarImpl::joinRegistrarTable() {
  return joinTable("registrar");
}

Value<Database::ID>& RegistrarImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinRegistrarTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string>& RegistrarImpl::addIco() {
  Value<std::string> *tmp = new Value<std::string>(Column("ico", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Ico");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addDic() {
  Value<std::string> *tmp = new Value<std::string>(Column("dic", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Dic");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addVarSymbol() {
  Value<std::string> *tmp = new Value<std::string>(Column("varsymb", joinRegistrarTable()));
  add(tmp);
  tmp->setName("VarSymbol");
  return *tmp;
}

Value<bool> & RegistrarImpl::addVat()
{
    Value<bool> *tmp = new Value<bool>(Column("vat", joinRegistrarTable()));
    add(tmp);
    tmp->setName("Vat");
    return *tmp;
}


Value<std::string>& RegistrarImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("handle", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Handle");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addName() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Name");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addOrganization() {
  Value<std::string> *tmp = new Value<std::string>(Column("organization", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Organization");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addStreet()
{
  Value<std::string> *tmp = new Value<std::string>(Column("(street1 || ' ' || street2 || ' ' || street3)", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Street");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addCity() {
  Value<std::string> *tmp = new Value<std::string>(Column("city", joinRegistrarTable()));
  add(tmp);
  tmp->setName("City");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addStateOrProvince() {
  Value<std::string> *tmp = new Value<std::string>(Column("stateorprovince", joinRegistrarTable()));
  add(tmp);
  tmp->setName("StateOrProvince");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addPostalCode() {
  Value<std::string> *tmp = new Value<std::string>(Column("postalcode", joinRegistrarTable()));
  add(tmp);
  tmp->setName("PostalCode");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addCountryCode() {
  Value<std::string> *tmp = new Value<std::string>(Column("country", joinRegistrarTable()));
  add(tmp);
  tmp->setName("CountryCode");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addTelephone() {
  Value<std::string> *tmp = new Value<std::string>(Column("telephone", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Telephone");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addFax() {
  Value<std::string> *tmp = new Value<std::string>(Column("fax", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Fax");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addEmail() {
  Value<std::string> *tmp = new Value<std::string>(Column("email", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Email");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addUrl() {
  Value<std::string> *tmp = new Value<std::string>(Column("url", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Url");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addZoneFqdn() {
    joinRegistrarTable();
    this->active = true;

    const char* subselect_reginzone = "(select distinct ri.registrarid as rid, z.id as zid "
            "from registrarinvoice ri join zone z on ri.zone = z.id "
            "where fromdate <= CURRENT_DATE "
            "and (todate >= CURRENT_DATE or todate is null) )";

    const char* subselect_zone = "(select id , fqdn from zone)";

    addJoin(new Join(
      Column("id", joinTable("registrar")),
      SQL_OP_EQ,
      Column("rid", joinTable(subselect_reginzone))
    ));

    addJoin(new Join(
      Column("zid", joinTable(subselect_reginzone)),
      SQL_OP_EQ,
      Column("id", joinTable(subselect_zone))
    ));

    Value<std::string> *tmp = new Value<std::string>(Column("fqdn", joinTable(subselect_zone)));
    tmp->setName("ZoneFqdn");
    add(tmp);
    return *tmp;
}

Value<Database::ID>& RegistrarImpl::addGroupId() {
    joinRegistrarTable();
    this->active = true;

    const char* subselect_regingroup = "(select distinct rgm.registrar_id as rid, rg.id as gid "
            "from registrar_group_map rgm join registrar_group rg on rgm.registrar_group_id = rg.id "
            "where member_from <= CURRENT_DATE "
            "and (member_until >= CURRENT_DATE or member_until is null) )";

    addJoin(new Join(
      Column("id", joinTable("registrar")),
      SQL_OP_EQ,
      Column("rid", joinTable(subselect_regingroup))
    ));

    Value<Database::ID> *tmp = new Value<Database::ID>(Column("gid", joinTable(subselect_regingroup)));
    tmp->setName("GroupId");
    add(tmp);
    return *tmp;
}


}
}
