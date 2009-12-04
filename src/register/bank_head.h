#ifndef _BANK_STATEMENT_HEAD_H_
#define _BANK_STATEMENT_HEAD_H_

#include <vector>

#include "common_impl_new.h"
#include "db_settings.h"
#include "model_bankstatementhead.h"
#include "bank_item.h"
#include "bank_common.h"
#include "bank_exporter.h"

namespace Register {
namespace Banking {

class StatementHead:
    public ModelBankStatementHead,
    virtual public Register::CommonObjectImplNew {
private:
    typedef std::vector<StatementItem>  StatementItemListType;
    StatementItemListType   m_statementItems;

    bool isCsvHeader();
    Database::ID getConflictId();
    bool isExisting();
    bool updateBankAccount();
    Database::ID getBankNumberId(
            const std::string &account_num,
            const std::string &bank_code);
    Database::ID getBankNumberId(const std::string &account_num);
    bool deleteHead();
public:
    void doExport(Exporter *exp);
    bool fromXML(
            const XMLnode &node, 
            const unsigned long long &fileId = 0);
    bool save();
    StatementItem *createStatementItem();
    unsigned int getStatementItemCount() const ;
    const StatementItem *getStatementItemByIdx(const unsigned int &idx) const;
    StatementItem *newStatementItem();
    StatementItem *addStatementItem(const StatementItem &statementItem);
    //const std::string &getZoneFqdn();
    //const std::string &getAccountNumber();
    //const std::string &getBankCode();
};

COMPARE_CLASS_IMPL_NEW(StatementHead, CreateDate);
COMPARE_CLASS_IMPL_NEW(StatementHead, BalanceOldDate);
COMPARE_CLASS_IMPL_NEW(StatementHead, BalanceOld);
COMPARE_CLASS_IMPL_NEW(StatementHead, BalanceNew);
COMPARE_CLASS_IMPL_NEW(StatementHead, BalanceCredit);
COMPARE_CLASS_IMPL_NEW(StatementHead, BalanceDebet);
// COMPARE_CLASS_IMPL_NEW(StatementHead, ZoneFqdn);
// COMPARE_CLASS_IMPL_NEW(StatementHead, AccountNumber);
// COMPARE_CLASS_IMPL_NEW(StatementHead, BankCode);

} // namespace Banking
} // namespace Register

#endif // _BANK_STATEMENT_HEAD_H_
