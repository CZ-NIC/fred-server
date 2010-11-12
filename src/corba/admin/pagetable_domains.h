#ifndef PAGETABLE_DOMAINS_H_
#define PAGETABLE_DOMAINS_H_

#include "pagetable_impl.h"

class ccReg_Domains_i : public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Fred::Domain::List> dl;

public:
  ccReg_Domains_i(Fred::Domain::List *dl, const Settings *_ptr);
  ~ccReg_Domains_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Fred::Domain::Domain* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_DOMAINS_H_*/
