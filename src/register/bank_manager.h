#ifndef _BANK_MANAGER_H_
#define _BANK_MANAGER_H_

#include "bank_head.h"
#include "bank_head_list.h"
#include "bank_item_list.h"
#include "db_settings.h"

namespace Register {
namespace Banking {

class Manager {
private:
public:
    HeadList *createList() const;
    ItemList *createItemList() const;
    static Manager *create();
    StatementHead *createStatement() const;
    bool importStatementXml(
            std::istream &in,
            const unsigned long long &fileId = 0,
            const bool &createCreditInvoice = false);
    bool insertBankAccount(
            const std::string &zone,
            const std::string &account_number,
            const std::string &account_name,
            const std::string &bank_code);
    bool insertBankAccount(
            const Database::ID &zone,
            const std::string &account_number,
            const std::string &account_name,
            const std::string &bank_code);
    bool moveItemToPayment(
            const Database::ID &paymentItem,
            const Database::ID &paymentHead,
            bool force = false);
}; // class Manager

} // namespace Banking
} // namespace Register

#endif // _BANK_MANAGER_H_
