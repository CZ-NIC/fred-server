#ifndef PAGETABLE_INVOICES_HH_41AB4BFB090C42228DC2EBACAE626194
#define PAGETABLE_INVOICES_HH_41AB4BFB090C42228DC2EBACAE626194

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

class ccReg_Invoices_i : public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
private:
  std::unique_ptr<LibFred::Invoicing::List> invoice_list_;

public:
  ccReg_Invoices_i(LibFred::Invoicing::List* _invoice_list);
  ~ccReg_Invoices_i();
  DECL_PAGETABLE_I;

  ccReg::Filters::Compound_ptr add();
  LibFred::Invoicing::Invoice* findId(ccReg::TID _id);
};

#endif /*PAGETABLE_INVOICES_H_*/
