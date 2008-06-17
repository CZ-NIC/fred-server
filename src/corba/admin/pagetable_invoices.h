#ifndef PAGETABLE_INVOICES_H_
#define PAGETABLE_INVOICES_H_

#include "pagetable_impl.h"

class ccReg_Invoices_i : public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Register::Invoicing::List> invoice_list_;

public:
  ccReg_Invoices_i(Register::Invoicing::List* _invoice_list);
  ~ccReg_Invoices_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  Register::Invoicing::Invoice* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_INVOICES_H_*/
