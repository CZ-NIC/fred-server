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


namespace Admin {


class BaseClient {
protected:
  std::string        m_connstring;
  std::string        m_nsAddr;
  Database::Manager *m_dbman;


public:
  BaseClient() : m_dbman(0) {
  }


  BaseClient(const std::string &_conn_string,
             const std::string &_nsaddr)
           : m_connstring(_conn_string),
             m_nsAddr(_nsaddr),
             m_dbman(new Database::Manager(new ConnectionFactory(_conn_string))) {
  }


  virtual ~BaseClient() {
    if (m_dbman) {
      delete m_dbman;
    }
  }


  void init(const std::string &_conn_string,
            const std::string &_nsaddr) {
    m_connstring = _conn_string;
    m_nsAddr     = _nsaddr;
    m_dbman      = new Database::Manager(new ConnectionFactory(m_connstring));
  }
  void no_help()
  {
    std::cout << "There is no help for this topic" << std::endl;
  }
};


}


#endif /*BASE_CLIENT_H_*/

