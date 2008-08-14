#ifndef PAGETABLE_CONTACTS_H_
#define PAGETABLE_CONTACTS_H_

#include "pagetable_impl.h"

class ccReg_Contacts_i: public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Register::Contact::List> cl;

public:
  ccReg_Contacts_i(Register::Contact::List *cl, const Settings *_ptr);
  ~ccReg_Contacts_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Register::Contact::Contact* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_CONTACTS_H_*/
