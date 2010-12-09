#ifndef _PAGETABLE_PAYMENTS_H_
#define _PAGETABLE_PAYMENTS_H_

#include "pagetable_impl.h"
#include "bank_payment_list.h"

class ccReg_Payments_i: public ccReg_PageTable_i,
                        public PortableServer::RefCountServantBase
{
private:
    Fred::Banking::PaymentListPtr list_;

public:
    ccReg_Payments_i(Fred::Banking::PaymentList *list);
    ~ccReg_Payments_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Fred::Banking::Payment *findId(ccReg::TID id);
}; // class ccReg_PageTable_i

#endif // _PAGETABLE_PAYMENTS_H_
