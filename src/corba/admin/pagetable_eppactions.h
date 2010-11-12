#ifndef PAGETABLE_EPPACTIONS_H_
#define PAGETABLE_EPPACTIONS_H_

#include "pagetable_impl.h"

class ccReg_EPPActions_i : public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
private:                     
  Fred::Registrar::EPPActionList *eal;
  
public:
  ccReg_EPPActions_i(Fred::Registrar::EPPActionList *eal);
  ~ccReg_EPPActions_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Fred::Registrar::EPPAction* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_EPPACTIONS_H_*/
