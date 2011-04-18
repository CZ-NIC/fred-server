/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
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

/**
 * @file baseclient.h
 * Base class for all corba clients
 */

#ifndef BASE_CLIENT_H_
#define BASE_CLIENT_H_

//#include "simple.h"
#include <corba/EPP.hh>
#include <map>
#include <string>



typedef std::map<std::string, int> METHODS;
typedef std::map<std::string, int>::iterator METHODS_IT;


#define CLIENT_COMMON           0
#define CLIENT_DOMAIN           1
#define CLIENT_KEYSET           2
#define CLIENT_CONTACT          3
#define CLIENT_INVOICE          4
#define CLIENT_AUTHINFO         5
#define CLIENT_BANK             6
#define CLIENT_POLL             7
#define CLIENT_REGISTRAR        8
#define CLIENT_NOTIFY           9
#define CLIENT_OBJECT           10
#define CLIENT_INFOBUFF         11
#define CLIENT_NSSET            12
#define CLIENT_FILE             13
#define CLIENT_MAIL             14
#define CLIENT_PUBLICREQUEST    15
#define CLIENT_ENUMPARAM        16

#define TYPE_NOTYPE     0
#define TYPE_STRING     1
#define TYPE_INT        2
#define TYPE_UINT       3
#define TYPE_ULONGLONG  4
#define TYPE_BOOL       5
#define TYPE_DOUBLE     6

struct options
{
    int         client;
    const char  *name;
    const char  *description;
    int         type;
    bool        callable;
    bool        visible;
};


namespace Admin {

class BaseClient {
protected:
  std::string        m_connstring;
  std::string        m_nsAddr;

public:
  BaseClient() {
  }


  BaseClient(const std::string &_conn_string,
             const std::string &_nsaddr)
           : m_connstring(_conn_string),
             m_nsAddr(_nsaddr) {
  }

  virtual ~BaseClient() {
  }


  void init(const std::string &_conn_string,
            const std::string &_nsaddr) {
    m_connstring = _conn_string;
    m_nsAddr     = _nsaddr;
  }
  void no_help()
  {
    std::cout << "There is no help for this topic" << std::endl;
  }

  void print_options(const std::string &clientName, 
          const struct options *opts, int count)
  {
      if (count == 0) {
          std::cout << "No parameters" << std::endl;
          return;
      }
      std::string omg;
      std::cout << clientName << std::endl << "Callable parameters:" << std::endl;
      for (int i = 0; i < count; i++) {
          if (opts[i].callable) {
              if (opts[i].type != TYPE_NOTYPE) {
                  omg = opts[i].name + std::string(" arg");
              } else {
                  omg = opts[i].name;
              }
              std::ios_base::fmtflags state = std::cout.flags();
              std::cout.width(24);
              std::cout << std::left << omg << " - " << opts[i].description << std::endl;
              std::cout.flags(state);
          }
      }
      std::cout << std::endl << "Other parametrs:" << std::endl;
      for (int i = 0; i < count; i++) {
          if (!opts[i].callable) {
              if (opts[i].type != TYPE_NOTYPE) {
                  omg = opts[i].name + std::string(" arg");
              } else {
                  omg = opts[i].name;
              }
              std::ios_base::fmtflags state = std::cout.flags();
              std::cout.width(24);
              std::cout << std::left << omg;
              std::cout << " - " << opts[i].description << std::endl;
              std::cout.flags(state);
          }
      }
  }
}; // class BaseClient

} // namespace Admin


#endif /*BASE_CLIENT_H_*/

