#ifndef _BANK_MANAGER_H_
#define _BANK_MANAGER_H_

#include "bank_payment.h"
#include "bank_payment_list.h"
#include "bank_statement.h"
#include "bank_statement_list.h"
#include "db_settings.h"

namespace Register {
namespace Banking {


class Manager {
public:
    virtual StatementList *createStatementList() const = 0;
    virtual PaymentList *createPaymentList() const = 0;
    static Manager *create();

    virtual bool importStatementXml(
            std::istream &in,
            const std::string &path,
            const bool &createCreditInvoice = false) = 0;

    virtual bool insertBankAccount(
            const std::string &zone,
            const std::string &account_number,
            const std::string &account_name,
            const std::string &bank_code) = 0;

    virtual bool insertBankAccount(
            const Database::ID &zone,
            const std::string &account_number,
            const std::string &account_name,
            const std::string &bank_code) = 0;

    virtual bool moveItemToPayment(
            const Database::ID &payment,
            const Database::ID &statement,
            bool force = false) = 0;

    virtual bool pairPaymentWithStatement(
            const Database::ID &payment,
            const Database::ID &statement,
            bool force = false) = 0;

}; // class Manager


// smart pointer
typedef std::auto_ptr<Manager> ManagerPtr;


} // namespace Banking
} // namespace Register

#endif // _BANK_MANAGER_H_
