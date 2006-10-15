#include "registrar.h"
#include "dbsql.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <vector>
#include <algorithm>
#include <functional>

#define SET(a,b) { a = b; changed = true; }

namespace Register
{
  namespace Registrar
  {
    class RegistrarImpl;
    class ACLImpl : virtual public ACL
    {
      unsigned id;
      std::string certificateMD5;
      std::string password;
      bool changed;
      friend class RegistrarImpl;
     public:
      ACLImpl() : id(0), changed(true)
      {
      }
      ACLImpl(
        unsigned _id, 
        const std::string _certificateMD5,
        const std::string _password
      ) : id(_id), certificateMD5(_certificateMD5),
          password(_password),changed(false)
      {
      }
      virtual const std::string& getCertificateMD5() const
      {
        return certificateMD5;
      }
      virtual void setCertificateMD5(const std::string& newCertificateMD5)
      {
        SET(certificateMD5,newCertificateMD5);
      }       
      virtual const std::string& getPassword() const
      {
        return password;
      }
      virtual void setPassword(const std::string& newPassword)
      {
        SET(password,newPassword);
      }
      bool operator==(unsigned _id)
      {
        return id == _id;
      }      
      bool hasChanged() const
      {
              return changed;
      }
      std::string makeSQL(unsigned registrarId)
      {
        std::ostringstream sql;
        sql << "INSERT INTO registraracl (registrarid,cert,password) VALUES "
            << "(" << registrarId << "," << certificateMD5 << "," 
            << password << ");";
        return sql.str();
      }
    };

    class RegistrarImpl : virtual public Registrar 
    {
      typedef std::vector<ACLImpl *> ACLList;
      typedef ACLList::iterator ACLListIter;
      DB *db; ///< db connection
      unsigned id; ///< DB: registrar.id
      std::string handle; ///< DB: registrar.handle
      std::string name; ///< DB: registrar.name
      std::string url; ///< DB: registrar.name
      std::string password; ///< DB: registraracl.passwd
      bool changed; ///< object was changed, need sync to database
      ACLList acl; ///< access control
     public:
      RegistrarImpl(DB *_db) :  db(_db), id(0), changed(true)
      {}
      RegistrarImpl(DB *_db,
        unsigned _id, const std::string& _handle, const std::string& _name,
        const std::string& _url
      ) : db(_db), id(_id), handle(_handle), name(_name), url(_url)
      {
      }
      ~RegistrarImpl()
      {
        for (unsigned i=0; i<acl.size(); i++) delete acl[i];
      }
      virtual unsigned getId() const
      {
        return id;
      }
      virtual const std::string& getHandle() const
      {
        return handle;
      }
      virtual void setHandle(const std::string& newHandle)
      {
        SET(handle,newHandle);
      }       
      virtual const std::string& getName() const
      {
        return name;
      }
      virtual void setName(const std::string& newName)
      {
        SET(name,newName);
      }
      virtual const std::string& getURL() const
      {
        return url;
      }
      virtual void setURL(const std::string& newURL)
      {
        SET(url,newURL);
      }
      virtual unsigned getACLSize() const
      {
        return acl.size();
      }
      virtual ACL* getACL(unsigned idx) const
      {
        return idx < acl.size() ? acl[idx] : NULL;
      }
      virtual ACL* newACL()
      {
        ACLImpl* newACL = new ACLImpl();
        acl.push_back(newACL);
        return newACL;
      }
      virtual void save() throw (SQL_ERROR)
      {
        if (changed) {
          // save registrar data
          /*
          SQL_SAVE(sql,"registrar",id);
          SQL_SAVE_ADD(sql,"name",name);
          SQL_SAVE_ADD(sql,"handle",handle);
          SQL_SAVE_ADD(sql,"url",url);
          SQL_SAVE_DOIT(sql,db);
          */
          std::ostringstream sql;
          if (id) {
            sql << "UPDATE registrar SET "
                << "name='" << getName() << "',"
                << "handle='" << getHandle() << "',"
                << "url='" << getURL() << "' "
                << "WHERE id=" << id;
          } else {
            id = db->GetSequenceID("registrar");
            sql << "INSERT INTO registrar "
                << "(id,name,handle,url,zone) "
                << "VALUES "
                << "("
                << id << ","
                << "'" << getName() << "',"
                << "'" << getHandle() << "',"
                << "'" << getURL() << "',"
                << "'{1}'"
                << ")";
          }
          if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();          
        }
        ACLList::const_iterator i = find_if(
          acl.begin(),acl.end(),std::mem_fun(&ACLImpl::hasChanged)
        );
        if (i != acl.end()) {
          std::ostringstream sql;
          sql << "DELETE FROM registraracl WHERE registrarid=" << id;
          // make sql
          for (unsigned j=0;j<acl.size();j++) {
            sql.str("");
            sql << acl[j]->makeSQL(id);
            if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();          
          }
        }
      }
      void putACL(
        unsigned id,
        const std::string& certificateMD5,
        const std::string& password
      )
      {
        acl.push_back(new ACLImpl(id,certificateMD5,password));
      }
      bool hasId(unsigned _id) const
      {
        return id == _id;
      }      
    };
    
    class RegistrarListImpl : virtual public RegistrarList
    {
      typedef std::vector<RegistrarImpl*> RegistrarListType;
      RegistrarListType registrars;
      DB *db;
      std::string name;
      std::string handle;
      unsigned idFilter;
     public:
      RegistrarListImpl(DB *_db) : db(_db), idFilter(0)
      {
      }
      virtual void setIdFilter(unsigned _idFilter)
      {
        idFilter = _idFilter;
      }
      virtual void setHandleFilter(const std::string& _handle)
      {
        handle = _handle;
      }
      virtual void setNameFilter(const std::string& _name)
      {
        name = _name;
      }
      void clear()
      {
        for (unsigned i=0; i<registrars.size(); i++)
          delete registrars[i];
        registrars.clear();
      }
      virtual void reload() throw (SQL_ERROR)
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT id,handle,name,url "
            << "FROM registrar WHERE 1=1 ";
        if (idFilter)
          sql << " AND id=" << idFilter << " ";
        if (!name.empty())
          sql << " AND name ILIKE '%" << name << "%'";
        if (!handle.empty())
          sql << " AND handle ILIKE '%" << name << "%'";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          registrars.push_back(
            new RegistrarImpl(
              db,
              atoi(db->GetFieldValue(i,0)),
              db->GetFieldValue(i,1),
              db->GetFieldValue(i,2),
              db->GetFieldValue(i,3)
            )
          );
        }
        db->FreeSelect();
        sql.str("");
        sql << "SELECT registrarid,cert,password "
            << "FROM registraracl";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          // find associated registrar
          unsigned registrarId = atoi(db->GetFieldValue(i,0));
          RegistrarListType::iterator r = find_if(
            registrars.begin(),registrars.end(),
            std::bind2nd(std::mem_fun(&RegistrarImpl::hasId),registrarId)
          );
          if (r == registrars.end()) continue;
          // if found add acl
          (*r)->putACL(0,db->GetFieldValue(i,1),db->GetFieldValue(i,2));
        }
        db->FreeSelect();
      }
      virtual unsigned size() const
      {
        return registrars.size();
      }
      virtual const Registrar* get(unsigned idx) const
      {
        if (idx > size()) return NULL;
        return registrars[idx];
      }
      virtual Registrar* get(unsigned idx)
      {
        if (idx > size()) return NULL;
        return registrars[idx];
      }
      virtual Registrar* create()
      {
        return new RegistrarImpl(db);
      }
      void clearFilter()
      {
        name = "";
        handle = "";
      }      
    };

    class EPPActionImpl : virtual public EPPAction
    {
      unsigned sessionId;
      unsigned type;
      std::string typeName;
      ptime startTime;
      std::string serverTransactionId;
      std::string clientTransactionId;
      std::string message;
      unsigned result;
      std::string registrarHandle;
     public:
      EPPActionImpl(
        unsigned _sessionId,
        unsigned _type,
        const std::string& _typeName,
        ptime _startTime,
        const std::string& _serverTransactionId,
        const std::string& _clientTransactionId,
        const std::string& _message,
        unsigned _result,
        const std::string& _registrarHandle
      ) : 
        sessionId(_sessionId), type(_type), typeName(_typeName),
        startTime(_startTime),
        serverTransactionId(_serverTransactionId),
        clientTransactionId(_clientTransactionId),
        message(_message), result(_result), registrarHandle(_registrarHandle)
      {
      }
      virtual unsigned getSessionId() const
      {
        return sessionId;
      }
      virtual unsigned getType() const
      {
        return type;
      }
      virtual const std::string& getTypeName() const
      {
        return typeName;
      }
      virtual const ptime getStartTime() const
      {
        return startTime;
      }
      virtual const std::string& getServerTransactionId() const
      {
        return serverTransactionId;
      }
      virtual const std::string& getClientTransactionId() const
      {
        return clientTransactionId;
      }
      virtual const std::string& getEPPMessage() const
      {
        return message;
      }
      virtual unsigned getResult() const
      {
        return result;
      }
      virtual const std::string& getRegistrarHandle() const
      {
        return registrarHandle;
      }
    };

    /// List of EPPAction objects
    class EPPActionListImpl : virtual public EPPActionList
    {
      typedef std::vector<EPPActionImpl *> ActionList;
      ActionList alist;
      unsigned sessionId;
      unsigned registrarId;
      std::string registrarHandle;
      time_period period;
      unsigned typeId;
      std::string type;
      unsigned returnCodeId;
      std::string clTRID;
      std::string svTRID;
      std::string handle;
      DB *db;
     public:
      EPPActionListImpl(DB *_db) :
       sessionId(0), registrarId(0), 
       period(ptime(neg_infin),ptime(pos_infin)),
       typeId(0), returnCodeId(0), db(_db)
      {
      }
      void setSessionFilter(unsigned _sessionId)
      {
        sessionId = _sessionId;
      }
      void setRegistrarFilter(unsigned _registrarId)
      {
        registrarId = _registrarId;
      }
      void setRegistrarHandleFilter(const std::string& _registrarHandle)
      {
        registrarHandle = _registrarHandle;
      }
      void setTimePeriodFilter(const time_period& _period)
      {
        period = _period;
      }
      virtual void setTypeFilter(unsigned _typeId)
      {
        typeId = _typeId;
      }
      virtual void setReturnCodeFilter(unsigned _returnCodeId)
      {
        returnCodeId = _returnCodeId;
      }
      virtual void setHandleFilter(const std::string& _handle)
      {
        handle = _handle;
      }
      virtual void setTextTypeFilter(const std::string& _textType)
      {
        type = _textType;
      }
      virtual void setClTRIDFilter(const std::string& _clTRID)
      {
        clTRID = _clTRID;
      }
      virtual void setSvTRIDFilter(const std::string& _svTRID)
      {      
        svTRID = _svTRID;
      }
      void clear()
      {
        for (unsigned i=0; i<alist.size(); i++)
          delete alist[i];
        alist.clear();
      }
      virtual void reload()
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT a.clientid,a.action,ea.status,a.startdate,"
            << "a.servertrid,a.clienttrid,"
            << "ax.xml,a.response,r.handle "
            << "FROM action a, action_xml ax, enum_action ea,"
            << "login l, "
            << "registrar r "
            << "WHERE a.id=ax.actionid AND l.id=a.clientid "
            << "AND r.id=l.registrarid AND ea.id=a.action ";
        if (registrarId)
          sql << "AND r.id=" << registrarId << " ";
        if (!registrarHandle.empty())
          sql << "AND r.handle='" << registrarHandle << "' ";
        if (!period.begin().is_special())
          sql << "AND a.startdate>='" 
              <<  to_iso_extended_string(period.begin().date()) 
              << "' ";
        if (!period.end().is_special())
          sql << "AND a.startdate<='" 
              <<  to_iso_extended_string(period.end().date()) 
              << "' ";
        if (!type.empty())
          sql << "AND ea.status='" << type << "' ";
        if (returnCodeId)
          sql << "AND a.response=" << returnCodeId << " ";
        if (!clTRID.empty())
          sql << "AND a.clienttrid='" << clTRID << "' ";
        if (!svTRID.empty())
          sql << "AND a.servertrid='" << svTRID << "' ";
        if (!handle.empty())
          sql << "AND ax.xml ILIKE '%" << handle << "%' ";
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          alist.push_back(
            new EPPActionImpl(
              atoi(db->GetFieldValue(i,0)),
              atoi(db->GetFieldValue(i,1)),
              db->GetFieldValue(i,2),
              ptime(time_from_string(db->GetFieldValue(i,3))),
              db->GetFieldValue(i,4),
              db->GetFieldValue(i,5),
              db->GetFieldValue(i,6),
              atoi(db->GetFieldValue(i,7)),
              db->GetFieldValue(i,8)
            )
          );
        }
      }
      virtual const unsigned size() const
      {
        return alist.size();
      }
      virtual const EPPAction* get(unsigned idx) const
      {
        return idx >= alist.size() ?  NULL : alist[idx];
      }
      virtual void clearFilter()
      {
        sessionId = 0;
        registrarId = 0;
        registrarHandle = "";
        period = time_period(ptime(neg_infin),ptime(pos_infin));
        typeId = 0;
        type = "";
        returnCodeId = 0;
        clTRID = "";
        svTRID = "";
        handle = "";
      }
    };

    class ManagerImpl : virtual public Manager
    {
      DB *db; ///< connection do db
      RegistrarListImpl rl;
      EPPActionListImpl eal;
     public:
      ManagerImpl(DB *_db) :
        db(_db), rl(_db), eal(db)
      {}     
      virtual RegistrarList *getList()
      {
        return &rl;
      }
      virtual EPPActionList *getEPPActionList()
      {
        return &eal;
      }
    }; // class ManagerImpl
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }

  }; // namespace Registrar
}; // namespace Register
