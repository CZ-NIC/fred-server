#include "registrar.h"
#include "dbsql.h"
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
    };

    class RegistrarImpl : virtual public Registrar 
    {
      typedef std::vector<ACLImpl *> ACLList;
      typedef ACLList::iterator ACLListIter;
      unsigned id; ///< DB: registrar.id
      std::string handle; ///< DB: registrar.handle
      std::string name; ///< DB: registrar.name
      std::string url; ///< DB: registrar.name
      std::string password; ///< DB: registraracl.passwd
      bool changed; ///< object was changed, need sync to database
      ACLList acl; ///< access control
     public:
      RegistrarImpl() : id(0), changed(true)
      {}
      RegistrarImpl(
        unsigned _id, const std::string& _handle, const std::string& _name,
        const std::string& _url
      ) : id(_id), handle(_handle), name(_name), url(_url)
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
      virtual unsigned getACLSize()
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
        if (!changed) return;
        /*
        SQL_SAVE(sql,"registrar",id);
        SQL_SAVE_ADD(sql,"name",name);
        SQL_SAVE_ADD(sql,"handle",handle);
        SQL_SAVE_ADD(sql,"url",url);
        SQL_SAVE_DOIT(sql,db);
        */
      }
      void putACL(
        unsigned id,
        const std::string& certificateMD5,
        const std::string& password
      )
      {
        acl.push_back(new ACLImpl(id,certificateMD5,password));
      }
      bool operator==(unsigned _id)
      {
        return id == _id;
      }      
    };
    
    class RegistrarListImpl : virtual public RegistrarList
    {
      typedef std::vector<RegistrarImpl*> RegistrarListType;
      RegistrarListType registrars;
      DB *db;
     public:
      RegistrarListImpl(DB *_db) : db(_db)
      {
      }
      virtual void reload() throw (SQL_ERROR)
      {
        std::ostringstream sql;
        sql << "SELECT id,handle,name,url "
            << "FROM registrar";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          registrars.push_back(
            new RegistrarImpl(
              atoi(db->GetFieldValue(i,0)),
              db->GetFieldValue(i,1),
              db->GetFieldValue(i,2),
              db->GetFieldValue(i,3)
            )
          );
        }
        db->FreeSelect();
        sql.str("");
        sql << "SELECT registrarid,cert,passwd "
            << "FROM registraracl";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          // find associated registrar
          unsigned registrarId = atoi(db->GetFieldValue(i,0));
          RegistrarListType::iterator r;
          //          RegistrarListType::iterator r = find(
          //            registrars.begin(),registrars.end(),
          //            bind2nd(mem_fun_ref(RegistrarImpl::hasId),registrarId)
          //          );
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
      virtual Registrar* create()
      {
        return new RegistrarImpl();
      }
      
    };
    
    class ManagerImpl : virtual public Manager
    {
      DB *db; ///< connection do db
      RegistrarListImpl rl;
     public:
      ManagerImpl(DB *_db) :
        db(_db), rl(_db)
      {}     
      virtual RegistrarList *getList()
      {
        return &rl;
      }
    }; // class ManagerImpl
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }

  }; // namespace Registrar
}; // namespace Register
