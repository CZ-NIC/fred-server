#ifndef PAGETABLE_INVOICES_H_
#define PAGETABLE_INVOICES_H_

#include "pagetable_impl.h"

class ccReg_Invoices_i : public ccReg_PageTable_i,
                         public PortableServer::RefCountServantBase {
private:
  std::auto_ptr<Register::Invoicing::InvoiceList> invl;

public:
  ccReg_Invoices_i(Register::Invoicing::InvoiceList* _invl);
  ~ccReg_Invoices_i();
  DECL_PAGETABLE_I;

  DECL_ATTRIBUTE_ID(id);
  DECL_ATTRIBUTE_STR(number);
  DECL_ATTRIBUTE_DATE(crDate);
  DECL_ATTRIBUTE_ID(registrarId);
  DECL_ATTRIBUTE_STR(registrarHandle);
  DECL_ATTRIBUTE_ID(zone);
  DECL_ATTRIBUTE_TYPE(type,ccReg::Invoicing::InvoiceType);
  DECL_ATTRIBUTE_STR(varSymbol);
  DECL_ATTRIBUTE_DATE(taxDate);
  DECL_ATTRIBUTE_STR(objectName);
  DECL_ATTRIBUTE_ID(objectId);
  DECL_ATTRIBUTE_STR(advanceNumber);
};

#endif /*PAGETABLE_INVOICES_H_*/
