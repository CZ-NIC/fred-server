#ifndef PAGETABLE_REGISTRARS_H_
#define PAGETABLE_REGISTRARS_H_

#include "pagetable_impl.h"

class ccReg_Registrars_i : public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
private:
  Fred::Registrar::RegistrarList::AutoPtr rl;
  Fred::Zone::Manager::ZoneListPtr zl;
  Fred::Registrar::RegistrarZoneAccess rza;

  enum cols {static_cols = 4};///number of static columns in pagetable

public:
  ccReg_Registrars_i(Fred::Registrar::RegistrarList::AutoPtr _rl
					  , Fred::Zone::Manager::ZoneListPtr _zl);
  ~ccReg_Registrars_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  Fred::Registrar::Registrar* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_REGISTRARS_H_*/
