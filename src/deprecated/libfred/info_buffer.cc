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

#include "src/deprecated/libfred/info_buffer.hh"
#include "src/deprecated/util/dbsql.hh"

namespace LibFred
{
  namespace InfoBuffer
  {
    class TempSequence {
        DBSharedPtr db;
     public:
      TempSequence(DBSharedPtr _db): db(_db)
      {
        std::stringstream sql;
        sql << "CREATE TEMPORARY SEQUENCE tmp_seq_epp_info_buffer_content";
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        sql.str("");
        if (!db->ExecSelect(
              "SELECT SETVAL('tmp_seq_epp_info_buffer_content',1, false)"
            )) throw (SQL_ERROR());
        db->FreeSelect();
      }
      ~TempSequence() {
        try {
            std::stringstream sql;
            sql << "DROP SEQUENCE tmp_seq_epp_info_buffer_content";
            if (!db->ExecSQL(sql.str().c_str())) {
#ifdef HAVE_LOGGER
                LOGGER.error("Failed to drop temporary sequence");
#endif
            }
        } catch (...) {
#ifdef HAVE_LOGGER
            LOGGER.debug("Unexpected exception in ~TempSequence");
#endif
        }
      }
    };
    /// designed to support database cursor implementation
    class ChunkImpl : virtual public Chunk
    {
      std::vector<std::string> bufferList; ///< list of records in chunk
      std::vector<std::string>::const_iterator it; ///< iterator over result
      std::string buffer; ///< actual record (because it could be empty string)
      unsigned long chunkSize; ///< result size (same as bufferList.size()
     public:
      ChunkImpl(DBSharedPtr db, TID registrar, unsigned size)
      {
        // select total number of records (count) in result and actual pointer
        // to result (current)
        std::stringstream sql;
        sql << "SELECT COUNT(DISTINCT eibc.id), MAX(eib.current) "
            << "FROM "
            << "epp_info_buffer eib, epp_info_buffer_content eibc "
            << "WHERE eib.registrar_id=eibc.registrar_id "
            << "AND eib.registrar_id=" << registrar<< " "
            << "GROUP BY eibc.registrar_id ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        unsigned long count = 0;
        unsigned long current = 0;
        if (db->GetSelectRows() == 1) {
          count = atol(db->GetFieldValue(0,0));
          if (db->IsNotNull(0,1))
            current = atol(db->GetFieldValue(0,1));
        }
        db->FreeSelect();
        /// chunk size is minimum of remaing count and specified size
        chunkSize = count - current < size ? count - current : size;
        if (!chunkSize) return;
        /// first update current point to last record fo current chunk
        sql.str("");
        sql << "UPDATE epp_info_buffer "
            << "SET current=" << current + chunkSize << " "
            << "WHERE registrar_id=" << registrar;
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        // then select chunkSize records from current
        sql.str("");
        sql << "SELECT obr.name "
            << "FROM epp_info_buffer_content eibc, object_registry obr "
            << "WHERE eibc.object_id=obr.id "
            << "AND eibc.registrar_id=" << registrar << " "
            << "AND eibc.id>" << current << " "
            << "AND eibc.id<=" << current + chunkSize
            << " ORDER BY eibc.id ASC";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (int i=0; i<db->GetSelectRows(); i++)
          bufferList.push_back(db->GetFieldValue(i,0));
        db->FreeSelect();
        it = bufferList.begin();
      }
      virtual unsigned long getCount() const
      {
        return chunkSize;
      }
      virtual const std::string& getNext()
      {
        if (it != bufferList.end()) buffer=*it++;
        else buffer = "";
        return buffer;
      }
    };
    class ManagerImpl : virtual public Manager
    {
      DBSharedPtr db;
      Domain::Manager *dm;
      Contact::Manager *cm;
      Nsset::Manager *nm;
      Keyset::Manager *km;
     public:
      ManagerImpl(
        DBSharedPtr  _db, Domain::Manager *_dm,
        Nsset::Manager *_nm, Contact::Manager *_cm, Keyset::Manager *_km
      ) : db(_db), dm(_dm), cm(_cm), nm(_nm), km(_km)
      {}


      #define LIST_DOWNCAST_AND_CHECK(_type)          \
      _type *tmp = dynamic_cast<_type* >(list.get()); \
      if (!tmp) {                                     \
        throw std::bad_cast();                        \
      }

      #define LIST_DOWNCAST_CALL(_meth) tmp->_meth;

      virtual unsigned long info(
              const std::string &registrar, Type infotype,
              const std::string &request)
         
      {
          std::stringstream query;
          query
              << "SELECT id FROM registrar WHERE handle="
              << Database::Value(registrar);
          if (!db->ExecSelect(query.str().c_str())) {
              throw SQL_ERROR();
          }
          if (db->GetSelectRows() != 1) {
              throw INVALID_REGISTRAR();
          }
          TID registrarId = atoi(db->GetFieldValue(0, 0));
          db->FreeSelect();
          return info(registrarId, infotype, request);
      }

      virtual unsigned long info(
        TID registrar, Type infotype, const std::string& request
      )
      {
        if (!registrar) throw INVALID_REGISTRAR();
        // init list of object specific temporary tables using their
        // managers
        std::unique_ptr<LibFred::ObjectList> list;
        switch (infotype) {
          case T_LIST_DOMAINS :
            {
              list.reset(dm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Domain::List)
              LIST_DOWNCAST_CALL(setRegistrarFilter(registrar))
            }
            break;

          case T_DOMAINS_BY_NSSET:
            {
              list.reset(dm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Domain::List)
              LIST_DOWNCAST_CALL(setNssetHandleFilter(request))
            }
            break;

          case T_DOMAINS_BY_CONTACT:
            {
              list.reset(dm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Domain::List)

              list->setWildcardExpansion(false);
              LIST_DOWNCAST_CALL(setRegistrantHandleFilter(request))

              list->fillTempTable(false);
              LIST_DOWNCAST_CALL(setRegistrantHandleFilter(""))
              LIST_DOWNCAST_CALL(setAdminHandleFilter(request))

              list->fillTempTable(false);
              LIST_DOWNCAST_CALL(setAdminHandleFilter(""));
              LIST_DOWNCAST_CALL(setTempHandleFilter(request))
            }
            break;

          case T_DOMAINS_BY_KEYSET:
            {
              list.reset(dm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Domain::List)
              LIST_DOWNCAST_CALL(setKeysetHandleFilter(request))
            }
            break;

          case T_LIST_NSSETS:
            {
              list.reset(nm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Nsset::List)
              LIST_DOWNCAST_CALL(setRegistrarFilter(registrar))
            }
            break;

          case T_LIST_KEYSETS:
            {
              list.reset(km->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Keyset::List)
              LIST_DOWNCAST_CALL(setRegistrarFilter(registrar))
            }
            break;

          case T_NSSETS_BY_CONTACT:
            {
              list.reset(nm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Nsset::List)
              LIST_DOWNCAST_CALL(setAdminFilter(request))
            }
            break;

          case T_KEYSETS_BY_CONTACT:
            {
              list.reset(km->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Keyset::List)
              LIST_DOWNCAST_CALL(setAdminFilter(request))
            }
            break;

          case T_NSSETS_BY_NS:
            {
              list.reset(nm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Nsset::List)
              LIST_DOWNCAST_CALL(setHostNameFilter(request))
            }
            break;

          case T_LIST_CONTACTS:
            {
              list.reset(cm->createList());
              LIST_DOWNCAST_AND_CHECK(LibFred::Contact::List)
              LIST_DOWNCAST_CALL(setRegistrarFilter(registrar))
            }
            break;
        };
        list->setWildcardExpansion(false);
        list->fillTempTable(false);
        std::stringstream sql;
        // if there is no record for given registrar in
        // epp_info_buffer table create one and set current=NULL
        sql << "INSERT INTO epp_info_buffer "
            << "SELECT r.id, NULL "
            << "FROM registrar r "
            << "LEFT JOIN epp_info_buffer eib ON (r.id=eib.registrar_id) "
            << "WHERE eib.registrar_id ISNULL AND r.id=" << registrar;
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        // maybe registrar was already there so update existing record
        // to current=NULL (must be done before DELETE of content because of
        // foreign key to content table)
        sql.str("");
        sql << "UPDATE epp_info_buffer SET current=NULL "
            << "WHERE registrar_id=" << registrar;
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        // delete content table for this registrar
        sql.str("");
        sql << "DELETE FROM epp_info_buffer_content "
            << "WHERE registrar_id=" << registrar;
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        // create temporary sequence for numbering of result.
        // numbering is then used in streaming download.
        // it should disappier at the and of block
        TempSequence seq(db);
        // copy content of temporary table into result table
        sql.str("");
        sql << "INSERT INTO epp_info_buffer_content "
            << "SELECT NEXTVAL('tmp_seq_epp_info_buffer_content'), "
            << registrar << ", t.id "
            << "FROM "
            << "(SELECT tmp.id FROM " << list->getTempTableName() << " tmp, "
            << "object_registry obr "
            << "WHERE tmp.id=obr.id ORDER BY obr.name ASC) t";
        if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();
        // count result
        sql.str("");
        sql << "SELECT COUNT(*) FROM epp_info_buffer_content "
            << "WHERE registrar_id=" << registrar;
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        unsigned long result = 0;
        if (db->GetSelectRows() == 1)
          result = atol(db->GetFieldValue(0,0));
        db->FreeSelect();
        return result;
      }
      virtual Chunk *getChunk(const std::string &registrar, unsigned int size)
         
      {
          std::stringstream query;
          query
              << "SELECT id FROM registrar WHERE handle="
              << Database::Value(registrar);
          if (!db->ExecSelect(query.str().c_str())) {
              throw SQL_ERROR();
          }
          if (db->GetSelectRows() != 1) {
              throw INVALID_REGISTRAR();
          }
          TID registrarId = atoi(db->GetFieldValue(0, 0));
          db->FreeSelect();
          return getChunk(registrarId, size);
      }

      virtual Chunk *getChunk(TID registrar, unsigned size)
       
      {
        if (!registrar) throw INVALID_REGISTRAR();
        return new ChunkImpl(db,registrar,size);
      }
    };
    Manager *Manager::create(
        DBSharedPtr db, Domain::Manager *dm, Nsset::Manager *nm, Contact::Manager *cm, Keyset::Manager *km
    )
    {
      return new ManagerImpl(db,dm,nm,cm, km);
    }
  };
};
