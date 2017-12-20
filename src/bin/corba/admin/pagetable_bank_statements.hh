#ifndef _PAGETABLE_STATEMENTS_H
#define	_PAGETABLE_STATEMENTS_H

#include "src/bin/corba/admin/pagetable_impl.hh"
#include "src/libfred/banking/bank_statement_list.hh"

class ccReg_Statements_i: public ccReg_PageTable_i,
                          public PortableServer::RefCountServantBase
{
private:
    LibFred::Banking::StatementListPtr list_;

public:
    ccReg_Statements_i(LibFred::Banking::StatementList *list);
    ~ccReg_Statements_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    LibFred::Banking::Statement *findId(ccReg::TID id);
}; // class ccReg_PageTable_i


#endif	/* _PAGETABLE_STATEMENTS_H */

