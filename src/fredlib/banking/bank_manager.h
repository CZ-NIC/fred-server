#ifndef _BANK_MANAGER_H_
#define _BANK_MANAGER_H_

#include "bank_payment.h"
#include "bank_payment_list.h"
#include "bank_statement.h"
#include "bank_statement_list.h"
#include "file.h"
#include "db_settings.h"

namespace Fred {
namespace Banking {


class Manager {
public:
    virtual StatementList *createStatementList() const = 0;
    virtual PaymentList *createPaymentList() const = 0;
    static Manager *create(File::Manager *_file_manager);

    virtual void addBankAccount(
            const std::string &_account_number,
            const std::string &_bank_code,
            const std::string &_zone,
            const std::string &_account_name) //throw (std::runtime_error)
                = 0;

    virtual void importStatementXml(
            std::istream &_in,
            const std::string &_file_path,
            const std::string &_file_mime,
            const bool &_generate_invoices = false) //throw (std::runtime_error)
                = 0;

    virtual bool pairPaymentWithStatement(
            const Database::ID &payment,
            const Database::ID &statement,
            bool force = false) = 0;

    virtual bool pairPaymentWithRegistrar(
            const Database::ID &paymentId,
            const std::string &registrarHandle) = 0;

    virtual void setPaymentType(
            const Database::ID &payment,
            const int &type) = 0;

}; // class Manager


// smart pointer
typedef std::auto_ptr<Manager> ManagerPtr;


} // namespace Banking
} // namespace Fred

#endif // _BANK_MANAGER_H_
