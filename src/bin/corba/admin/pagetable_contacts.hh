#ifndef PAGETABLE_CONTACTS_HH_1DBBB8837AF745B2837E47F11CEB324F
#define PAGETABLE_CONTACTS_HH_1DBBB8837AF745B2837E47F11CEB324F

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Contacts_i: public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::Contact::List> cl;

public:
  ccReg_Contacts_i(LibFred::Contact::List *cl, const Settings *_ptr);
  ~ccReg_Contacts_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::Contact::Contact* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_CONTACTS_H_*/
