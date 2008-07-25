#include "epp_session_filter.h"

namespace Database {
namespace Filters {

EppSessionImpl::EppSessionImpl() {
  setName("EppSession");
}

EppSessionImpl::~EppSessionImpl() {
}

Table& EppSessionImpl::joinLoginTable() {
  return joinTable("login");
}

Registrar& EppSessionImpl::addRegistrar() {
  RegistrarImpl* tmp = new RegistrarImpl();
  add(tmp);
  tmp->setName("Registrar");
  tmp->joinOn(new Join(
      Column("registrarid", joinLoginTable()),
      SQL_OP_EQ,
      Column("id", tmp->joinRegistrarTable())
  ));
  return *tmp;
}

Interval<Database::DateInterval>& EppSessionImpl::addLogin() {
  Interval<Database::DateInterval> *tmp(new Interval<Database::DateInterval>(
      Column("logindate", joinLoginTable())));
  tmp->setName("Login");
  add(tmp);
  return *tmp;
}

}
}
