#include "domain.h"
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
      Zone::Manager *zm; ///< zone management api
      DB *db; ///< connection do db
     public:
      ManagerImpl(DB *_db, Zone::Manager *_zm) :
       db(_db), zm(_zm)
      {}
      std::string makeEnumDomain(const std::string& number)
       throw (NOT_A_NUMBER)
      {
        // +420385514407
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
        if (l-last == NUMLEN)
          result += zm->getDefaultEnumSuffix();
        // append enum domain zone
        result += zm->getEnumZoneString(); 
        return result;
      }
      void parseDomainName(const std::string& fqdn, DomainName& domain) 
       throw (INVALID_DOMAIN_NAME)
      {
        std::string part;
        for (unsigned i=0; i<fqdn.size(); i++) {
          if (part.empty()) {
            // first character of every label has to be letter
            if (!IS_LETTER(fqdn[i])) throw INVALID_DOMAIN_NAME();
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
    };
    Manager *Manager::create(DB *db, Zone::Manager *zm)
    {
      return new ManagerImpl(db,zm);
    }
  };
};
