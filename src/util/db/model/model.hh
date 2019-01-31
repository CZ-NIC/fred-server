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

#ifndef MODEL_HH_259EE9F5F8C8484C9B2735CF642C00D1
#define MODEL_HH_259EE9F5F8C8484C9B2735CF642C00D1

#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/foreach.hpp>
#include <sstream>

#include "src/util/db/model/model_field.hh"
//#include "model_field_related.h"
#include "src/util/db/model/model_field_attrs.hh"
#include "src/util/db/model/model_field_macros.hh"
#include "src/util/db/model/model_field_list.hh"
#include "src/util/db/model/job_queue.hh"

#include "src/util/db/query/query.hh"

#include "config.h"
#ifdef HAVE_LOGGER
#include "util/log/logger.hh"
#endif

using namespace boost::assign;


namespace Model {


class Base {
public:
  /**
   * Insert operation
   * insert all data to database if constrained criteria are ok and load primary key
   * value of this new record
   *
   * @param _object  instance pointer for data to insert
   */
  template<class _class> 
  void insert(_class *_object) {
#ifdef HAVE_LOGGER
    try {
#endif
      /* handle inherited models when is needed to insert only child for
       * existing parent - (test primary key is set and has attribute
       * default (works only for sequences) */
      typename _class::field_list list = _object->getFields();
      Field::PrimaryKey<_class, sequence_type> *pk = list.template getPrimaryKey<sequence_type>();
      if (pk && pk->getAttrs().isDefault() && pk->getField(_object).isChanged()) {
#ifdef HAVE_LOGGER
          LOGGER.debug("PK is set; data was already inserted (insert aborted)");
#endif
          return;
      }

      JobQueue jobs;
      Database::InsertQuery *iquery = new Database::InsertQuery(_class::table_name);

      jobs.push(iquery);

      BOOST_FOREACH(typename Field::List<_class>::value_type _field, _object->getFields()) {
        _field->serialize(jobs, *iquery, _object);
      }

      if (!iquery->empty()) {
        jobs.execute();
        this->reloadPrimaryKey_(_object);

        BOOST_FOREACH(typename Field::List<_class>::value_type _field, _object->getFields()) {
          _field->markSerialized(_object);
        }
      }
#ifdef HAVE_LOGGER 
    }
    catch (Model::Exception &_err) {
      LOGGER.alert(_err.what());
      throw;
    }
    catch (Database::Exception &_err) {
      LOGGER.error(_err.what());
      throw;
    }
    catch (...) {
      LOGGER.info("Unknown error");
      throw;
    }
#endif
  }


  /**
   * Update operation
   * (where part of query is always generated from primary key so it should be set)
   *
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

      if (!uquery->empty()) {
        jobs.execute();

        BOOST_FOREACH(typename Field::List<_class>::value_type _field, _object->getFields()) {
          _field->markSerialized(_object);
        }
      }
#ifdef HAVE_LOGGER
    }
    catch (Model::Exception &_err) {
      LOGGER.alert(_err.what());
      throw;
    }
    catch (Database::Exception &_err) {
      LOGGER.error(_err.what());
      throw;
    }
    catch (...) {
      LOGGER.error("Unknown error");
      throw;
    }
#endif
  }


  /**
   * Remove operation
   * (where part of query is always generated from primary key so it should be set)
   *
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
      LOGGER.error(_err.what());
      throw;
    }
    catch (Database::Exception &_err) {
      LOGGER.error(_err.what());
      throw;
    }
    catch (...) {
      LOGGER.error("Unknown error");
      throw;
    }
#endif
  }


  /**
   * Request reload data from database to synchronized it with object
   *
   * @param _object  instance pointer for data to load
   */
  template<class _class>
  void reload(_class *_object) {
    try {
      this->reloadByPrimaryKey_(_object);
    }
    catch (Database::NoDataFound &_err) {
      LOGGER.info(_err.what());
      throw;
    }
    catch (Database::Exception &_err) {
      LOGGER.error(_err.what());
      throw;
    }
  }

  
  /**
   * Dump all fields and its values to simple string
   * (can be used for logging and debugging)
   *
   * @param _object  instance pointer to data to dump
   * @return         stringized object data
   */
  template<class _class>
  std::string toString(const _class *_object) const {
    std::stringstream out;
    
    out << _class::table_name << " {";
    BOOST_FOREACH(typename Field::List<_class>::value_type _field, _object->getFields()) {
      out << " ";
      _field->serialize(out, _object);
    }
    out << " } ";
    
    return out.str();
  }


  /**
   * TEST: get table method for support table name generation
   *       by overriding in concrete model method specialization
   *       (should be usefull for table partitioning)
   */
  template<class _class>
  static std::string getTableName() {
    return _class::table_name;
  }


  /**
   * TEST: load by result
   */
  template<class _class>
  void load(_class *_object, const Database::Row &_data) {
    this->load__(_object, _data, _class::getFields());
  }



private:
  typedef unsigned long long    sequence_type; /**< type used for database sequence */


  template<class _class>
  void reloadPrimaryKey_(_class *_object) {
    typename _class::field_list list = _object->getFields();
    try {
      Field::PrimaryKey<_class, sequence_type> *pk = list.template getPrimaryKey<sequence_type>();
      if (pk)
        this->reloadCurrentSequence_(_object);
    }
    catch (Model::Exception &_err) {
      /* PK is not a sequence, PK should be already set */
      LOGGER.debug(boost::format("PK is not a sequence type: %1%") % _err.what());
    }
  }


  /**
   * Prepare query for reload current sequence id
   * (for after insert reload)
   *
   * @param  _object  pointer to object receiving primary key value of new record
   */
  template<class _class>
  void reloadCurrentSequence_(_class *_object) {
    typename _class::field_list list = _class::getFields();
    Field::PrimaryKey<_class, sequence_type> *pk = list.template getPrimaryKey<sequence_type>();

    if (!pk->getField(_object).isChanged() && pk->getAttrs().isDefault()) {
      std::string pk_table = pk->getTableName();
      std::string pk_field = pk->getName();

      std::string query = (boost::format("SELECT currval('%1%_%2%_seq') AS %3%")
                                       % pk_table % pk_field % pk_field).str();

      typename _class::field_list tmp;
      tmp.push_back(pk);
      this->reload__(_object, query, tmp);
    }
  }


  /**
   * Prepare query for reload by current sequence - the last added record
   * (for after insert reload)
   *
   * @param  _object  pointer to object receiving database data
   */
  template<class _class>
  void reloadByCurrentSequence_(_class *_object) {
    typename _class::field_list list = _class::getFields();
    Field::PrimaryKey<_class, sequence_type> *pk = list.template getPrimaryKey<sequence_type>();

    if (!pk->getField(_object).isChanged() && pk->getAttrs().isDefault()) {
      std::string pk_table = pk->getTableName();
      std::string pk_field = pk->getName();

      std::string query = (boost::format("SELECT * FROM %1% WHERE %2% = (SELECT currval('%1%_%2%_seq'))")
                                       % pk_table % pk_field).str();

      this->reload__(_object, query, list);
    }
  }


  /**
   * Prepare query for reload by primary key value
   * (for after update reload)
   *
   * @param  _object  pointer to object receiving database data
   */
  template<class _class>
  void reloadByPrimaryKey_(_class *_object) {
    typename _class::field_list list = _class::getFields();
    Field::PrimaryKey<_class, sequence_type> *pk = list.template getPrimaryKey<sequence_type>();

    if (pk->getField(_object).isChanged() || !pk->getAttrs().isDefault()) {
      std::string pk_table = pk->getTableName();
      std::string pk_field = pk->getName();

      std::string query = (boost::format("SELECT * FROM %1% WHERE %2% = %3%")
                                       % pk_table % pk_field % pk->getValue(_object)).str();

      this->reload__(_object, query, list);
    }
  }


  /**
   * Load data to object through executing query
   *
   * @param  _object  pointer to object receiving database data
   * @param  _query   select query
   * @param  _fields  model field list (what we expect to load from query)
   *
   * TODO: check number of columns and number of fields - should be same
   */
  template<class _class>
  void reload__(_class *_object, const std::string &_query, const typename _class::field_list &_fields) {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result data = conn.exec(_query);

      if (data.size() != 1) {
         throw Database::NoDataFound(_query);
      }

      load__(_object, *(data.begin()), _fields);
  }


  /**
   * Load data from database row object
   *
   * @param  _object  pointer to object receiving database data
   * @param  _data    row data
   * @param  _fields  model fields list (what we search for in row data)
   *
   */
  template<class _class>
  void load__(_class *_object, const Database::Row &_data, const typename _class::field_list &_fields)
  {
    try {
      BOOST_FOREACH(typename _class::field_list::value_type field, _fields) {
        /* it is loaded to database so reset the is_set flag (false parameter) */
        bool set_flag = false;
        if (field->getAttrs().isPrimaryKey())
          set_flag = true;

        Database::Value v = _data[field->getName()];
        if (!v.isnull())
          field->setValue(_object, v, set_flag);
      }
    }
    catch (Database::NoSuchField &_err) {
      throw Model::DataLoadError(str(boost::format("can't load all data from Database::Row (%1% in row data)") 
                                 % _err.what()));
    }
    catch (Database::Exception &_err) {
      throw Model::DataLoadError(str(boost::format("database error (%1%)") % _err.what()));
    }
    catch (...) {
      throw Model::DataLoadError("unknown error");
    }

  }
};


}


#endif /*MODEL_H_*/

