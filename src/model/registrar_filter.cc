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

Value<std::string>& RegistrarImpl::addCity() {
  Value<std::string> *tmp = new Value<std::string>(Column("city", joinRegistrarTable()));
  add(tmp);
  tmp->setName("City");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addEmail() {
  Value<std::string> *tmp = new Value<std::string>(Column("email", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Email");
  return *tmp;
}

Value<std::string>& RegistrarImpl::addCountry() {
  Value<std::string> *tmp = new Value<std::string>(Column("country", joinRegistrarTable()));
  add(tmp);
  tmp->setName("Country");
  return *tmp;
}

}
}
