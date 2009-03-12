/*  
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
 * 
 * This file is part of FRED.
 * 
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file model_exception.h
 *  Exception classes
 */

#ifndef MODEL_EXCEPTION_H_
#define MODEL_EXCEPTION_H_

#include "base_exception.h"


namespace Model {


class Exception : public ::Exception {
public:
  Exception(const std::string &_what) : ::Exception(_what) { }
};


class DefinitionError : public Exception {
public:
  DefinitionError(const std::string &_table,
                  const std::string &_problem)
                : Exception("Model::Error definition: table=`" + _table + "' " + _problem) {
  }
};


class DataLoadError : public Exception {
public:
  DataLoadError(const std::string &_reason)
              : Exception("Model::DataLoadError: reason=`" + _reason +"'") {
  }
};


namespace Field {


class SerializationError : public Model::Exception {
public:
  SerializationError(const std::string &_qtype,
                     const std::string &_tname,
                     const std::string &_fname,
                     const std::string &_detail)
                   : Model::Exception("Model::Field serialization error:") {
    what_ += " query_type=`" + _qtype + "' " +
             " table_name=`" + _tname + "' " +
             " field_name=`" + _fname + "' " +
             " details=`" + _detail + "'";
  }
};


}
}


#endif /*MODEL_EXCEPTION_H_*/

