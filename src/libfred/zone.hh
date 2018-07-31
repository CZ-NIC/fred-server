#ifndef ZONE_HH_2C5BE1B6D84C4463A8A448A6E083225F
#define ZONE_HH_2C5BE1B6D84C4463A8A448A6E083225F

#include <idna.h>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "src/libfred/common_impl_new.hh"
#include "src/libfred/common_object.hh"
#include "src/libfred/exceptions.hh"
#include "src/libfred/object.hh"
#include "src/libfred/types.hh" 
#include "src/util/decimal/decimal.hh"
#include "src/util/types/data_types.hh"
#include "src/util/types/money.hh"

/// forward declaration for database connection
class DB;

namespace LibFred
{
  namespace Zone
  {
      enum Operation {
          CREATE,
          RENEW
      };

      /// member identification (i.e. for sorting)
      enum MemberType
      {
         MT_FQDN ///< zone name
        ,MT_EXPERIODMIN ///< extension period min
        ,MT_EXPERIODMAX ///< extension period max
        ,MT_VALPERIOD ///< revalidation period
        ,MT_DOTSMAX ///< max number of dots in domain name
        ,MT_ENUMZONE ///< is enum domain
        ,MT_TTL ///< time to live
        ,MT_HOSTMASTER ///< hostmaster
        ,MT_SERIAL ///< serial
        ,MT_REFRESH ///< refresh
        ,MT_UPDATERETR ///< update retry
        ,MT_EXPIRY ///< expiry
        ,MT_MINIMUM ///< minimum
        ,MT_NSFQDN ///< ns name
      };

    /// exception thrown when string is not a domain name
    class INVALID_DOMAIN_NAME : public std::runtime_error
    {
    public:
          INVALID_DOMAIN_NAME()
        : std::runtime_error("Zone INVALID_DOMAIN_NAME")
        {}
    };
    /// exception thrown when string is not a phone number
    class NOT_A_NUMBER : public std::runtime_error
    {
    public:
        NOT_A_NUMBER()
        : std::runtime_error("Zone NOT_A_NUMBER")
        {}
    };
    /// tokenized domain name type
    typedef std::vector<std::string> DomainName;

    ///zone ns records
    class ZoneNs
    {
    protected:
        /// protected destruktor - object is managed by Manager object
         virtual ~ZoneNs() {}
    public:
         ///get zonens id
         virtual const TID &getId() const = 0;
         ///set zonens id
         virtual void setId(const TID id) = 0;
         ///get zone id
         virtual const TID &getZoneId() const = 0;
         ///set zone id
         virtual void setZoneId(const TID id) = 0;
         ///get zonens name
         virtual const std::string &getFqdn() const = 0;
         ///set zonens name
         virtual void setFqdn(const std::string &fqdn) = 0;
         ///get zonens addrs
         virtual const std::string &getAddrs() const = 0;
         ///set zonens addrs
         virtual void setAddrs(const std::string &addrs) = 0;

    };

    /// zone attributes and specific parameters 
    class Zone
    {
    public:
     /// public virtual destructor
      virtual ~Zone() {}
      ///get zone id
      virtual const TID &getId() const = 0;
      ///set zone id
      virtual void setId(const TID id) = 0;
      ///get zone name
      virtual const std::string &getFqdn() const = 0;
      ///set zone name
      virtual void setFqdn(const std::string &fqdn) = 0;
      ///get zone min exp
      virtual const int &getExPeriodMin() const = 0;
      ///set zone min exp
      virtual void setExPeriodMin(const int exPeriodMin) = 0;
      ///get zone max exp
      virtual const int &getExPeriodMax() const = 0;
      ///set zone max exp
      virtual void setExPeriodMax(const int exPeriodMax) = 0;
      ///get zone ValPeriod
      virtual const int &getValPeriod() const = 0;
      ///set zone ValPeriod
      virtual void setValPeriod(const int valPeriod) = 0;
      ///get zone max dots
      virtual const int &getDotsMax() const = 0;
      ///set zone max dots
      virtual void setDotsMax(const int dotsMax) = 0;
      ///get zone is_enum
      virtual const bool &getEnumZone() const = 0;
      virtual const bool &isEnumZone() const = 0;
      ///set zone is_enum
      virtual void setEnumZone(const bool enumZone) = 0;
      ///return maximal level of domains in this zone
      virtual unsigned getMaxLevel() const = 0;

      virtual const int &getTtl() const = 0;
      virtual void setTtl(const int ttl) = 0;

      virtual const std::string &getHostmaster() const = 0;
      virtual void setHostmaster(const std::string &hostmaster) = 0;

      virtual const int &getSerial() const = 0;
      virtual void setSerial(const int serial) = 0;

      virtual const int &getRefresh() const = 0;
      virtual void setRefresh(const int refresh) = 0;

      virtual const int &getUpdateRetr() const = 0;
      virtual void setUpdateRetr(const int updateRetr) = 0;

      virtual const int &getExpiry() const = 0;
      virtual void setExpiry(const int expiry) = 0;

      virtual const int &getMinimum() const = 0;
      virtual void setMinimum(const int minimum) = 0;

      virtual const std::string &getNsFqdn() const = 0;
      virtual void setNsFqdn(const std::string &nsFqdn) = 0;

      /// Create new ZoneNs record
      virtual ZoneNs* newZoneNs() = 0;
      /// Return ZoneNs list size
      virtual unsigned getZoneNsSize() const = 0;
      /// Return ZoneNs list member by index
      virtual ZoneNs* getZoneNs(unsigned idx) const = 0;
      /// Delete ZoneNs or do nothing
      virtual void deleteZoneNs(unsigned idx) = 0;
      /// Clear ZoneNs list
      virtual void clearZoneNsList() = 0;
      /// Check if zone is applicable for given domain
      virtual bool isDomainApplicable(const std::string& domain) const =0;

    };//class Zone

    /// List of Zone object
    class ZoneList : virtual public LibFred::CommonListNew
    {
    public:
      virtual ~ZoneList() {
      }
      /// Filter in id
      virtual void setIdFilter(TID id) = 0;
      /// Filter in fqdn
      virtual void setFqdnFilter(const std::string& fqdn) = 0;
      /// Filter in registrar handle
      virtual void setRegistrarHandleFilter(const std::string& registrar_handle) = 0;
      /// reload actual list of zones
      virtual void reload()   = 0;
      /// reload with filter
      virtual void reload(Database::Filters::Union &uf) = 0;
      /// Get zone detail object by list index for update
      //  virtual Zone* get(unsigned idx) const = 0;
      /// Create new zone in list
      virtual Zone* create() = 0;
      /// clear filter data
      virtual void clearFilter() = 0;
      /// sort by column
      virtual void sort(MemberType _member, bool _asc) = 0;

      virtual const char* getTempTableName() const = 0;
      virtual void makeQuery(bool, bool, std::stringstream&) const = 0;

      virtual LibFred::Zone::Zone* findId(Database::ID id) const =0;
    };

    class idn_conversion_fail : public std::exception {};
    /// holder for zones managed by registry
    class Manager
    {
     public:
      /// destruktor
      virtual ~Manager() {}
      /// check Punycode validity
      virtual bool is_valid_punycode(const std::string& fqdn) const = 0;
      /// encode UTF8 domain name into IDN ascii string
      virtual std::string utf8_to_punycode(const std::string& fqdn) const = 0;
      /// decode IDN ascii domain name into UTF8 string
      virtual std::string punycode_to_utf8(const std::string& fqdn) const = 0;
      /// tokenize domain name into sequence
      virtual void parseDomainName(
        const std::string& fqdn, DomainName& domain, bool allowIDN
      ) const = 0;
      /// check validity of enum domain name (every part is one digit)
      virtual bool checkEnumDomainName(const DomainName& domain) const = 0;
      /// check if domain is under global zone e164.arpa
      virtual bool checkEnumDomainSuffix(const std::string& fqdn) const = 0;
      /// translate phone number into domain name
      virtual std::string makeEnumDomain(const std::string& number)
        const = 0;
      /// return default enum country prefix e.g '0.2.4'
      virtual const std::string& getDefaultEnumSuffix() const = 0;
      /// return default domain suffix e.g. 'cz'
      virtual const std::string& getDefaultDomainSuffix() const = 0;
      /// return enum zone string 'e164.arpa'
      virtual const std::string& getEnumZoneString() const = 0;
      /// find zone from domain fqdn
      virtual const Zone* findApplicableZone(const std::string& domain_fqdn) const = 0;
      /// check fqdn agains list of toplevel domain (true=found) 
      virtual bool checkTLD(const DomainName& domain) const = 0;
      /// add zone with zone_soa record

      virtual void addZone(
              const std::string& _fqdn,
              int _ex_period_min = 12,
              int _ex_period_max = 120,
              int _ttl = 18000,
              const std::string& _hostmaster = "hostmaster@localhost",
              int _refresh = 10600,
              int _update_retr = 3600,
              int _expiry = 1209600,
              int _minimum = 7200,
              const std::string& _ns_fqdn = "localhost") = 0;

      virtual void addZoneNs(
              const std::string& _zone,
              const std::string& _fqdn = "localhost",
              const std::string& _addr = "") = 0;

      virtual void addPrice(
              int zone,
              const std::string& operation,
              const Database::DateTime &validFrom,
              const Database::DateTime &validTo,
              const Money &price,
              int period
              , const bool enable_postpaid_operation) = 0;
      virtual void addPrice(
              const std::string &zone,
              const std::string& operation,
              const Database::DateTime &validFrom,
              const Database::DateTime &validTo,
              const Money &price,
              int period
              , const bool enable_postpaid_operation) = 0;
      /// Return list of zones
        //virtual ZoneList *getList() = 0;
        ///list factory
        typedef std::unique_ptr<ZoneList> ZoneListPtr;
        virtual ZoneListPtr createList() =0;

        typedef std::unique_ptr<LibFred::Zone::Manager> ZoneManagerPtr;
      /// create manager object
      static ZoneManagerPtr create();
    };//class Manager
  };//namespace Zone
};//namespace LibFred

#endif /*ZONE_H_*/

