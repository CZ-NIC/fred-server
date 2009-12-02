#include "registrar_filter.h"

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
    addJoin(new Join(
      Column("zone", joinTable("registrarinvoice")),
      SQL_OP_EQ,
      Column("id", joinTable("zone"))
    ));
    Value<std::string> *tmp = new Value<std::string>(Column("fqdn", joinTable("zone")));
    tmp->setName("ZoneFqdn");
    add(tmp);
    return *tmp;

}


Zone& RegistrarImpl::_addRIFilter()
{
 Zone *tmp = new ZoneImpl();
 //enabled registrars only filter

  add(tmp);
  tmp->addJoin(new Join(
	  Column("zone", joinTable("registrarinvoice")),
	  SQL_OP_EQ,
	  Column("id", tmp->joinZoneTable())));
  tmp->joinOn(new Join(
	  Column("id", joinRegistrarTable()),
	  SQL_OP_EQ,
	  Column("registrarid", joinTable("registrarinvoice"))));
  Value<Database::DateTime> *tmp2 = new Value<Database::DateTime>(Column("todate", joinTable("registrarinvoice")));
  tmp2->setNULL();
  tmp->add(tmp2);
  return *tmp;
}

Zone& RegistrarImpl::addActiveZone()
{
	  Zone &tmp = _addRIFilter();
	  tmp.setName("RegistrarZone");
	  return tmp;
}


}
}
