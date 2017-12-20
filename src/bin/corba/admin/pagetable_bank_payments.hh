#ifndef _PAGETABLE_PAYMENTS_H_
#define _PAGETABLE_PAYMENTS_H_

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "src/libfred/banking/bank_payment_list.hh"

class ccReg_Payments_i: public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase
{
private:
    LibFred::Banking::PaymentListPtr list_;

public:
    ccReg_Payments_i(LibFred::Banking::PaymentList *list);
    ~ccReg_Payments_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    LibFred::Banking::Payment *findId(ccReg::TID id);
}; // class ccReg_PageTable_i

#endif // _PAGETABLE_PAYMENTS_H_
