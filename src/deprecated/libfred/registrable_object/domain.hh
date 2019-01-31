/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DOMAIN_HH_315E04A6F30B4BF29977B9D8FCA3C01A
#define DOMAIN_HH_315E04A6F30B4BF29977B9D8FCA3C01A

#include <string>
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "src/deprecated/libfred/zone.hh"
#include "src/deprecated/libfred/object.hh"
#include "src/deprecated/libfred/exceptions.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"
#include "src/deprecated/util/dbsql.hh"

#include "src/util/settings.hh"


using namespace boost::gregorian;

/// forward declared parameter type
class DB;

namespace LibFred {
namespace Domain {

/// member identification (i.e. for sorting)
enum MemberType {
  MT_FQDN, ///< fully-qualified domain name
  MT_CRDATE, ///< create date
  MT_ERDATE, ///< delete date
  MT_REGISTRANT_HANDLE, ///< registrant
  MT_REGISTRANT_NAME, ///< registratn name
  MT_REGISTRANT_ORG, ///< registrant organization
  MT_REGISTRANT_PHONE, ///< registrant phone
  MT_REGISTRAR_HANDLE, ///< registrar handle
  MT_ZONE_STATUS, ///< in/out zone flag
  MT_EXDATE, ///< expiration date
  MT_OUTZONEDATE, ///< out zone date
  MT_CANCELDATE, ///< cancel date
  MT_1ADMIN_NAME,///< 1. admin name
  MT_1ADMIN_ORG,///< 1. admin organization
  MT_1ADMIN_PHONE,///< 1. admin phone
  MT_2ADMIN_NAME,///< 2. admin name
  MT_2ADMIN_ORG,///< 2. admin organization
  MT_2ADMIN_PHONE,///< 2. admin phone
  MT_3ADMIN_NAME,///< 3. admin name
  MT_3ADMIN_ORG,///< 3. admin organization
  MT_3ADMIN_PHONE///< 3. admin phone
};


/// return type for checkAvail method
enum CheckAvailType {
  CA_INVALID_HANDLE, ///< bad formed handle
  CA_BAD_ZONE, ///< domain outside of registry
  CA_BAD_LENGHT, ///< domain longer then acceptable
  CA_BLACKLIST, ///< registration blocked in blacklist
  CA_REGISTRED, ///< domain registred
  CA_PARENT_REGISTRED, ///< parent already registred
  CA_CHILD_REGISTRED, ///< child already registred
  CA_AVAILABLE ///< domain is available
};


/// domain detail
class Domain : virtual public LibFred::Object {
public:
  /// public destructor
  virtual ~Domain() {
  }
  /// return fully qualified domain name
  virtual const std::string& getFQDN() const = 0;
  ///
  virtual const std::string& getHandle() const = 0;
  /// return fully qualified domain name translated from IDN encoded form
  virtual const std::string& getFQDNIDN() const = 0;
  /// return id of zone
  virtual TID getZoneId() const = 0;
  /// return handle of nsset
  virtual const std::string& getNssetHandle() const = 0;
  /// return id of nsset
  virtual TID getNssetId() const = 0;
  /// set nsset
  virtual void setNssetId(TID nsset) = 0;
  /// return handle of keyset
  virtual const std::string &getKeysetHandle() const = 0;
  /// return id of keyset
  virtual TID getKeysetId() const = 0;
  /// set keyset
  virtual void setKeysetId(TID keyset) = 0;
  /// return handle of registrant
  virtual const std::string& getRegistrantHandle() const = 0;
  /// return name of registrant
  virtual const std::string& getRegistrantName() const = 0;
  /// return registrant organization
  virtual const std::string& getRegistrantOrganization() const = 0;
  /// return registrant phone
  virtual const std::string& getRegistrantPhone() const = 0;
  /// return id of registrant
  virtual TID getRegistrantId() const = 0;
  /// set registrant
  virtual void setRegistrantId(TID registrant) = 0;
  /// return count of admin contacts
  virtual unsigned getAdminCount(unsigned role=1) const = 0;
  /// return id of admin contact by index
  virtual TID getAdminIdByIdx(unsigned idx, unsigned role=1) const
      = 0;
  /// return handle of admin contact by index
  virtual const std::string
      & getAdminHandleByIdx(unsigned idx, unsigned role=1) const
          = 0;
  /// remove contact from admin contact list
  virtual void removeAdminId(TID id) = 0;
  /// insert contact into admin contact list
  virtual void insertAdminId(TID id) = 0;
  /// return name of admin
  virtual const std::string getAdminNameByIdx(unsigned idx, unsigned role=1) const = 0;
  /// return Admin organization
  virtual const std::string getAdminOrganizationByIdx(unsigned idx, unsigned role=1) const = 0;
  /// return Admin phone
  virtual const std::string getAdminPhoneByIdx(unsigned idx, unsigned role=1) const = 0;
  /// return date of registration expiration
  virtual date getExpirationDate() const = 0;
  /// return date of validation expiration
  virtual date getValExDate() const = 0;
  /// return wheather has to be enum domain published in dictionary or not
  virtual bool getPublish() const = 0;
  /// return time of last change in zone generation status
  virtual ptime getZoneStatusTime() const = 0;
  /// return date when domain will not be generated in zone (= expiration date + 30 days) or erdate (see reload())
  virtual ptime getOutZoneDate() const = 0;
  /// return true time object delete from registry (= expiration date + 45 days) or erdate (see reload())
  virtual ptime getCancelDate() const = 0;
  /// return zone generation status
  virtual unsigned getZoneStatus() const = 0;
};


/// domain list
class List : virtual public ObjectList {
public:
  virtual ~List() {
  }
  /// get detail of loaded domain
  virtual Domain *getDomain(unsigned idx) const = 0;
  /// set filter for domain zone
  virtual void setZoneFilter(TID zoneId) = 0;
  /// set filter for registrant
  virtual void setRegistrantFilter(TID registrantId) = 0;
  /// set filter for registrant handle
  virtual void
      setRegistrantHandleFilter(const std::string& registrantHandle) = 0;
  /// set filter for nsset
  virtual void setNssetFilter(TID nssetId) = 0;
  /// set filter for nsset handle
  virtual void setNssetHandleFilter(const std::string& nssetHandle) = 0;
  /// set filter for keyset
  virtual void setKeysetFilter(TID keysetId) = 0;
  /// set filter for keyset handle
  virtual void setKeysetHandleFilter(const std::string &keysetHandle) = 0;
  //virtual void setKeysetHandleFilter(const std::string &_keysetHandle) = 0;
  /// set filter for admin
  virtual void setAdminFilter(TID adminId) = 0;
  /// set filter for admin handle
  virtual void setAdminHandleFilter(const std::string& adminHandle) = 0;
  /// set filter for temp contact
  virtual void setTempFilter(TID tempId) = 0;
  /// set filter for temp contact handle
  virtual void setTempHandleFilter(const std::string& tempHandle) = 0;
  /// set filter for contact in any role
  virtual void setContactFilter(TID contactId) = 0;
  /// set filter for handle of contact in any role
  virtual void setContactHandleFilter(const std::string& cHandle) = 0;
  /// set filter for domain name
  virtual void setFQDNFilter(const std::string& fqdn) = 0;
  /// set filter for period of expiration date
  virtual void setExpirationDateFilter(time_period period) = 0;
  /// set filter for period of val. expiration date
  virtual void setValExDateFilter(time_period period) = 0;
  /// set filter for admin handle in associated nsset
  virtual void setTechAdminHandleFilter(const std::string& handle) = 0;
  /// set filter for IP address in associated nsset
  virtual void setHostIPFilter(const std::string& ip) = 0;
  /// set filter for zone generation status
  virtual void setZoneStatusFilter(unsigned status) = 0;
  /// testing new reload function
  virtual void reload(Database::Filters::Union &uf) = 0;
  /// reload list with current filter
  virtual void reload() = 0;
  /// clear filter data
  virtual void clearFilter() = 0;
  /// sort
  virtual void sort(MemberType member, bool asc) = 0;
};


/// main entry class
class Manager {
public:
  /// destructor
  virtual ~Manager() {
  }
  /// check handle of domain without contacting database
  virtual CheckAvailType checkHandle(const std::string& fqdn,
                                     bool allowIDN) const = 0;
  /// check availability of domain
  virtual CheckAvailType checkAvail(const std::string& fqdn,
                                    NameIdPair& conflictFqdn,
                                    bool allowIDN,
                                    bool lock = false
                                    ) const = 0;
  /// return current count of domains by zone
  virtual unsigned long getDomainCount(const std::string& zone) const = 0;
  /// return current count of signed domains by zone
  virtual unsigned long getSignedDomainCount(const std::string & _fqdn) const = 0;
  /// return current count of enum numbers
  virtual unsigned long getEnumNumberCount() const = 0;
  /// test fqdn to be delete pending
  virtual bool isDeletePending(const std::string &_fqdn) const = 0;
  /// create list of domains
  virtual List *createList() = 0;
  /// factory method
  static Manager *create(DBSharedPtr db, Zone::Manager *zm);
};

unsigned long long getRegistrarDomainCount(Database::ID regid, const boost::gregorian::date &date, unsigned int zone_id);

/// return expired domain counts for specified date periods
std::vector<unsigned long long> getExpiredDomainSummary(const std::string &registrar, const std::vector<date_period> &date_intervals);

} // namespace Domain
} // namespace LibFred

#endif /*DOMAIN_H_*/
