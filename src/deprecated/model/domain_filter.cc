/*
 * Copyright (C) 2008-2020  CZ.NIC, z. s. p. o.
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

#include "src/deprecated/model/domain_filter.hh"

namespace Database {
namespace Filters {

/** 
 * Table which joins the enumval (or enumval_history) table to domain
 * (domain_history) 
 * Doing this in a direct way wouldn't yield the correct table order or 
 * it would cause duplicity in joins. A class inheriting from Compound had to be used
 */
class EnumVal : public Compound {

public:
	EnumVal(bool h) {
		enable_history = h;
		publish = new Value<bool>(Column("publish", joinEnumValTable()));
		publish->setName("Publish");
		add(publish);
		if(!enable_history) {
			setName("EnumVal");
		} else {
			setName("EnumValHistory");
		}
	}
	
	~EnumVal() {
	}

	Table& joinEnumValTable()
	{
		if(!enable_history) {
			return joinTable("enumval");
		} else {
			return joinTable("enumval_history");
		}
	}

	Value<bool> &getPublish() {
		return *publish;
	}

	friend class boost::serialization::access;
	  template<class Archive> void serialize(Archive& _ar,
	      const unsigned int _version) {
	    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
	  }

	private:
	Value<bool> *publish;
	bool enable_history;
};

Domain* Domain::create() {
  return new DomainHistoryImpl();
}

/*
 * DOMAIN IMPLEMENTATION
 */
DomainImpl::DomainImpl() :
  ObjectImpl() {
  setName("Domain");
  addType().setValue(getType());
}

DomainImpl::~DomainImpl() {
}

Value<std::string>& DomainImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->addPreValueString("LOWER(");
  tmp->addPostValueString(")");
  tmp->setName("Handle");
  return *tmp;
}

Value<Database::ID>& DomainImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinDomainTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string> & DomainImpl::addFQDN() {
    return addHandle();
}

Value<Database::ID>& DomainImpl::addNSSetId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("nsset", joinDomainTable()));
  tmp->setName("NSSetId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& DomainImpl::addKeySetId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("keyset", joinDomainTable()));
  tmp->setName("KeySetId");
  add(tmp);
  return *tmp;
}

Value<bool>& DomainImpl::addPublish() {

  EnumVal *tmp = new EnumVal(false);
  tmp->joinOn(new Join(Column("id", joinDomainTable()),
		SQL_OP_EQ,
		Column("domainid", tmp->joinEnumValTable())
	));
  add(tmp);
  return tmp->getPublish();

}

Value<Database::ID>& DomainImpl::addRegistrantId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("registrant", joinDomainTable()));
  tmp->setName("RegistrantId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& DomainImpl::addZoneId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("zone", joinDomainTable()));
  tmp->setName("ZoneId");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainImpl::addExpirationDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->setName("ExpirationDate");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainImpl::addOutZoneDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - " + this->joinDomainTable().getAlias() + ".expiration_dns_protection_period");
  tmp->setName("OutZoneDate");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainImpl::addCancelDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - " + this->joinDomainTable().getAlias() + ".expiration_registration_protection_period");
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

KeySet& DomainImpl::addKeySet() {
  KeySet *tmp = new KeySetImpl();
  tmp->joinOn(new Join(Column("keyset", joinDomainTable()), SQL_OP_EQ, Column("id", tmp->joinKeySetTable())));
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

namespace {

constexpr auto domain_join_dlp = "(SELECT d.*,dlp.expiration_dns_protection_period,dlp.expiration_registration_protection_period "
                                  "FROM domain d "
                                  "JOIN domain_lifecycle_parameters dlp ON dlp.valid_for_exdate_after=(SELECT MAX(valid_for_exdate_after) "
                                                                                                      "FROM domain_lifecycle_parameters "
                                                                                                      "WHERE valid_for_exdate_after<=d.exdate))";

}//namespace Database::Filters::{anonymous}

Table& DomainImpl::joinDomainTable() {
  return joinTable(domain_join_dlp);
}

void DomainImpl::_joinPolymorphicTables() {
  ObjectImpl::_joinPolymorphicTables();
  Table *d = findTable(domain_join_dlp);
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
  addType().setValue(getType());
}

DomainHistoryImpl::~DomainHistoryImpl() {
}

Value<std::string>& DomainHistoryImpl::addHandle() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinObjectRegistryTable()));
  add(tmp);
  tmp->addPreValueString("LOWER(");
  tmp->addPostValueString(")");
  tmp->setName("Handle");
  return *tmp;
}

Value<Database::ID>& DomainHistoryImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinDomainTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string>& DomainHistoryImpl::addFQDN() {
  return addHandle();
}

Value<Database::ID>& DomainHistoryImpl::addNSSetId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("nsset", joinDomainTable()));
  tmp->setName("NSSetId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& DomainHistoryImpl::addKeySetId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("keyset", joinDomainTable()));
  tmp->setName("KeySetId");
  add(tmp);
  return *tmp;
}

Value<bool>& DomainHistoryImpl::addPublish() {

  EnumVal *tmp = new EnumVal(true);
  tmp->joinOn(new Join(Column("historyid", joinDomainTable()),
		SQL_OP_EQ,
		Column("historyid", tmp->joinEnumValTable())
	));
  add(tmp);
  return tmp->getPublish();

}

Value<Database::ID>& DomainHistoryImpl::addRegistrantId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("registrant", joinDomainTable()));
  tmp->setName("RegistrantId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& DomainHistoryImpl::addZoneId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("zone", joinDomainTable()));
  tmp->setName("ZoneId");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainHistoryImpl::addExpirationDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->setName("ExpirationDate");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainHistoryImpl::addOutZoneDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - " + this->joinDomainTable().getAlias() + ".expiration_dns_protection_period");
  tmp->setName("OutZoneDate");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainHistoryImpl::addCancelDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - " + this->joinDomainTable().getAlias() + ".expiration_registration_protection_period");
  tmp->setName("CancelDate");
  add(tmp);
  return *tmp;
}

Contact& DomainHistoryImpl::addRegistrant() {
  Contact *tmp = Contact::create();
  tmp->setName("Registrant");
  tmp->joinOn(new Join(Column("registrant", joinDomainTable()), SQL_OP_EQ, Column("id", tmp->joinContactTable())));
  add(tmp);
  return *tmp;
}

NSSet& DomainHistoryImpl::addNSSet() {
  NSSet *tmp = NSSet::create();
  tmp->setName("NSSet");
  tmp->joinOn(new Join(Column("nsset", joinDomainTable()), SQL_OP_EQ, Column("id", tmp->joinNSSetTable())));
  add(tmp);
  return *tmp;
}

KeySet &
DomainHistoryImpl::addKeySet()
{
    KeySet *tmp = new KeySetHistoryImpl();
    tmp->setName("KeySet");
    tmp->joinOn(
            new Join(
                Column("keyset", joinDomainTable()),
                SQL_OP_EQ,
                Column("id", tmp->joinKeySetTable())
                )
            );
    add(tmp);
    return *tmp;
}

Contact& DomainHistoryImpl::_addDCMFilter(unsigned _role) {
  Contact *tmp = Contact::create();
  Value<int> *role_filter = new Value<int>(Column("role", joinTable("domain_contact_map_history")), _role);
  role_filter->setValue(_role);
  add(role_filter);
  add(tmp);
  tmp->addJoin(new Join(
      Column("id", tmp->joinObjectRegistryTable()),
      SQL_OP_EQ,
      Column("contactid", joinTable("domain_contact_map_history"))));
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

namespace {

constexpr auto domain_history_join_dlp =
        "(SELECT dh.*,dlp.expiration_dns_protection_period,dlp.expiration_registration_protection_period "
         "FROM domain_history dh "
         "JOIN domain_lifecycle_parameters dlp ON dlp.valid_for_exdate_after=(SELECT MAX(valid_for_exdate_after) "
                                                                             "FROM domain_lifecycle_parameters "
                                                                             "WHERE valid_for_exdate_after<=dh.exdate))";

}//namespace Database::Filters::{anonymous}

Table& DomainHistoryImpl::joinDomainTable() {
  return joinTable(domain_history_join_dlp);
}

void DomainHistoryImpl::_joinPolymorphicTables() {
  Table *d = findTable(domain_history_join_dlp);
  if (d) {
    joins.push_back(new Join(
        Column("historyid", joinTable("object_history")),
        SQL_OP_EQ,
        Column("historyid", *d)
    ));
  }
  ObjectHistoryImpl::_joinPolymorphicTables();
}

}//namespace Database::Filters
}//namespace Database
