#ifndef PAGETABLE_NSSETS_H_
#define PAGETABLE_NSSETS_H_

#include "pagetable_impl.h"

class ccReg_NSSets_i : public ccReg_PageTable_i,
                       public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Register::NSSet::List> nl;
  
public:
  ccReg_NSSets_i(Register::NSSet::List *nl, const Settings *_ptr);
  ~ccReg_NSSets_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  Register::NSSet::NSSet* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_NSSETS_H_*/
