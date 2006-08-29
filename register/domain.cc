#include <sstream>
#include <memory>
#include "domain.h"
#include "blacklist.h"
#include "dbsql.h"

#define IS_NUMBER(x) (x >= '0' && x <= '9')
#define IS_LETTER(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))
#define NUMLEN 9

namespace Register
{
  namespace Domain
  {
    class ManagerImpl : virtual public Manager
    {
      DB *db; ///< connection do db
      Zone::Manager *zm; ///< zone management api
      std::auto_ptr<Blacklist> blacklist; ///< black list manager
     public:
      ManagerImpl(DB *_db, Zone::Manager *_zm) :
       db(_db), zm(_zm), blacklist(Blacklist::create(_db))
      {}
      /// interface method implementation  
      std::string makeEnumDomain(const std::string& number)
       const throw (NOT_A_NUMBER)
      {
        std::string result;
        unsigned l = number.size();
        if (!l) throw NOT_A_NUMBER();
        // where to stop in backward processing
        unsigned last = 0;
        if (!number.compare(0,2,"00")) last = 2;
        else if (number[0] == '+') last = 1;
        // process string
        for (unsigned i=l-1; i>last; i--) {
          if (!IS_NUMBER(number[i])) throw NOT_A_NUMBER();
          result += number[i];
          result += '.';
        }
        if (!IS_NUMBER(number[last])) throw NOT_A_NUMBER();
        result += number[last];
        // append default country code if short
        if (l-last <= NUMLEN) {
          result += '.';
          result += zm->getDefaultEnumSuffix();
        }
        // append enum domain zone
        result += '.';
        result += zm->getEnumZoneString(); 
        return result;
      }
      /// interface method implementation  
      void parseDomainName(const std::string& fqdn, DomainName& domain) 
       const throw (INVALID_DOMAIN_NAME)
      {
        std::string part;
        for (unsigned i=0; i<fqdn.size(); i++) {
          if (part.empty()) {
            // first character of every label has to be letter or digit
            if (!IS_NUMBER(fqdn[i]) && !IS_LETTER(fqdn[i]))
              throw INVALID_DOMAIN_NAME();
          }
          else {
            // dot '.' is a separator of labels, store and clear part
            if (fqdn[i] == '.') {
              domain.push_back(part);
              part.clear();
              continue;
            } 
            else {
              if (fqdn[i] == '-') {
                // dash '-' is acceptable only if last character wasn't dash
                if (part[part.length()-1] == '-') throw INVALID_DOMAIN_NAME();
              } 
              else {
                // other character could be only number or letter
                if (!IS_NUMBER(fqdn[i]) && !IS_LETTER(fqdn[i]))
                  throw INVALID_DOMAIN_NAME();
              }
            }
          }
          // add character into part
          part += fqdn[i];
        }
        // last part cannot be empty
        if (part.empty()) throw INVALID_DOMAIN_NAME();
        // append last part
        domain.push_back(part);
      }
      /// interface method implementation  
      CheckAvailType checkAvail(const std::string& fqdn) const 
        throw (SQL_ERROR)
      {
        DomainName domain;
        try { parseDomainName(fqdn,domain); }
        catch (INVALID_DOMAIN_NAME) { return CA_INVALID_HANDLE; }
        if (!zm->findZoneId(fqdn)) return CA_BAD_ZONE;
        if (blacklist->checkDomain(fqdn)) return CA_BLACKLIST;
        std::stringstream sql;
        CheckAvailType ret = CA_AVAILABLE;
        // domain cannot be subdomain or parent domain of registred domain
        sql << "SELECT fqdn FROM domain WHERE "
            << "('" << fqdn << "' LIKE '%'||fqdn) OR "
            << "(fqdn LIKE '%'||'" << fqdn << "')";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        if (db->GetSelectRows() == 1) {
          std::string fqdnLoaded = db->GetFieldValue(0,0);
          if (fqdn == fqdnLoaded) ret = CA_REGISTRED;
          else ret = CA_PARENT_REGISTRED;  
        }
        db->FreeSelect();
        return ret;        
      } 
    };
    Manager *Manager::create(DB *db, Zone::Manager *zm)
    {
      return new ManagerImpl(db,zm);
    }
  };
};
