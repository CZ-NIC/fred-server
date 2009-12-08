#ifndef _PAGETABLE_STATEMENTS_H
#define	_PAGETABLE_STATEMENTS_H

#include "pagetable_impl.h"
#include "register/bank_head_list.h"

class ccReg_StatementHeads_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:
    std::auto_ptr<Register::Banking::HeadList> m_statementList;
public:
    ccReg_StatementHeads_i(Register::Banking::HeadList *list);
    ~ccReg_StatementHeads_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Register::Banking::StatementHead *findId(ccReg::TID id);
}; // class ccReg_PageTable_i


#endif	/* _PAGETABLE_STATEMENTS_H */

