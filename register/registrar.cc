#include "registrar.h"
#include "dbsql.h"
#include <vector>

#define SET(a,b) { a = b; changed = true; }
namespace Register
{
  namespace Registrar
  {
    class RegistrarImpl : virtual public Registrar 
    {
      unsigned id; ///< DB: registrar.id
      std::string handle; ///< DB: registrar.handle
      std::string name; ///< DB: registrar.name
      std::string url; ///< DB: registrar.name
      std::string password; ///< DB: registrar.name
      bool changed; ///< object was changed, need sync to database
     public:
      RegistrarImpl() : id(0), changed(true)
      {}
      RegistrarImpl(
        unsigned _id, const std::string& _handle, const std::string& _name,
        const std::string& _url
      ) : id(_id), handle(_handle), name(_name), url(_url)
      {
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
      virtual void setPassword(const std::string& newPassword)
      {
        SET(password,newPassword);
      }
      virtual void save() throw (SQL_ERROR)
      {
        std::ostringstream sql;
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
            << "FROM registrars ";
        if (!db->ExecSelect(sql.str().c_str())) throw SQL_ERROR();
        for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
          //registrars.push_back(new RegistrarImpl(
//            db->GetFieldValue(i,0)
//          );
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
     public:
      ManagerImpl(DB *_db) :
       db(_db)
      {}     
      virtual RegistrarList *getList()
      {
        return NULL;
      }
    }; // class ManagerImpl
    Manager *Manager::create(DB *db)
    {
      return new ManagerImpl(db);
    }

  }; // namespace Registrar
}; // namespace Register
