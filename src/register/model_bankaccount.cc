#include "model_bankaccount.h"


std::string ModelBankAccount::table_name = "bank_account";

DEFINE_PRIMARY_KEY(ModelBankAccount, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelBankAccount, ModelZone, unsigned long long, zoneId, m_zoneId, table_name, "zone", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelBankAccount, std::string, accountNumber, m_accountNumber, table_name, "account_number", .setNotNull())
DEFINE_BASIC_FIELD(ModelBankAccount, std::string, accountName, m_accountName, table_name, "account_name", )
DEFINE_BASIC_FIELD(ModelBankAccount, std::string, bankCode, m_bankCode, table_name, "bank_code", )
DEFINE_BASIC_FIELD(ModelBankAccount, Database::Money, balance, m_balance, table_name, "balance", )
DEFINE_BASIC_FIELD(ModelBankAccount, Database::Date, lastDate, m_lastDate, table_name, "last_date", )
DEFINE_BASIC_FIELD(ModelBankAccount, int, lastNum, m_lastNum, table_name, "last_num", )

DEFINE_ONE_TO_ONE(ModelBankAccount, ModelZone, zone, m_zone, unsigned long long, zoneId, m_zoneId)

ModelBankAccount::field_list ModelBankAccount::fields = list_of<ModelBankAccount::field_list::value_type>
    (&ModelBankAccount::id)
    (&ModelBankAccount::zoneId)
    (&ModelBankAccount::accountNumber)
    (&ModelBankAccount::accountName)
    (&ModelBankAccount::bankCode)
    (&ModelBankAccount::balance)
    (&ModelBankAccount::lastDate)
    (&ModelBankAccount::lastNum);
