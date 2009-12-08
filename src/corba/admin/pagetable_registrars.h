#ifndef PAGETABLE_REGISTRARS_H_
#define PAGETABLE_REGISTRARS_H_

#include "pagetable_impl.h"

class ccReg_Registrars_i : public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
private:
  Register::Registrar::Manager::RegistrarListPtr rl;
  Register::Zone::Manager::ZoneListPtr zl;

  enum cols {static_cols = 3};///number of static columns in pagetable

public:
  ccReg_Registrars_i(Register::Registrar::Manager::RegistrarListPtr _rl
					  , Register::Zone::Manager::ZoneListPtr _zl
					  );
  ~ccReg_Registrars_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Register::Registrar::Registrar* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_REGISTRARS_H_*/
