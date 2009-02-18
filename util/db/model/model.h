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
 *  @file model.h
 *  \class Model 
 *  \brief interface for database operations
 *
 *  Base class for objects that should be capable of database operations
 *  - insert, update, remove
 */

#ifndef MODEL_H_
#define MODEL_H_

#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <sstream>

#include "model_field.h"
#include "model_field_related.h"
#include "model_field_attrs.h"
#include "model_field_macros.h"
#include "model_field_list.h"
#include "job_queue.h"

#include "db/query/query.h"

#include "config.h"
#ifdef HAVE_LOGGER
#include "log/logger.h"
#endif

using namespace boost::assign;


namespace Model {


class Base {
public:
  /**
   * Insert operation
   * @param _object  instance pointer for data to insert
   */
  template<class _class> 
  void insert(_class *_object) {
#ifdef HAVE_LOGGER
    try {
#endif
      JobQueue jobs;
      Database::InsertQuery *iquery = new Database::InsertQuery(_class::table_name);

      jobs.push(iquery);

      BOOST_FOREACH(typename Field::List<_class>::value_type _field, _object->getFields()) {
        _field->serialize(jobs, *iquery, _object);
      }

      jobs.execute();
      this->reloadByCurrentSequence_(_object);
#ifdef HAVE_LOGGER 
    }
    catch (Model::Exception &_err) {
      LOGGER(PACKAGE).alert(_err.what());
      throw;
    }
    catch (Database::Exception &_err) {
      LOGGER(PACKAGE).error(_err.what());
      throw;
    }
    catch (...) {
      LOGGER(PACKAGE).info("Unknown error");
      throw;
    }
#endif
  }


  /**
   * Update operation
   * (where part of query is always generated from primary key so it should be set)
   * @param _object  instance pointer for data to update
   */
  template<class _class>
  void update(_class *_object) {
#ifdef HAVE_LOGGER
    try {
#endif
      JobQueue jobs;
      Database::UpdateQuery *uquery = new Database::UpdateQuery(_class::table_name);

      jobs.push(uquery);

      BOOST_FOREACH(typename Field::List<_class>::value_type _field, _object->getFields()) {
        _field->serialize(jobs, *uquery, _object);
      }

      jobs.execute();
      this->reloadByCurrentSequence_(_object);
#ifdef HAVE_LOGGER
    }
    catch (Model::Exception &_err) {
      LOGGER(PACKAGE).alert(_err.what());
      throw;
    }
    catch (Database::Exception &_err) {
      LOGGER(PACKAGE).error(_err.what());
      throw;
    }
    catch (...) {
      LOGGER(PACKAGE).error("Unknown error");
      throw;
    }
#endif
  }


  /**
   * Remove operation
   * (where part of query is always generated from primary key so it should be set)
   * @param _object  instance pointer for data to remove
   */
  template<class _class>
  void remove(_class *_data) {
#ifdef HAVE_LOGGER
    try {
#endif

#ifdef HAVE_LOGGER
    }
    catch (Model::Exception &_err) {
      LOGGER(PACKAGE).error(_err.what());
      throw;
    }
    catch (Database::Exception &_err) {
      LOGGER(PACKAGE).error(_err.what());
      throw;
    }
    catch (...) {
      LOGGER(PACKAGE).error("Unknown error");
      throw;
    }
#endif
  }


  /**
   * TEST: load object by PK value
   */
  template<class _class>
  void load(_class *_object, const Database::Value &_value) {
    try {
      typedef typename Field::List<_class>::value_type  field_type;

      field_type pk_field;
      BOOST_FOREACH(pk_field, _object->getFields()) {
        if (pk_field->getAttrs().isPrimaryKey())
          break;
      }

      std::stringstream query;
      query << "SELECT * FROM " << pk_field->getTableName() << " WHERE "
            << pk_field->getName() << " = " << static_cast<std::string>(_value);
                    

      Database::Connection conn = Database::Manager::acquire();
      Database::Result r = conn.exec(query.str());
      
      if (r.size() == 1) {
        unsigned i = 0;
        BOOST_FOREACH(Field::Base_<_class> *_field, _object->getFields()) {
          Database::Value v = r[0][i++];
          if (!v.isnull()) {
            _field->setValue(_object, v);
          }
        }
      }
    }
    catch (Database::Exception &_err) {
      LOGGER(PACKAGE).error(_err.what());
      throw;
    }
  }


private:
  template<class _class>
  void reloadByCurrentSequence_(_class *_object) {
    try {
      typedef unsigned long long                        seq_type;
      typedef typename Field::List<_class>::value_type  field_type;

      std::string query = "SELECT * FROM %1% WHERE %2% = %3%";
      Field::PrimaryKey<_class, seq_type> *pk = 0;

      BOOST_FOREACH(field_type field, _object->getFields()) {
        if (field->getAttrs().isPrimaryKey()) {
          pk = dynamic_cast<Field::PrimaryKey<_class, seq_type>* >(field);
          if (!pk) {
            throw Model::DefinitionError(_class::table_name + "::" + field->getName(), "data type of primary key field doesn't fit");
          }
          break;
        }
      }

      if (pk == 0) {
        /* primary key not found in list */
        throw Model::DefinitionError(_class::table_name, "can't find primary key field");
      }

      if (!pk->getField(_object).isSet() && pk->getAttrs().isDefault()) {
        /* reload data with current sequence */
        std::string subquery = "(SELECT currval('" 
                             + pk->getTableName() + "_" + pk->getName() + "_seq"
                             + "'))";
        query = (boost::format(query) % pk->getTableName() 
                                      % pk->getName() 
                                      % subquery).str();
      }
      else {
        /* should have PK loaded */
        query = (boost::format(query) % pk->getTableName() 
                                      % pk->getName() 
                                      % pk->getValue(_object)).str();
      }

      Database::Connection conn = Database::Manager::acquire();
      Database::Result r = conn.exec(query);
      
      if (r.size() == 1) {
        unsigned i = 0;
        BOOST_FOREACH(Field::Base_<_class> *field, _object->getFields()) {
          Database::Value v = r[0][i++];
          if (!v.isnull()) {
            field->setValue(_object, v);
          }
        }
      }
    }
    catch (Database::Exception &_err) {
      LOGGER(PACKAGE).error(_err.what());
      throw;
    }
  }
};


}


#endif /*MODEL_H_*/

