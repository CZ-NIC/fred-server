#ifndef PAGETABLE_REGISTRARS_H_
#define PAGETABLE_REGISTRARS_H_

#include "pagetable_impl.h"

class ccReg_Registrars_i : public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
private:
  Register::Registrar::RegistrarList *rl;

public:
  ccReg_Registrars_i(Register::Registrar::RegistrarList *rl);
  ~ccReg_Registrars_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Register::Registrar::Registrar* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_REGISTRARS_H_*/
