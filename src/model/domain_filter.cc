#include "domain_filter.h"

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
  tmp->addPostValueString("::date - (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval");
  tmp->setName("OutZoneDate");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainImpl::addCancelDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval");
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
  tmp->addPostValueString("::date - (SELECT val || ' day' FROM enum_parameters WHERE id = 4)::interval");
  tmp->setName("OutZoneDate");
  add(tmp);
  return *tmp;
}

Interval<Database::DateInterval>& DomainHistoryImpl::addCancelDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("exdate", joinDomainTable()));
  tmp->addPostValueString("::date - (SELECT val || ' day' FROM enum_parameters WHERE id = 6)::interval");
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

Table& DomainHistoryImpl::joinDomainTable() {
  return joinTable("domain_history");
}

void DomainHistoryImpl::_joinPolymorphicTables() {
  Table *d = findTable("domain_history");
  if (d) {
    joins.push_back(new Join(
        Column("historyid", joinTable("object_history")),
        SQL_OP_EQ,
        Column("historyid", *d)
    ));
  }
  ObjectHistoryImpl::_joinPolymorphicTables();
}

}
}
