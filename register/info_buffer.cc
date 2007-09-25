#include "info_buffer.h"
#include "dbsql.h"

namespace Register 
{
  namespace InfoBuffer
  {
    /// designed to support database cursor implementation
    class ChunkImpl : virtual public Chunk
    {
      std::vector<std::string> bufferList; ///< list of records in chunk
      std::vector<std::string>::const_iterator it; ///< iterator over result
      std::string buffer; ///< actual record (because it could be empty string)
      unsigned long chunkSize; ///< result size (same as bufferList.size()
     public:
      ChunkImpl(DB *db, TID registrar, unsigned size) throw (SQL_ERROR)
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
      DB *db;
      Domain::Manager *dm;
      Contact::Manager *cm;
      NSSet::Manager *nm;
     public:
      ManagerImpl(
        DB *_db, Domain::Manager *_dm, 
        NSSet::Manager *_nm, Contact::Manager *_cm
      ) : db(_db), dm(_dm), cm(_cm), nm(_nm)
      {}
      #define LIST(type) (dynamic_cast<type *>(list.get()))
      virtual unsigned long info(
        TID registrar, Type infotype, const std::string& request
      ) throw (SQL_ERROR, INVALID_REGISTRAR)
      {
    	if (!registrar) throw INVALID_REGISTRAR();
        // init list of object specific temporary tables using their
        // managers
        std::auto_ptr<Register::ObjectList> list;        
        switch (infotype) {
          case T_LIST_DOMAINS :
            list.reset(dm->createList());
            LIST(Register::Domain::List)->setRegistrarFilter(registrar);
            break;
          case T_DOMAINS_BY_NSSET:
            list.reset(dm->createList());
            LIST(Register::Domain::List)->setNSSetHandleFilter(request);
            break;
          case T_DOMAINS_BY_CONTACT:
            list.reset(dm->createList());
            list->setWildcardExpansion(false);
            LIST(Register::Domain::List)->setRegistrantHandleFilter(request);
            list->fillTempTable(false);
            LIST(Register::Domain::List)->setRegistrantHandleFilter("");
            LIST(Register::Domain::List)->setAdminHandleFilter(request);
            list->fillTempTable(false);
            LIST(Register::Domain::List)->setAdminHandleFilter("");
            LIST(Register::Domain::List)->setTempHandleFilter(request);
            break;
          case T_LIST_NSSETS:
            list.reset(nm->createList());
            LIST(Register::NSSet::List)->setRegistrarFilter(registrar);
            break;
          case T_NSSETS_BY_CONTACT:
            list.reset(nm->createList());
            LIST(Register::NSSet::List)->setAdminFilter(request);
            break;
          case T_NSSETS_BY_NS:
            list.reset(nm->createList());
            LIST(Register::NSSet::List)->setHostNameFilter(request);
            break;
          case T_LIST_CONTACTS:
            list.reset(cm->createList());
            LIST(Register::Contact::List)->setRegistrarFilter(registrar);
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
        // this query should fail only if sequence exists which don't mind  
        db->ExecSQL(
          "CREATE TEMPORARY SEQUENCE tmp_seq_epp_info_buffer_content"
        );
        // for sure clear sequence (it could already exist)
        if (!db->ExecSelect(
              "SELECT SETVAL('tmp_seq_epp_info_buffer_content',1, false)"
            )) throw (SQL_ERROR());
        db->FreeSelect();
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
      virtual Chunk *getChunk(TID registrar, unsigned size)
        throw (SQL_ERROR, INVALID_REGISTRAR)
      {
        if (!registrar) throw INVALID_REGISTRAR();
        return new ChunkImpl(db,registrar,size); 
      }
    };
    Manager *Manager::create(
      DB *db, Domain::Manager *dm, NSSet::Manager *nm, Contact::Manager *cm
    )
    {
      return new ManagerImpl(db,dm,nm,cm);
    }
  };
};
