#include "model_bankstatementhead.h"

std::string ModelBankStatementHead::table_name = "bank_head";

DEFINE_PRIMARY_KEY(ModelBankStatementHead, unsigned long long, id, m_id, table_name, "id", .setDefault())
//DEFINE_FOREIGN_KEY(ModelBankStatementHead, ModelBankAccount, unsigned long long, accountId, m_accountId, table_name, "account_id", id, )
DEFINE_BASIC_FIELD(ModelBankStatementHead, int, num, m_num, table_name, "num", )
DEFINE_BASIC_FIELD(ModelBankStatementHead, Database::Date, createDate, m_createDate, table_name, "create_date", .setDefault())
DEFINE_BASIC_FIELD(ModelBankStatementHead, Database::Date, balanceOldDate, m_balanceOldDate, table_name, "balance_old_date", )
DEFINE_BASIC_FIELD(ModelBankStatementHead, Database::Money, balanceOld, m_balanceOld, table_name, "balance_old", )
DEFINE_BASIC_FIELD(ModelBankStatementHead, Database::Money, balanceNew, m_balanceNew, table_name, "balance_new", )
DEFINE_BASIC_FIELD(ModelBankStatementHead, Database::Money, balanceCredit, m_balanceCredit, table_name, "balance_credit", )
DEFINE_BASIC_FIELD(ModelBankStatementHead, Database::Money, balanceDebet, m_balanceDebet, table_name, "balance_debet", )
//DEFINE_FOREIGN_KEY(ModelBankStatementHead, ModelFiles, unsigned long long, fileId, m_fileId, table_name, "file_id", id, )

//DEFINE_ONE_TO_ONE(ModelBankStatementHead, ModelBankAccount, account, m_account, unsigned long long, accountId, m_accountId)
//DEFINE_ONE_TO_ONE(ModelBankStatementHead, ModelFiles, file, m_file, unsigned long long, fileId, m_fileId)

ModelBankStatementHead::field_list ModelBankStatementHead::fields = list_of<ModelBankStatementHead::field_list::value_type>
    (&ModelBankStatementHead::id)
    //(&ModelBankStatementHead::accountId)
    (&ModelBankStatementHead::num)
    (&ModelBankStatementHead::createDate)
    (&ModelBankStatementHead::balanceOldDate)
    (&ModelBankStatementHead::balanceOld)
    (&ModelBankStatementHead::balanceNew)
    (&ModelBankStatementHead::balanceCredit)
    (&ModelBankStatementHead::balanceDebet)
    //(&ModelBankStatementHead::fileId)
;

