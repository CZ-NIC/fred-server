/*
 *  Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

#include <vector>
#include <algorithm>
#include <functional>
#include "zone.h"
#include "dbsql.h"
#include <iostream>
#include "types.h"
#include <ctype.h>

#define RANGE(x) x.begin(),x.end()
#define IS_NUMBER(x) (x >= '0' && x <= '9')
#define IS_LETTER(x) ((x >= 'a' && x <= 'z') || (x >= 'A' && x <= 'Z'))

namespace Register
{
  namespace Zone
  {
    struct ZoneImpl : public virtual Zone {
      ZoneImpl(
        TID _id, const std::string& _fqdn, bool _isEnum, 
        unsigned _maxLevel
      ) :
        id(_id), fqdn(_fqdn), isEnum(_isEnum), maxLevel(_maxLevel)
      {}
      ~ZoneImpl() {}
      TID id;
      std::string fqdn;
      bool isEnum;
      unsigned maxLevel; ///< Maximal domain level in this zone
      /// compare if domain belongs to this zone (according to suffix)
      bool operator==(const std::string& domain) const
      {
        std::string copy = domain;
        for (unsigned i=0; i<copy.length(); i++)
          copy[i] = tolower(copy[i]);
        unsigned l = fqdn.length();
        if (copy.length() < l) return false;
        return copy.compare(copy.length()-l,l,fqdn) == 0;  
      }
      /// interface implementation
      const std::string& getFqdn() const 
      {
        return fqdn; 
      }
      /// interface implementation
      bool isEnumZone() const 
      {
        return isEnum; 
      }
      /// interface implementation
      virtual unsigned getMaxLevel() const
      {
        return maxLevel;
      }                 
    };
    typedef std::vector<ZoneImpl> ZoneList;
    class ManagerImpl : virtual public Manager
    {
      ZoneList zoneList;
      DB *db;
      std::string defaultEnumSuffix;
      std::string enumZoneString;
      std::string defaultDomainSuffix;
      bool loaded;
     public:
      ManagerImpl(DB *_db) :
       db(_db), 
       defaultEnumSuffix("0.2.4"),
       enumZoneString("e164.arpa"),
       defaultDomainSuffix("cz"),
       loaded(false) 
      {
      }
      void load()
      {
        if (!db->ExecSelect(
          "SELECT id, fqdn, enum_zone, "
          "ARRAY_UPPER(STRING_TO_ARRAY(fqdn,'.'),1) + dots_max "
          "FROM zone ORDER BY LENGTH(fqdn) DESC"
        )) return;
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          zoneList.push_back(ZoneImpl(
           STR_TO_ID(db->GetFieldValue(i,0)),
           db->GetFieldValue(i,1),
           *db->GetFieldValue(i,2) == 't' ? true : false,
           atoi(db->GetFieldValue(i,3))
           ));
        }
        db->FreeSelect();
        loaded = true;
      }
      /// interface method implementation  
      void parseDomainName(const std::string& fqdn, DomainName& domain) 
       const throw (INVALID_DOMAIN_NAME)
      {
        std::string part; // one part(label) of fqdn  
        for (unsigned i=0; i<fqdn.size(); i++) {
          if (part.empty()) {
            // first character of every label has to be letter or digit
            // digit is not in RFC 1034 but it's required in ENUM domains
            if (!IS_NUMBER(fqdn[i]) && !IS_LETTER(fqdn[i]))
              throw INVALID_DOMAIN_NAME();
          }
          else {
            // dot '.' is a separator of labels, store and clear part
            if (fqdn[i] == '.') {
              // part must finish with letter or digit
              if (!IS_NUMBER(*part.rbegin()) && !IS_LETTER(*part.rbegin()))
                throw INVALID_DOMAIN_NAME();
              // length of part should be < 64
              if (part.length() > 63)
                throw INVALID_DOMAIN_NAME();
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
        // enum domains has special rules
        if (checkEnumDomainSuffix(fqdn) && !checkEnumDomainName(domain))
          throw INVALID_DOMAIN_NAME();
      }
      /// interface method implementation
      bool checkEnumDomainName(DomainName& domain) const
      {
        // must have suffix e164.org
        if (domain.size()<=2) return false;
        // every part of domain name except of suffix must be one digit
        for (unsigned i=0; i<domain.size()-2; i++)
          if (domain[i].length() != 1 || !IS_NUMBER(domain[i][0]))
            return false;
        return true;
      }
      /// check if suffix is e164.arpa
      bool checkEnumDomainSuffix(const std::string& fqdn) const
      {
        const std::string& esuf = getEnumZoneString();
        // check if substring 'esuf' found from right
        // is last substring in fqdn
        std::string::size_type i = fqdn.rfind(esuf);
        return  i != std::string::npos && i + esuf.size() == fqdn.size();
      }
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
        if (!last) {
          result += '.';
          result += getDefaultEnumSuffix();
        }
        // append enum domain zone
        result += '.';
        result += getEnumZoneString(); 
        return result;
      }      
      const std::string& getDefaultEnumSuffix() const
      {
        return defaultEnumSuffix;
      }
      const std::string& getDefaultDomainSuffix() const
      {
        return defaultDomainSuffix;
      }
      const std::string& getEnumZoneString() const
      {
        return enumZoneString; 
      }
      const Zone* findZoneId(const std::string& fqdn) const
      {
        // nonconst casting for lazy initialization
        if (!loaded) ((Register::Zone::ManagerImpl *)this)->load();
        ZoneList::const_iterator i = find(RANGE(zoneList),fqdn);
        if (i!=zoneList.end()) return &(*i);
        else return NULL;
      }
    };
    Manager* Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }
  };
};

