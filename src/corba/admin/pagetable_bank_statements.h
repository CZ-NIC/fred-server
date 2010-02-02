#ifndef _PAGETABLE_STATEMENTS_H
#define	_PAGETABLE_STATEMENTS_H

#include "pagetable_impl.h"
#include "register/bank_statement_list.h"

class ccReg_Statements_i: public ccReg_PageTable_i,
                          public PortableServer::RefCountServantBase
{
private:
    Register::Banking::StatementListPtr list_;

public:
    ccReg_Statements_i(Register::Banking::StatementList *list);
    ~ccReg_Statements_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Register::Banking::Statement *findId(ccReg::TID id);
}; // class ccReg_PageTable_i


#endif	/* _PAGETABLE_STATEMENTS_H */

