#include "domain_filter.h"

namespace DBase {
namespace Filters {

/*
 * DOMAIN IMPLEMENTATION 
 */
DomainImpl::DomainImpl() :
  ObjectImpl() {
  setName("Domain");
  setType(getType());
}

DomainImpl::~DomainImpl() {
}

Value<DBase::ID>& DomainImpl::addId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("id", joinDomainTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<DBase::ID>& DomainImpl::addNSSetId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("nsset", joinDomainTable()));
  tmp->setName("NSSetId");
  add(tmp);
  return *tmp;
}

Value<DBase::ID>& DomainImpl::addRegistrantId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("registrant", joinDomainTable()));
  tmp->setName("RegistrantId");
  add(tmp);
  return *tmp;
}

Value<DBase::ID>& DomainImpl::addZoneId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("zone", joinDomainTable()));
  tmp->setName("ZoneId");
  add(tmp);
  return *tmp;
}

Interval<DBase::DateInterval>& DomainImpl::addExpirationDate() {
  Interval<DBase::DateInterval> *tmp = new Interval<DBase::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->setName("ExpirationDate");
  add(tmp);
  return *tmp;
}

Interval<DBase::DateInterval>& DomainImpl::addOutZoneDate() {
  Interval<DBase::DateInterval> *tmp = new Interval<DBase::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - (select val from enum_parameters where id = 4)::int");
  tmp->setName("OutZoneDate");
  add(tmp);
  return *tmp;
}

Interval<DBase::DateInterval>& DomainImpl::addCancelDate() {
  Interval<DBase::DateInterval> *tmp = new Interval<DBase::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - (select val from enum_parameters where id = 6)::int");
  tmp->setName("CancelDate");
  add(tmp);
  return *tmp;
}

Contact& DomainImpl::addRegistrant() {
  Contact *tmp = new ContactImpl();
  tmp->setName("Registrant");
  tmp->joinOn(new Join(Column("registrant", joinDomainTable()), SQL_OP_EQ, Column("id", tmp->joinContactTable())));
  add(tmp);
  return *tmp;
}

NSSet& DomainImpl::addNSSet() {
  NSSet *tmp = new NSSetImpl();
  tmp->joinOn(new Join(Column("nsset", joinDomainTable()), SQL_OP_EQ, Column("id", tmp->joinNSSetTable())));
  add(tmp);
  return *tmp;
}

Contact& DomainImpl::_addDCMFilter(unsigned _role) {
  Contact *tmp = new ContactImpl();
  add(new Value<int>(Column("role", joinTable("domain_contact_map")), _role));
  add(tmp);
  tmp->addJoin(new Join(
      Column("contactid", joinTable("domain_contact_map")),
      SQL_OP_EQ,
      Column("id", tmp->joinObjectRegistryTable())));
  tmp->joinOn(new Join(
      Column("id", joinDomainTable()),
      SQL_OP_EQ,
      Column("domainid", joinTable("domain_contact_map"))));
  return *tmp;
}

Contact& DomainImpl::addAdminContact() {
  Contact &tmp = _addDCMFilter(1);
  tmp.setName("AdminContact");
  return tmp;
}

Contact& DomainImpl::addTempContact() {
  Contact &tmp = _addDCMFilter(2);
  tmp.setName("TempContact");
  return tmp;
}

Table& DomainImpl::joinDomainTable() {
  return joinTable("domain");
}

void DomainImpl::_joinPolymorphicTables() {
  ObjectImpl::_joinPolymorphicTables();
  Table *d = findTable("domain");
  if (d) {
    joins.push_back(new Join(
        Column("id", joinTable("object_registry")),
        SQL_OP_EQ,
        Column("id", *d)
    ));
  }
}

/*
 * DOMAIN HISTORY IMPLEMENTATION 
 */
DomainHistoryImpl::DomainHistoryImpl() :
  ObjectHistoryImpl() {
  setName("DomainHistory");
  setType(getType());
}

DomainHistoryImpl::~DomainHistoryImpl() {
}

Value<DBase::ID>& DomainHistoryImpl::addId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("id", joinDomainTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<DBase::ID>& DomainHistoryImpl::addNSSetId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("nsset", joinDomainTable()));
  tmp->setName("NSSetId");
  add(tmp);
  return *tmp;
}

Value<DBase::ID>& DomainHistoryImpl::addRegistrantId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("registrant", joinDomainTable()));
  tmp->setName("RegistrantId");
  add(tmp);
  return *tmp;
}

Value<DBase::ID>& DomainHistoryImpl::addZoneId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("zone", joinDomainTable()));
  tmp->setName("ZoneId");
  add(tmp);
  return *tmp;
}

Interval<DBase::DateInterval>& DomainHistoryImpl::addExpirationDate() {
  Interval<DBase::DateInterval> *tmp = new Interval<DBase::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->setName("ExpirationDate");
  add(tmp);
  return *tmp;
}

Interval<DBase::DateInterval>& DomainHistoryImpl::addOutZoneDate() {
  Interval<DBase::DateInterval> *tmp = new Interval<DBase::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - (select val from enum_parameters where id = 4)::int");
  tmp->setName("OutZoneDate");
  add(tmp);
  return *tmp;
}

Interval<DBase::DateInterval>& DomainHistoryImpl::addCancelDate() {
  Interval<DBase::DateInterval> *tmp = new Interval<DBase::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - (select val from enum_parameters where id = 6)::int");
  tmp->setName("CancelDate");
  add(tmp);
  return *tmp;
}

Contact& DomainHistoryImpl::addRegistrant() {
  Contact *tmp = new ContactHistoryImpl();
  tmp->setName("Registrant");
  tmp->joinOn(new Join(Column("registrant", joinDomainTable()), SQL_OP_EQ, Column("id", tmp->joinContactTable())));
  add(tmp);
  return *tmp;
}

NSSet& DomainHistoryImpl::addNSSet() {
  NSSet *tmp = new NSSetHistoryImpl();
  tmp->joinOn(new Join(Column("nsset", joinDomainTable()), SQL_OP_EQ, Column("id", tmp->joinNSSetTable())));
  add(tmp);
  return *tmp;
}

Contact& DomainHistoryImpl::_addDCMFilter(unsigned _role) {
  Contact *tmp = new ContactHistoryImpl();
  Value<int> *role_filter = new Value<int>(Column("role", joinTable("domain_contact_map_history")), _role);
  role_filter->setValue(_role); 
  add(role_filter);
  add(tmp);
  tmp->addJoin(new Join(
      Column("contactid", joinTable("domain_contact_map_history")),
      SQL_OP_EQ,
      Column("id", tmp->joinObjectRegistryTable())));
  tmp->joinOn(new Join(
      Column("historyid", joinDomainTable()),
      SQL_OP_EQ,
      Column("historyid", joinTable("domain_contact_map_history"))));
  return *tmp;
}

Contact& DomainHistoryImpl::addAdminContact() {
  Contact &tmp = _addDCMFilter(1);
  tmp.setName("AdminContact");
  return tmp;
}

Contact& DomainHistoryImpl::addTempContact() {
  Contact &tmp = _addDCMFilter(2);
  tmp.setName("TempContact");
  return tmp;
}

Table& DomainHistoryImpl::joinDomainTable() {
  return joinTable("domain_history");
}

void DomainHistoryImpl::_joinPolymorphicTables() {
  ObjectHistoryImpl::_joinPolymorphicTables();
  Table *d = findTable("domain_history");
  if (d) {
    joins.push_back(new Join(
        Column("historyid", joinTable("object_registry")),
        SQL_OP_EQ,
        Column("historyid", *d)
    ));
  }
}

}
}
