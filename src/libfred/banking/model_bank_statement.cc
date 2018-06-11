#include "src/libfred/banking/model_bank_statement.hh"

std::string ModelBankStatement::table_name = "bank_statement";

DEFINE_PRIMARY_KEY(ModelBankStatement, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelBankStatement, unsigned long long, accountId, m_accountId, table_name, "account_id", .setForeignKey())
DEFINE_BASIC_FIELD(ModelBankStatement, int, num, m_num, table_name, "num", )
DEFINE_BASIC_FIELD(ModelBankStatement, Database::Date, createDate, m_createDate, table_name, "create_date", .setDefault())
DEFINE_BASIC_FIELD(ModelBankStatement, Database::Date, balanceOldDate, m_balanceOldDate, table_name, "balance_old_date", )
DEFINE_BASIC_FIELD(ModelBankStatement, std::string, balanceOld, m_balanceOld, table_name, "balance_old", )
DEFINE_BASIC_FIELD(ModelBankStatement, std::string, balanceNew, m_balanceNew, table_name, "balance_new", )
DEFINE_BASIC_FIELD(ModelBankStatement, std::string, balanceCredit, m_balanceCredit, table_name, "balance_credit", )
DEFINE_BASIC_FIELD(ModelBankStatement, std::string, balanceDebet, m_balanceDebet, table_name, "balance_debet", )
DEFINE_BASIC_FIELD(ModelBankStatement, unsigned long long, fileId, m_fileId, table_name, "file_id", .setForeignKey())

//DEFINE_ONE_TO_ONE(ModelBankStatement, ModelBankAccount, account, m_account, unsigned long long, accountId, m_accountId)

ModelBankStatement::field_list ModelBankStatement::fields = list_of<ModelBankStatement::field_list::value_type>
    (&ModelBankStatement::id)
    (&ModelBankStatement::accountId)
    (&ModelBankStatement::num)
    (&ModelBankStatement::createDate)
    (&ModelBankStatement::balanceOldDate)
    (&ModelBankStatement::balanceOld)
    (&ModelBankStatement::balanceNew)
    (&ModelBankStatement::balanceCredit)
    (&ModelBankStatement::balanceDebet)
    (&ModelBankStatement::fileId)
;

