#ifndef PAGETABLE_DOMAINS_H_
#define PAGETABLE_DOMAINS_H_

#include "pagetable_impl.h"

class ccReg_Domains_i : public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Register::Domain::List> dl;

public:
  ccReg_Domains_i(Register::Domain::List *dl, const Settings *_ptr);
  ~ccReg_Domains_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Register::Domain::Domain* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_DOMAINS_H_*/
