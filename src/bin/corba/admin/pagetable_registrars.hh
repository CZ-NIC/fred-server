#ifndef PAGETABLE_REGISTRARS_HH_21777BFEC188458A863AC807DD811054
#define PAGETABLE_REGISTRARS_HH_21777BFEC188458A863AC807DD811054

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Registrars_i : public ccReg_PageTable_i,
                           public PortableServer::RefCountServantBase {
private:
  LibFred::Registrar::RegistrarList::AutoPtr rl;
  LibFred::Zone::Manager::ZoneListPtr zl;
  LibFred::Registrar::RegistrarZoneAccess rza;

  enum cols {static_cols = 4};///number of static columns in pagetable

public:
  ccReg_Registrars_i(LibFred::Registrar::RegistrarList::AutoPtr _rl
					  , LibFred::Zone::Manager::ZoneListPtr _zl);
  ~ccReg_Registrars_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::Registrar::Registrar* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_REGISTRARS_H_*/
