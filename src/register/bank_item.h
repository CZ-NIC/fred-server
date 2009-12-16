#ifndef _BANK_STATEMENT_ITEM_H_
#define _BANK_STATEMENT_ITEM_H_

#include "common_impl_new.h"
#include "db_settings.h"
#include "model_bankstatementitem.h"
#include "bank_common.h"

namespace Register {
namespace Banking {

class StatementItem:
    public ModelBankStatementItem,
    virtual public Register::CommonObjectImplNew {
private:
    bool moveItem(const Database::ID &paymentId);
    void testBankCode();
    Database::ID getExisting();
public:
    bool fromXML(const XMLnode &item);
    bool save();
};

COMPARE_CLASS_IMPL_NEW(StatementItem, Id);
COMPARE_CLASS_IMPL_NEW(StatementItem, StatementId);
COMPARE_CLASS_IMPL_NEW(StatementItem, AccountNumber);
COMPARE_CLASS_IMPL_NEW(StatementItem, BankCodeId);
COMPARE_CLASS_IMPL_NEW(StatementItem, Type);
COMPARE_CLASS_IMPL_NEW(StatementItem, Code);
COMPARE_CLASS_IMPL_NEW(StatementItem, KonstSym);
COMPARE_CLASS_IMPL_NEW(StatementItem, VarSymb);
COMPARE_CLASS_IMPL_NEW(StatementItem, SpecSymb);
COMPARE_CLASS_IMPL_NEW(StatementItem, Price);
COMPARE_CLASS_IMPL_NEW(StatementItem, AccountEvid);
COMPARE_CLASS_IMPL_NEW(StatementItem, AccountDate);
COMPARE_CLASS_IMPL_NEW(StatementItem, AccountMemo);
COMPARE_CLASS_IMPL_NEW(StatementItem, InvoiceId);
COMPARE_CLASS_IMPL_NEW(StatementItem, AccountName);
COMPARE_CLASS_IMPL_NEW(StatementItem, CrTime);

} // namespace Banking
} // namespace Register;

#endif // _BANK_STATEMENT_ITEM_H_
