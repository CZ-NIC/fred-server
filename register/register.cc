#include <memory> ///< auto_ptr<>
#include <ctype.h> ///< toupper()
#include "register.h"
#include "dbsql.h"
#include "zone.h"
#include "domain.h"

namespace Register
{
 class StatusDescImpl : public virtual StatusDesc
 {
   TID id;
   std::string name;
   bool external;
   std::map<std::string,std::string> desc;
   bool contact;
   bool nsset;
   bool domain;
  public:
   StatusDescImpl(
     TID _id, const std::string& _name, bool _external, 
     const std::string& types
   )
    : id(_id), name(_name), external(_external)
   {
     contact = types.find("1") != std::string::npos;
     nsset = types.find("2") != std::string::npos;
     domain = types.find("3") != std::string::npos;
   }
   void addDesc(const std::string& lang, const std::string text)
   {
     desc[lang] = text;
   }
   bool operator==(TID _id) const
   {
     return id == _id;
   }
   virtual TID getId() const
   {
     return id;
   }
   virtual const std::string& getName() const
   {
     return name;
   }
   virtual bool getExternal() const
   {
     return external;
   }
   virtual const std::string& getDesc(const std::string& lang) const
     throw (BAD_LANG)
   {
	 std::map<std::string,std::string>::const_iterator i = desc.find(lang);
	 if (i == desc.end()) throw BAD_LANG();
     return i->second;
   }
   virtual bool isForType(short type) const
   {
     return type == 1 ? contact : type == 2 ? nsset : domain;
   }
 };
 class ManagerImpl : virtual public Manager
 {
   DB *db;
   std::auto_ptr<Zone::Manager> zm;
   std::auto_ptr<Domain::Manager> dm;
   std::auto_ptr<Registrar::Manager> rm;
   std::auto_ptr<Contact::Manager> cm;
   std::auto_ptr<NSSet::Manager> nm;
   std::vector<CountryDesc> countries;
   bool restrictedHandles;
   std::vector<StatusDescImpl> statusList;
  public:
   ManagerImpl(DB *_db, bool _restrictedHandles) : 
     db(_db), restrictedHandles(_restrictedHandles)
   {
     zm.reset(Zone::Manager::create(db));
     dm.reset(Domain::Manager::create(db,zm.get()));
     rm.reset(Registrar::Manager::create(db));
     cm.reset(Contact::Manager::create(db,restrictedHandles));
     nm.reset(NSSet::Manager::create(db,restrictedHandles));
     // TODO SQL load
     CountryDesc cd;
     cd.cc = "CZ";
     cd.name = "Czech Republic";
     countries.push_back(cd);
     cd.cc = "SK";
     cd.name = "Slovak Republic";
     countries.push_back(cd);
   } 
   /// interface method implementation
   void checkHandle(const std::string& handle, CheckHandleList& chl) const
   {
     CheckHandle ch;
     bool isEnum = false;
     try {
       // trying convert string to enum domain
       ch.newHandle = dm->makeEnumDomain(handle);
       ch.type = HT_ENUM_NUMBER;
       isEnum = true;
     } catch (...) {}
     bool isDomain = true;
     std::string fqdn = isEnum ? ch.newHandle : handle;
     NameIdPair conflictFQDN; 
     switch (dm->checkAvail(fqdn, conflictFQDN)) {
       case Domain::CA_INVALID_HANDLE:
        isDomain = false; break;
       case Domain::CA_BAD_LENGHT: 
         ch.handleClass = CH_UNREGISTRABLE_LONG; break;
       case Domain::CA_BAD_ZONE:
         ch.handleClass = CH_UNREGISTRABLE; break;
       case Domain::CA_BLACKLIST: 
         ch.handleClass = CH_PROTECTED; break;
       case Domain::CA_REGISTRED:
         ch.handleClass = CH_REGISTRED; break;
       case Domain::CA_PARENT_REGISTRED: 
         ch.handleClass = CH_REGISTRED_PARENT; break;
       case Domain::CA_CHILD_REGISTRED: 
         ch.handleClass = CH_REGISTRED_CHILD; break;
       case Domain::CA_AVAILABLE:
         ch.handleClass = CH_FREE; break;
     }
     ch.conflictHandle = conflictFQDN.name;
     if (isDomain) {
       if (!isEnum)
         ch.type = dm->checkEnumDomainSuffix(fqdn) ? HT_ENUM_DOMAIN : HT_DOMAIN;
       chl.push_back(ch);
     }
     // check if handle is registred contact
     NameIdPair conflictContact;
     if (getContactManager()->checkAvail(handle,conflictContact) == 
         Contact::Manager::CA_REGISTRED) {
       CheckHandle chCon;
       chCon.type = HT_CONTACT;
       chCon.handleClass= CH_REGISTRED; 
       chl.push_back(chCon);
     }
     // check if handle is registred nsset   
     NameIdPair conflictNSSet;
     if (getNSSetManager()->checkAvail(handle,conflictNSSet) == 
         NSSet::Manager::CA_REGISTRED) {
       CheckHandle chNss;
       chNss.type = HT_NSSET;
       chNss.handleClass= CH_REGISTRED; 
       chl.push_back(chNss);
     }
     // check if handle is registrant   
     if (getRegistrarManager()->checkHandle(handle)) {
       CheckHandle chReg;
       chReg.type = HT_REGISTRAR;
       chReg.handleClass= CH_REGISTRED;
       chl.push_back(chReg);       
     }
     // if empty return OTHER   
     if (!chl.size()) {
       CheckHandle chOth;
       chOth.type = HT_OTHER;
       chOth.handleClass= CH_FREE;
       chl.push_back(chOth);
     }
   }
   Domain::Manager *getDomainManager()
   {
     return dm.get();
   }   
   Registrar::Manager *getRegistrarManager() const
   {
     return rm.get();
   }   
   Contact::Manager *getContactManager() const
   {
     return cm.get();
   }   
   NSSet::Manager *getNSSetManager() const
   {
     return nm.get();
   }
   virtual unsigned getCountryDescSize() const
   {
     return countries.size();
   }
   virtual const CountryDesc& getCountryDescByIdx(unsigned idx) const
     throw (NOT_FOUND)
   {
     if (idx >= countries.size()) throw NOT_FOUND();
     return countries[idx];
   }
   virtual void initStates() throw (SQL_ERROR)
   {
     if (!db->ExecSelect(
       "SELECT id, name, external, ARRAY_TO_STRING(types,',') "
       "FROM enum_object_states")
     ) throw SQL_ERROR();
     statusList.clear();
     for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
       statusList.push_back(
         StatusDescImpl( 
           STR_TO_ID(db->GetFieldValue(i,0)),
           db->GetFieldValue(i,1),
           *db->GetFieldValue(i,2) == 't',
           db->GetFieldValue(i,3)
         )
       );
     }
     db->FreeSelect();
     if (!db->ExecSelect(
       "SELECT state_id, lang, description FROM enum_object_states_desc"
     )) throw SQL_ERROR();
     for (unsigned i=0; i < (unsigned)db->GetSelectRows(); i++) {
       std::vector<StatusDescImpl>::iterator it = find(
         statusList.begin(), statusList.end(), 
         STR_TO_ID(db->GetFieldValue(i,0))
       );
       if (it != statusList.end())
         it->addDesc(db->GetFieldValue(i,1),db->GetFieldValue(i,2));
     }
     // hack for OK state
     statusList.push_back(StatusDescImpl(0,"ok",true,"1,2,3"));
     statusList.back().addDesc("CS","Objekt je bez omezení");
     statusList.back().addDesc("EN","Objekt is without restrictions");
   }
   virtual const StatusDesc* getStatusDesc(TID status) const
   {
     std::vector<StatusDescImpl>::const_iterator it = find(
       statusList.begin(), statusList.end(), status 
     );
     if (it == statusList.end()) return NULL;
     return &(*it);
   }
   virtual unsigned getStatusDescCount() const
   {
     return statusList.size();
   }
   virtual const StatusDesc* getStatusDescByIdx(unsigned idx) const
   {
     if (idx >= statusList.size()) return NULL;
     return &statusList[idx];
   }
   virtual void updateObjectStates() const throw (SQL_ERROR)
   {
     if (!db->ExecSelect("SELECT update_object_states()"))
       throw SQL_ERROR();
     db->FreeSelect();
   }
 };
 Manager *Manager::create(DB *db, bool restrictedHandles)
 {
   return new ManagerImpl(db, restrictedHandles);
 }
}
