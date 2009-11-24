#ifndef _PAGETABLE_STATEMENT_ITEMS_H_
#define _PAGETABLE_STATEMENT_ITEMS_H_

#include "pagetable_impl.h"
#include "register/bank_item_list.h"

class ccReg_StatementItems_i:
    public ccReg_PageTable_i,
    public PortableServer::RefCountServantBase {
private:
    std::auto_ptr<Register::Banking::ItemList> m_statementItemList;
public:
    ccReg_StatementItems_i(Register::Banking::ItemList *itemList);
    ~ccReg_StatementItems_i();
    DECL_PAGETABLE_I;

    ccReg::Filters::Compound_ptr add();
    Register::Banking::StatementItem *findId(ccReg::TID id);
}; // class ccReg_PageTable_i

#endif // _PAGETABLE_STATEMENT_ITEMS_H_
