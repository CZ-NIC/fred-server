#ifndef BANK_MANAGER_HH_5C8C2E1D0B944AA19ED43A94CE1AE7FC
#define BANK_MANAGER_HH_5C8C2E1D0B944AA19ED43A94CE1AE7FC

#include "src/libfred/banking/invoice_reference.hh"
#include "src/libfred/banking/payment_invoices.hh"
#include "src/libfred/db_settings.hh"
#include "src/libfred/file.hh"
#include "src/util/types/money.hh"

#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/gregorian/gregorian_types.hpp>
#include <boost/optional.hpp>

namespace LibFred {
namespace Banking {


class Manager {
public:
    static Manager *create(File::Manager *_file_manager);
    static Manager *create();

    virtual void addBankAccount(
            const std::string &_account_number,
            const std::string &_bank_code,
            const std::string &_zone,
            const std::string &_account_name) //throw (std::runtime_error)
                = 0;

    virtual PaymentInvoices importPayment(
            const std::string& _uuid,
            const std::string& _account_number,
            const std::string& _account_bank_code,
            const std::string& _account_payment_ident,
            const std::string& _counter_account_number,
            const std::string& _counter_account_bank_code,
            const std::string& _counter_account_name,
            const std::string& _constant_symbol,
            const std::string& _variable_symbol,
            const std::string& _specific_symbol,
            const Money& _price,
            const boost::gregorian::date _date,
            const std::string& _memo,
            const boost::posix_time::ptime _creation_time,
            const boost::optional<std::string>& _registrar_handle)
                = 0;

}; // class Manager


// smart pointer
typedef std::unique_ptr<Manager> ManagerPtr;


} // namespace Banking
} // namespace LibFred

#endif
