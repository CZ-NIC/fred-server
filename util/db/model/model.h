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

#include "model_field.h"
#include "model_field_attrs.h"
#include "job_queue.h"

#include "db/database.h"
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

      BOOST_FOREACH(Field::Base_<_class> *_field, _object->getFields()) {
        _field->serialize(jobs, *iquery, _object);
      }

      jobs.execute();
#ifdef HAVE_LOGGER 
    }
    catch (Field::SerializationError &_err) {
      LOGGER(PACKAGE).error(_err.what());
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

      BOOST_FOREACH(Field::Base_<_class> *_field, _object->getFields()) {
        _field->serialize(jobs, *uquery, _object);
      }

      jobs.execute();
#ifdef HAVE_LOGGER
    }
    catch (Field::SerializationError &_err) {
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
    catch (Field::SerializationError &_err) {
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
};


}


#endif /*MODEL_H_*/

