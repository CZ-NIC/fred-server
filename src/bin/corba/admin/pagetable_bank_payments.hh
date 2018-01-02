#ifndef PAGETABLE_BANK_PAYMENTS_HH_4D8F5789C23F4D148DC38D20725ECE13
#define PAGETABLE_BANK_PAYMENTS_HH_4D8F5789C23F4D148DC38D20725ECE13

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

#endif
