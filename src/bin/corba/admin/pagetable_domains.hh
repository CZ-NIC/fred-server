#ifndef PAGETABLE_DOMAINS_HH_4C3AC2808BAE4DC68423C4FA72C0997B
#define PAGETABLE_DOMAINS_HH_4C3AC2808BAE4DC68423C4FA72C0997B

#include "src/bin/corba/admin/pagetable_impl.hh"

class ccReg_Domains_i : public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::Domain::List> dl;

public:
  ccReg_Domains_i(LibFred::Domain::List *dl, const Settings *_ptr);
  ~ccReg_Domains_i();
  DECL_PAGETABLE_I;
  
  ccReg::Filters::Compound_ptr add();
  LibFred::Domain::Domain* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_DOMAINS_H_*/
