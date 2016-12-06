#ifndef PAGETABLE_NSSETS_H_E7904D94854D49418F4C811593A989BD
#define PAGETABLE_NSSETS_H_E7904D94854D49418F4C811593A989BD

#include "pagetable_impl.h"

class ccReg_NSSets_i : public ccReg_PageTable_i,
                       public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Fred::Nsset::List> nl;
  
public:
  ccReg_NSSets_i(Fred::Nsset::List *nl, const Settings *_ptr);
  ~ccReg_NSSets_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  Fred::Nsset::Nsset* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_NSSETS_H_*/
