#include "registrar.h"
#include "sql.h"
#include "dbsql.h"
#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/regex.hpp>
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
            << "(" << registrarId << ",'" << certificateMD5 << "','" 
            << password << "');";
        return sql.str();
      }
    };

    class RegistrarImpl : virtual public Registrar 
    {
      typedef std::vector<ACLImpl *> ACLList;
      typedef ACLList::iterator ACLListIter;
      DB *db; ///< db connection
      TID id; ///< DB: registrar.id
      std::string handle; ///< DB: registrar.handle
      std::string name; ///< DB: registrar.name
      std::string url; ///< DB: registrar.name
      std::string password; ///< DB: registraracl.passwd
      std::string organization; ///< DB: registrar.organization
      std::string street1; ///< DB: registrar.street1
      std::string street2; ///< DB: registrar.street2
      std::string street3; ///< DB: registrar.street3
      std::string city; ///< DB: registrar.city
      std::string province; ///< DB: registrar.stateorprovince
      std::string postalCode; ///< DB: registrar.postalcode
      std::string country; ///< DB: registrar.country
      std::string telephone; ///< DB: registrar.telephone
      std::string fax; ///< DB: registrar.fax
      std::string email; ///< DB: registrar.email
      unsigned long credit; ///< DB: registrar.credit
      bool changed; ///< object was changed, need sync to database
      ACLList acl; ///< access control
     public:
      RegistrarImpl(DB *_db) :  db(_db), id(0), changed(true)
      {}
      RegistrarImpl(DB *_db,
        TID _id, const std::string& _handle, const std::string& _name,
        const std::string& _url, const std::string& _organization, 
        const std::string& _street1, const std::string& _street2, 
        const std::string& _street3, const std::string& _city, 
        const std::string& _province, const std::string& _postalCode, 
        const std::string& _country, const std::string& _telephone, 
        const std::string& _fax, const std::string& _email, 
        unsigned long _credit 
      ) : db(_db), id(_id), handle(_handle), name(_name), url(_url),
          organization(_organization), street1(_street1), street2(_street2), 
          street3(_street3), city(_city), province(_province), 
          postalCode(_postalCode), country(_country), telephone(_telephone), 
          fax(_fax), email(_email), credit(_credit)
      {
      }
      void clear()
      {
        for (unsigned i=0; i<acl.size(); i++) delete acl[i];
        acl.clear();
      }
      ~RegistrarImpl()
      {
        clear();
      }
      virtual TID getId() const
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
      virtual const std::string& getOrganization() const
      {
        return organization;
      }
      virtual void setOrganization(const std::string& _organization)
      {
        SET(organization,_organization);
      }
      virtual const std::string& getStreet1() const
      {
        return street1;
      }
      virtual void setStreet1(const std::string& _street1)
      {
        SET(street1,_street1);
      }
      virtual const std::string& getStreet2() const
      {
        return street2;
      }
      virtual void setStreet2(const std::string& _street2)
      {
        SET(street2,_street2);
      }
      virtual const std::string& getStreet3() const
      {
        return street3;
      }
      virtual void setStreet3(const std::string& _street3)
      {
        SET(street3,_street3);
      }
      virtual const std::string& getCity() const
      {
        return city;
      }
      virtual void setCity(const std::string& _city)
      {
        SET(city,_city);
      }
      virtual const std::string& getProvince() const
      {
        return province;
      }
      virtual void setProvince(const std::string& _province)
      {
        SET(province,_province);
      }
      virtual const std::string& getPostalCode() const
      {
        return postalCode;
      }
      virtual void setPostalCode(const std::string& _postalCode)
      {
        SET(postalCode,_postalCode);
      }
      virtual const std::string& getCountry() const
      {
        return country;
      }
      virtual void setCountry(const std::string& _country)
      {
        SET(country,_country);
      }
      virtual const std::string& getTelephone() const
      {
        return telephone;
      }
      virtual void setTelephone(const std::string& _telephone)
      {
        SET(telephone,_telephone);
      }
      virtual const std::string& getFax() const
      {
        return fax;
      }
      virtual void setFax(const std::string& _fax)
      {
        SET(fax,_fax);
      }
      virtual const std::string& getEmail() const
      {
        return email;
      }
      virtual void setEmail(const std::string& _email)
      {
        SET(email,_email);
      }
      virtual unsigned long getCredit() const
      {
        return credit;
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
      virtual void deleteACL(unsigned idx)
      {
        if (idx < acl.size()) {
          delete acl[idx];
          acl.erase(acl.begin()+idx);
        }
      }
      virtual void clearACLList()
      {
        clear();
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
                << "url='" << getURL() << "', "
                << "organization='" << getOrganization() << "',"
                << "street1='" << getStreet1() << "',"
                << "street2='" << getStreet2() << "',"
                << "street3='" << getStreet3() << "',"
                << "city='" << getCity() << "',"
                << "stateorprovince='" << getProvince() << "',"
                << "postalcode='" << getPostalCode() << "',"
                << "country='" << getCountry() << "',"
                << "telephone='" << getTelephone() << "',"
                << "fax='" << getFax() << "',"
                << "email='" << getEmail() << "' "
                << "WHERE id=" << id;
          } else {
            id = db->GetSequenceID("registrar");
            sql << "INSERT INTO registrar "
                << "(id,name,handle,url,organization,street1,street2,"
                << "street3,city,stateorprovince,postalcode,country,"
                << "telephone,fax,email) "
                << "VALUES "
                << "("
                << id << ","
                << "'" << getName() << "',"
                << "'" << getHandle() << "',"
                << "'" << getURL() << "',"
                << "'" << getOrganization() << "',"
                << "'" << getStreet1() << "',"
                << "'" << getStreet2() << "',"
                << "'" << getStreet3() << "',"
                << "'" << getCity() << "',"
                << "'" << getProvince() << "',"
                << "'" << getPostalCode() << "',"
                << "'" << getCountry() << "',"
                << "'" << getTelephone() << "',"
                << "'" << getFax() << "',"
                << "'" << getEmail() << "'"
                << ")";
          }
          if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();          
        }
        ACLList::const_iterator i = find_if(
          acl.begin(),acl.end(),std::mem_fun(&ACLImpl::hasChanged)
        );
        {
          std::ostringstream sql;
          sql << "DELETE FROM registraracl WHERE registrarid=" << id;
          if (!db->ExecSQL(sql.str().c_str())) throw SQL_ERROR();          
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
      std::string xml;
      TID idFilter;
     public:
      RegistrarListImpl(DB *_db) : db(_db), idFilter(0)
      {
      }
      ~RegistrarListImpl()
      {
        clear();
      }
      virtual void setIdFilter(TID _idFilter)
      {
        idFilter = _idFilter;
      }
      virtual void setHandleFilter(const std::string& _handle)
      {
        handle = _handle;
      }
      virtual void setXMLFilter(const std::string& _xml)
      {
        xml = _xml;
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
        sql << "SELECT r.id,r.handle,r.name,r.url,r.organization,"
            << "r.street1,r.street2,r.street3,r.city,r.stateorprovince,"
            << "r.postalcode,r.country,r.telephone,r.fax,r.email,"
            << "COALESCE(SUM(i.credit),0) "
            << "FROM registrar r "
            << "LEFT JOIN invoice i ON (r.id=i.registrarid AND "
            << "NOT(i.credit ISNULL)) WHERE 1=1 ";
        SQL_ID_FILTER(sql,"r.id",idFilter);
        SQL_HANDLE_FILTER(sql,"r.name",name);
        SQL_HANDLE_FILTER(sql,"r.handle",handle);
        sql << "GROUP BY r.id,r.handle,r.name,r.url,r.organization,"
            << "r.street1,r.street2,r.street3,r.city,r.stateorprovince,"
            << "r.postalcode,r.country,r.telephone,r.fax,r.email ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          registrars.push_back(
            new RegistrarImpl(
              db,
              STR_TO_ID(db->GetFieldValue(i,0)),
              db->GetFieldValue(i,1),
              db->GetFieldValue(i,2),
              db->GetFieldValue(i,3),
              db->GetFieldValue(i,4),
              db->GetFieldValue(i,5),
              db->GetFieldValue(i,6),
              db->GetFieldValue(i,7),
              db->GetFieldValue(i,8),
              db->GetFieldValue(i,9),
              db->GetFieldValue(i,10),
              db->GetFieldValue(i,11),
              db->GetFieldValue(i,12),
              db->GetFieldValue(i,13),
              db->GetFieldValue(i,14),
              atol(db->GetFieldValue(i,15))
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
          unsigned registrarId = STR_TO_ID(db->GetFieldValue(i,0));
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
      TID id;
      TID sessionId;
      unsigned type;
      std::string typeName;
      ptime startTime;
      std::string serverTransactionId;
      std::string clientTransactionId;
      std::string message;
      unsigned result;
      std::string registrarHandle;
      std::string handle;
     public:
      EPPActionImpl(
        TID _id,
        TID _sessionId,
        unsigned _type,
        const std::string& _typeName,
        ptime _startTime,
        const std::string& _serverTransactionId,
        const std::string& _clientTransactionId,
        const std::string& _message,
        unsigned _result,
        const std::string& _registrarHandle,
        const std::string& _handle
      ) : 
        id(_id), sessionId(_sessionId), type(_type), typeName(_typeName),
        startTime(_startTime),
        serverTransactionId(_serverTransactionId),
        clientTransactionId(_clientTransactionId),
        message(_message), result(_result), registrarHandle(_registrarHandle),
        handle(_handle)
      {
      }
      virtual TID getId() const
      {
        return id;
      }
      virtual TID getSessionId() const
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
      virtual std::string getResultStatus() const
      {
        return (result < 2000 && result) ? "OK" : "FAILED";
      }
      virtual const std::string& getRegistrarHandle() const
      {
        return registrarHandle;
      }
      virtual const std::string& getHandle() const
      {
        return handle;
      }
    };

    /// List of EPPAction objects
    class EPPActionListImpl : virtual public EPPActionList
    {
      typedef std::vector<EPPActionImpl *> ActionList;
      ActionList alist;
      TID id;
      TID sessionId;
      TID registrarId;
      std::string registrarHandle;
      time_period period;
      unsigned typeId;
      std::string type;
      unsigned returnCodeId;
      std::string clTRID;
      std::string svTRID;
      std::string handle;
      std::string xml;
      EPPActionResultFilter result;
      DB *db;
     public:
      EPPActionListImpl(DB *_db) :
       id(0), sessionId(0), registrarId(0), 
       period(ptime(neg_infin),ptime(pos_infin)),
       typeId(0), returnCodeId(0), result(EARF_ALL),
       db(_db)
      {
      }
      ~EPPActionListImpl()
      {
        clear();
      }
      void setIdFilter(TID _id)
      {
        id = _id;
      }
      void setSessionFilter(TID _sessionId)
      {
        sessionId = _sessionId;
      }
      void setRegistrarFilter(TID _registrarId)
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
      virtual void setResultFilter(EPPActionResultFilter _result)
      {
        result = _result;
      }
      virtual void setHandleFilter(const std::string& _handle)
      {
        handle = _handle;
      }
      virtual void setXMLFilter(const std::string& _xml)
      {
        xml = _xml;
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
#define DB_NULL_INT(i,j) \
  (db->IsNotNull(i,j)) ? atoi(db->GetFieldValue(i,j)) : 0
#define DB_NULL_STR(i,j) \
  (db->IsNotNull(i,j)) ? db->GetFieldValue(i,j) : ""
      virtual void reload()
      {
        clear();
        std::ostringstream sql;
        sql << "SELECT a.id,a.clientid,a.action,ea.status,a.startdate,"
            << "a.servertrid,a.clienttrid,"
            << "ax.xml,a.response,r.handle,'HANDLE' "
            << "FROM enum_action ea, action a " 
            << "LEFT JOIN enum_error er ON (a.response=er.id) " 
            << "LEFT JOIN action_xml ax ON (a.id=ax.actionid) "
            << "LEFT JOIN login l ON (l.id=a.clientid) "
            << "LEFT JOIN registrar r ON (r.id=l.registrarid) "
            << "WHERE ea.id=a.action ";
        SQL_ID_FILTER(sql,"a.id",id);
        SQL_ID_FILTER(sql,"r.id",registrarId);
        SQL_HANDLE_FILTER(sql,"r.handle",registrarHandle);
        SQL_DATE_FILTER(sql,"a.startdate",period);
        SQL_HANDLE_FILTER(sql,"ea.status",type);
        SQL_ID_FILTER(sql,"a.response",returnCodeId);
        SQL_HANDLE_FILTER(sql,"a.clienttrid",clTRID);
        SQL_HANDLE_FILTER(sql,"a.servertrid",svTRID);
        /// TODO - handle has to have special data column
        if (!handle.empty())
          sql << "AND ax.xml ILIKE '%" << handle << "%' ";
        if (!xml.empty())
          sql << "AND ax.xml ILIKE '%" << xml << "%' ";
        if (result != EARF_ALL)
          sql << "AND (a.response " 
              << (result == EARF_OK ? "<" : " IS NULL OR a.response >=") 
              << " 2000) ";
        sql << "LIMIT 1000";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          alist.push_back(
            new EPPActionImpl(
              STR_TO_ID(db->GetFieldValue(i,0)),
              DB_NULL_INT(i,1),
              atoi(db->GetFieldValue(i,2)),
              db->GetFieldValue(i,3),
              ptime(time_from_string(db->GetFieldValue(i,4))),
              db->GetFieldValue(i,5),
              db->GetFieldValue(i,6),
              DB_NULL_STR(i,7),
              DB_NULL_INT(i,8),
              DB_NULL_STR(i,9),
              DB_NULL_STR(i,10)
            )
          );
        }
        db->FreeSelect();
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
        id = 0;
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
        xml = "";
        result = EARF_ALL;
      }
    };

    class ManagerImpl : virtual public Manager
    {
      DB *db; ///< connection do db
      RegistrarListImpl rl;
      EPPActionListImpl eal;
      std::vector<std::string> actionTypes;
     public:
      ManagerImpl(DB *_db) :
        db(_db), rl(_db), eal(db)
      {
        // TODO SQL load
        actionTypes.push_back("DomainCreate");
        actionTypes.push_back("ContactCreate");
        actionTypes.push_back("NSSetCreate");
        actionTypes.push_back("ContactUpdate");
        actionTypes.push_back("DomainUpdate");
        actionTypes.push_back("NSSetUpdate");
        actionTypes.push_back("ContactDelete");
        actionTypes.push_back("DomainDelete");
        actionTypes.push_back("NSSetDelete");
        actionTypes.push_back("ContactTransfer");
        actionTypes.push_back("DomainTransfer");
        actionTypes.push_back("NSSetTransfer");
        actionTypes.push_back("ContactCheck");
        actionTypes.push_back("DomainCheck");
        actionTypes.push_back("NSSetCheck");
        actionTypes.push_back("ContactInfo");
        actionTypes.push_back("DomainInfo");
        actionTypes.push_back("NSSetInfo");
        actionTypes.push_back("DomainRenew");
        actionTypes.push_back("ClientLogin");
        actionTypes.push_back("ClientLogout");
      }     
      virtual RegistrarList *getList()
      {
        return &rl;
      }
      virtual EPPActionList *getEPPActionList()
      {
        return &eal;
      }
      virtual unsigned getEPPActionTypeCount()
      {
        return actionTypes.size();
      }
      virtual const std::string& getEPPActionTypeByIdx(unsigned idx) const
        throw (NOT_FOUND)
      {
        if (idx >= actionTypes.size()) throw NOT_FOUND();
        return actionTypes[idx];
      }      
      virtual bool checkHandle(const std::string handle) const 
        throw (SQL_ERROR)
      {
        if (!boost::regex_match(handle,boost::regex("[rR][eE][gG]-.*")))
          return false;
        std::stringstream sql;
        sql << "SELECT COUNT(*) FROM registrar "
            << "WHERE UPPER(handle)=UPPER('" << handle << "')";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        bool result = atoi(db->GetFieldValue(0,0));
        db->FreeSelect();
        return result;            
      }
    }; // class ManagerImpl
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }

  }; // namespace Registrar
}; // namespace Register
