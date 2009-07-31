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

#include "simple.h"

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
              std::cout.width(24);
              std::cout << std::left << omg << " - " << opts[i].description << std::endl;
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
              std::cout.width(24);
              std::cout << std::left << omg;
              std::cout << " - " << opts[i].description << std::endl;
          }
      }
  }
}; // class BaseClient

} // namespace Admin


#endif /*BASE_CLIENT_H_*/

