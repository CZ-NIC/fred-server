#ifndef REGISTRAR_H_
#define REGISTRAR_H_

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>

#include <memory>
#include <string>
#include <map>
#include <vector>

#include "common_impl_new.h"
#include "common_object.h"
#include "object.h"
#include "types.h"
#include "exceptions.h" 
#include "db_settings.h"
#include "model/model_filters.h"
#include "old_utils/dbsql.h"
#include "decimal/decimal.h"
#include "types/money.h"

#include "model_registrar_acl.h"
#include "model_registrar.h"

#include "epp_corba_client.h"
#include "logger_client.h"

using namespace boost::posix_time;

/// forward declared parameter type 
class DB;


namespace Fred {
namespace Registrar {

class RegistrarZoneAccess;


/// member identification (i.e. for sorting)
enum MemberType {
  MT_NAME, ///< name
  MT_HANDLE, ///< contact identificator
  MT_URL, ///< url
  MT_MAIL, ///< email address
  MT_CREDIT, ///< credit
  MT_ICO, ///< ico
  MT_DIC, ///< dic
  MT_VARSYMB, ///< varsymbol
  MT_VAT, ///< vat
  MT_ORGANIZATION, ///< organization
  MT_STREET1, ///< address part
  MT_STREET2, ///< address part
  MT_STREET3, ///< address part
  MT_CITY, ///< city
  MT_PROVINCE, ///< state or province
  MT_POSTALCODE, ///< postal code
  MT_COUNTRY, ///< country code
  MT_TELEPHONE, ///< telephone
  MT_FAX, ///< fax
  MT_ZONE ///< zone
};


/// Access control attributes of registrar
class ACL {
protected:
  /// Protected destructor, object is managed by object Registrar
  virtual ~ACL() {
  }
public:
  /// Return MD5 of client SSL certificate for EPP communication
  virtual const std::string& getCertificateMD5() const = 0;
  /// Set MD5 of client SSL certificate for EPP communication
  virtual void setCertificateMD5(const std::string& newCertificateMD5) = 0;
  /// Return password for EPP login command
  virtual const std::string& getPassword() const = 0;
  /// Set password for EPP login command
  virtual void setPassword(const std::string& newPassword) = 0;
  virtual void setRegistrarId(const TID &_registrar_id) = 0;
};

/// Registrar's active zone structure
struct ZoneAccess
{
    TID id; /// registrarinvoice record id
    std::string name; /// zone name
    Money credit; /// registrar's credit
    Database::Date fromdate; /// from day
    Database::Date todate;/// to day
    ZoneAccess()
    :id()
        ,name()
        ,credit()
        ,fromdate()
        ,todate()
    {}
    ZoneAccess(std::string _name
            , Money _credit
            , Database::Date _fromdate
            , Database::Date _todate)
    :id()
    ,name(_name)
    ,credit(_credit)
    ,fromdate(_fromdate)
    ,todate(_todate)
    {}
    ZoneAccess(TID _id
            ,std::string _name
            , Money _credit
            , Database::Date _fromdate
            , Database::Date _todate)
    :id(_id)
    ,name(_name)
    ,credit(_credit)
    ,fromdate(_fromdate)
    ,todate(_todate)
    {}
};//struct ZoneAccess

/// Registrar detail access
class Registrar
	: virtual public Fred::CommonObjectNew
{
public:
  /// Public destructor, user is responsible for object delete
  virtual ~Registrar() {
  }

  virtual const TID& getId() const =0;
  virtual void setId(const unsigned long long &_id) =0;

  ///
  virtual const std::string& getIco() const = 0;
  ///
  virtual void setIco(const std::string& _ico) = 0;
  ///
  virtual const std::string& getDic() const = 0;
  ///
  virtual void setDic(const std::string& _dic) = 0;
  ///
  virtual const std::string& getVarSymb() const = 0;
  ///
  virtual void setVarSymb(const std::string& _var_symb) = 0;
  ///
  virtual bool getVat() const = 0;
  ///
  virtual void setVat(bool _vat) = 0;
  /// Return registrar handle (EPP session login name)
  virtual const std::string& getHandle() const = 0;
  /// Set registrar handle (EPP session login name)
  virtual void setHandle(const std::string& newHandle) = 0;
  /// Get registrar name 
  virtual const std::string& getName() const = 0;
  /// Set registrar name 
  virtual void setName(const std::string& newName) = 0;
  /// Get registrar URL       
  virtual const std::string& getURL() const = 0;
  /// Set registrar URL
  virtual void setURL(const std::string& newURL) = 0;
  /// Get registrar organization      
  virtual const std::string& getOrganization() const = 0;
  /// Set registrar organization
  virtual void setOrganization(const std::string& _organization) = 0;
  /// Get registrar street part 1
  virtual const std::string& getStreet1() const = 0;
  /// Set registrar street part 1
  virtual void setStreet1(const std::string& _street1) = 0;
  /// Get registrar street part 2
  virtual const std::string& getStreet2() const = 0;
  /// Set registrar street part 2
  virtual void setStreet2(const std::string& _street2) = 0;
  /// Get registrar street part 3
  virtual const std::string& getStreet3() const = 0;
  /// Set registrar street part 3
  virtual void setStreet3(const std::string& _street3) = 0;
  /// Get registrar city
  virtual const std::string& getCity() const = 0;
  /// Set registrar city
  virtual void setCity(const std::string& _city) = 0;
  /// Get registrar state or province 
  virtual const std::string& getProvince() const = 0;
  /// Set registrar state or province
  virtual void setProvince(const std::string& _province) = 0;
  /// Get registrar postal code
  virtual const std::string& getPostalCode() const = 0;
  /// Set registrar postal code
  virtual void setPostalCode(const std::string& _postalCode) = 0;
  /// Get registrar country code
  virtual const std::string& getCountry() const = 0;
  /// Set registrar country code
  virtual void setCountry(const std::string& _country) = 0;
  /// Get registrar telephone number
  virtual const std::string& getTelephone() const = 0;
  /// Set registrar telephone number
  virtual void setTelephone(const std::string& _telephone) = 0;
  /// Get registrar fax number
  virtual const std::string& getFax() const = 0;
  /// Set registrar fax number
  virtual void setFax(const std::string& _fax) = 0;
  /// Get registrar email
  virtual const std::string& getEmail() const = 0;
  /// Set registrar email
  virtual void setEmail(const std::string& _email) = 0;
  /// Get hidden flag for system registrar
  virtual bool getSystem() const = 0;
  /// Set hidden flag for system registrar
  virtual void setSystem(bool _system) = 0;
  /// Get total credit
  virtual Money getCredit() const = 0;
  /// Get credit for specific zone, id = 0 is unspecified zone (converted from null in database table invoice)
  virtual Money getCredit(Database::ID _zone_id) const = 0;
  /// Set credit for specific zone
  virtual void setCredit(Database::ID _zone_id, Money _credit) = 0;
  /// Create new ACL record
  virtual ACL* newACL() = 0;
  /// Return ACL list size
  virtual unsigned getACLSize() const = 0;
  /// Return ACL list member by index
  virtual ACL* getACL(unsigned idx) const = 0;
  /// Delete ACL or do nothing
  virtual void deleteACL(unsigned idx) = 0;
  /// Clear ACL list
  virtual void clearACLList() = 0;

  /// Create new ZoneAccess record
  virtual ZoneAccess* newZoneAccess() = 0;
  /// Return ZoneAccess list size
  virtual unsigned getZoneAccessSize() const = 0;
  /// Return ZoneAccess list member by index
  virtual ZoneAccess* getZoneAccess(unsigned idx) const = 0;
  /// Delete ZoneAccess or do nothing
  virtual void deleteZoneAccess(unsigned idx) = 0;
  /// Clear ZoneAccess list
  virtual void clearZoneAccessList() = 0;
  /// Look if registrar have currently access to zone by zone id
  virtual bool isInZone(unsigned id) const = 0;
  /// Look if registrar have currently access to zone by zone fqdn
  virtual bool isInZone(std::string zone_fqdn) const = 0;

  /// Save changes to database
  virtual void save() throw (SQL_ERROR) = 0;

  /// Zones number for credit by zone
  //virtual unsigned long getZonesNumber() = 0;

  ///Registrar smart pointer
  typedef std::auto_ptr<Registrar> AutoPtr;

};




/// List of registrar object
class RegistrarList : virtual public Fred::CommonListNew
{
public:
  /// public virtual destructor
  virtual ~RegistrarList()
  {}
  ///RegistrarList smart pointer
  typedef std::auto_ptr<RegistrarList> AutoPtr;
  /// testing new reload function
  virtual void reload(Database::Filters::Union &uf) = 0;
  /// Get registrar detail object by list index for update
  virtual Registrar* get(unsigned idx) const = 0;
  /// XXX get method with releaseing ownership functionality 
  virtual Registrar* getAndRelease(unsigned idx) = 0;
  /// sort by column
  virtual void sort(MemberType _member, bool _asc, unsigned _zone_id = 0
          , RegistrarZoneAccess* rzaptr =0 ) = 0;
  //virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual const char* getTempTableName() const = 0;

  virtual Fred::Registrar::Registrar* findId(Database::ID id) const =0;
};
//Add access to zone for registrar
unsigned long addRegistrarZone(
          const std::string& registrarHandle,
          const std::string zone,
          const Database::Date &fromDate,
          const Database::Date &toDate);

enum RegCertClass{c0,c1,c2,c3,c4,c5};///classification for registrar certification

/// Registrar certification structure
struct CertificationData
{
    TID id;
    Database::Date valid_from;
    Database::Date valid_until;
    RegCertClass classification;
    TID eval_file_id;
};//struct Certification
typedef  std::vector<CertificationData> CertificationSeq;

/// Registrar group structure
struct GroupData
{
    TID id;
    std::string name;
    Database::DateTime cancelled;
};
typedef  std::vector<GroupData> GroupSeq;


/// Registrar membership in group by registrar structure
struct MembershipByRegistrar
{
    TID id;
    TID group_id;
    Database::Date member_from;
    Database::Date member_until;
};
typedef  std::vector<MembershipByRegistrar> MembershipByRegistrarSeq;

/// Registrar membership in group by group structure
struct MembershipByGroup
{
    TID id;
    TID registrar_id;
    Database::Date member_from;
    Database::Date member_until;
};
typedef  std::vector<MembershipByGroup> MembershipByGroupSeq;

struct RequestFeeData {

RequestFeeData(std::string _reg_handle,
               Database::ID _reg_id,
               unsigned long long _request_count,
               unsigned long long _request_total_free,
               Decimal _price) :
                   reg_handle(_reg_handle),
                   reg_id(_reg_id),
                   request_count(_request_count),
                   request_total_free(_request_total_free),
                   price(_price)
    { }

    std::string reg_handle;
    Database::ID reg_id;
    unsigned long long request_count;
    unsigned long long request_total_free;
    Decimal price;
};

typedef std::map<std::string, RequestFeeData> RequestFeeDataMap;

struct BlockedReg {
    Database::ID reg_id;
    std::string reg_handle;
    std::string from_timestamp;
    Decimal price_limit;
    std::string email;
    std::string telephone;
};

typedef std::auto_ptr< std::vector<BlockedReg> > BlockedRegistrars;

/// Main entry point for Registrar namespace
class Manager {
public:
  /// Public destructor, user is responsible for delete
  virtual ~Manager() {}
  virtual bool checkHandle(const std::string) const throw (SQL_ERROR) = 0;

  virtual Registrar::AutoPtr createRegistrar() = 0;
  ///add Registrar acl record  used by fred-admin option registrar_acl_add
  virtual void addRegistrarAcl(
          const std::string &registrarHandle,
          const std::string &cert,
          const std::string &pass)
      throw (SQL_ERROR) = 0;

  virtual void updateRegistrarZone(
          const TID& id,
          const Database::Date &fromDate,
          const Database::Date &toDate) throw (SQL_ERROR) = 0;

  ///list factory
  virtual RegistrarList::AutoPtr createList() =0;
  ///registrar instance factory
  virtual Registrar::AutoPtr getRegistrarByHandle(const std::string& handle) =0;

  virtual unsigned long long getRegistrarByPayment(const std::string &varsymb,
                                                   const std::string &memo) = 0;

  ///create registrar group
  virtual unsigned long long createRegistrarGroup(const std::string &group_name) =0;
  ///cancel registrar group
  virtual void cancelRegistrarGroup(const TID& group_id) =0;
  ///update registrar group
  virtual void updateRegistrarGroup(const TID& group_id
          , const std::string &group_name) =0;
  ///create registrar certification
  virtual unsigned long long createRegistrarCertification( const TID& registrar_id
      , const Database::Date &valid_from
      , const Database::Date &valid_until
      , const RegCertClass classification
      , const TID& eval_file_id) =0;
  ///create registrar certification by handle
  virtual unsigned long long createRegistrarCertification( const std::string& registrar_handle
          , const Database::Date &valid_from
          , const Database::Date &valid_until
          , const RegCertClass classification
          , const TID& eval_file_id) =0;
  ///shorten registrar certification
  virtual void shortenRegistrarCertification( const TID& certification_id
      , const Database::Date &valid_until) =0;
  ///update registrar certification
  virtual void updateRegistrarCertification( const TID& certification_id
      , const RegCertClass classification
      , const TID& eval_file_id) =0;
  ///get registrar certifications
  virtual CertificationSeq getRegistrarCertifications( const TID& registrar_id) =0;

  ///create membership of registrar in group
  virtual unsigned long long createRegistrarGroupMembership( const TID& registrar_id
      , const TID& registrar_group_id
      , const Database::Date &member_from
      , const Database::Date &member_until) =0;
  ///create membership of registrar in group
  virtual unsigned long long createRegistrarGroupMembership( const std::string& registrar_handle
      , const std::string& registrar_group
      , const Database::Date &member_from
      , const Database::Date &member_until) =0;

  ///update membership of registrar in group
  virtual void updateRegistrarGroupMembership( const TID& mebership_id
      , const TID& registrar_id
      , const TID& registrar_group_id
      , const Database::Date &member_from
      , const Database::Date &member_until) =0;
  ///end of registrar membership in group
  virtual void endRegistrarGroupMembership(const TID& registrar_id
      , const TID& registrar_group_id) =0;

  ///get registrar groups
  virtual GroupSeq getRegistrarGroups() =0;
  ///get membership by registrar
  virtual MembershipByRegistrarSeq getMembershipByRegistrar( const TID& registrar_id) =0;
  ///get membership by groups
  virtual MembershipByGroupSeq getMembershipByGroup( const TID& group_id) =0;

  virtual bool blockRegistrar(const TID &registrar_id, const EppCorbaClient *epp_cli) = 0;
  virtual void unblockRegistrar(const TID &registrar_id, const TID &request_id) = 0;

  virtual void blockRegistrarsOverLimit(const EppCorbaClient *epp_client,
          Logger::LoggerClient *logger_client) = 0;

  virtual BlockedRegistrars getRegistrarsBlockedToday(void) = 0;

  virtual bool hasRegistrarZoneAccess(const unsigned long long &_registrar_id,
                                      const unsigned long long &_zone_id) = 0;

  /// logs error and throws when registrar doesn't exist
  virtual void checkRegistrarExists( const TID & registrar_id) = 0;

  virtual bool isRegistrarBlocked(Database::ID regId) = 0;

  virtual std::auto_ptr<RequestFeeDataMap> getRequestFeeDataMap(
          Logger::LoggerClient *logger_client,
          boost::posix_time::ptime p_from,
          boost::posix_time::ptime p_to,
          boost::gregorian::date zone_access_date) = 0;

  typedef std::auto_ptr<Fred::Registrar::Manager> AutoPtr;

  /// Factory method
  static AutoPtr create(DBSharedPtr db);
};//class Manager

///storage for flag of registrar's access to zone, used in registrar pagetable
class RegistrarZoneAccess
{
    enum ColIndex {RegistrarCol, ZoneCol, IsInZone};///order of query columns
    unsigned long long max_registrar_id;///maximal registrar id with access to zone in database
    unsigned long long max_zone_id;///maximal zone id accessed by some registrar in database
    typedef std::vector<bool> RegistrarZoneAccessRow;///registrar's zones flags
    typedef std::vector<RegistrarZoneAccessRow> RegistrarZoneAccessArray;///container of registrars rows
    RegistrarZoneAccessArray flag; ///zone access flag array
    unsigned long long max_id(ColIndex idx, Database::Result& result);///for size of flag array
public:
    void reload();///load from database
    bool isInZone(unsigned long long registrar_id,unsigned long long zone_id);///look if registrar currently have access to zone by id
};//class RegistrarZoneAccess


};//namespace Registrar
};//namespace Fred

#endif /*REGISTRAR_H_*/
