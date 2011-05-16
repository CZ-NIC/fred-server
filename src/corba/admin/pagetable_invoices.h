#ifndef PAGETABLE_INVOICES_H_
#define PAGETABLE_INVOICES_H_

#include "pagetable_impl.h"
#include "fredlib/invoicing/invoice.h"

class ccReg_Invoices_i : public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Fred::Invoicing::List> invoice_list_;

public:
  ccReg_Invoices_i(Fred::Invoicing::List* _invoice_list);
  ~ccReg_Invoices_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  Fred::Invoicing::Invoice* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_INVOICES_H_*/
