#ifndef BANK_PAYMENT_IMPL_HH_F11C942347B648A4A9EDA7A6E3495B32
#define BANK_PAYMENT_IMPL_HH_F11C942347B648A4A9EDA7A6E3495B32

#include "src/libfred/banking/bank_payment.hh"
#include "src/libfred/common_impl_new.hh"
#include "src/libfred/banking/model_bank_payment.hh"
#include "src/libfred/db_settings.hh"
#include "src/util/types/money.hh"

namespace LibFred {
namespace Banking {

class PaymentImpl : virtual public Payment,
                    public LibFred::CommonObjectImplNew,
                    private ModelBankPayment
{
private:
    unsigned long long iprefix_;
    unsigned long long advance_invoice_id_;
    std::string dest_account;

public:
    PaymentImpl()
    : ModelBankPayment()
    , iprefix_(0)
    , advance_invoice_id_(0)
    {
    }

    const unsigned long long &getId() const
    {
        return ModelBankPayment::getId();
    }
    const unsigned long long &getStatementId() const
    {
        return ModelBankPayment::getStatementId();
    }
    const unsigned long long &getAccountId() const
    {
        return ModelBankPayment::getAccountId();
    }
    const std::string &getAccountNumber() const
    {
        return ModelBankPayment::getAccountNumber();
    }
    const std::string &getBankCode() const
    {
        return ModelBankPayment::getBankCodeId();
    }
    const int &getCode() const
    {
        return ModelBankPayment::getCode();
    }
    const int &getType() const
    {
        return ModelBankPayment::getType();
    }
    const int &getStatus() const
    {
        return ModelBankPayment::getStatus();
    }
    const std::string &getKonstSym() const
    {
        return ModelBankPayment::getKonstSym();
    }
    const std::string &getVarSymb() const
    {
        return ModelBankPayment::getVarSymb();
    }
    const std::string &getSpecSymb() const
    {
        return ModelBankPayment::getSpecSymb();
    }
    Money getPrice() const
    {
        return Money(ModelBankPayment::getPrice());
    }
    const std::string &getAccountEvid() const
    {
        return ModelBankPayment::getAccountEvid();
    }
    const Database::Date &getAccountDate() const
    {
        return ModelBankPayment::getAccountDate();
    }
    const std::string &getAccountMemo() const
    {
        return ModelBankPayment::getAccountMemo();
    }
    const unsigned long long &getAdvanceInvoiceId() const
    {
        return advance_invoice_id_;
    }
    const std::string &getAccountName() const
    {
        return ModelBankPayment::getAccountName();
    }
    const Database::DateTime &getCrTime() const
    {
        return ModelBankPayment::getCrTime();
    }
    const std::string &getDestAccount() const
    {
        return dest_account;
    }

    void setId(const unsigned long long &id)
    {
        ModelBankPayment::setId(id);
    }
    void setStatementId(const unsigned long long &statementId)
    {
        ModelBankPayment::setStatementId(statementId);
    }
    void setAccountId(const unsigned long long &accountId)
    {
        ModelBankPayment::setAccountId(accountId);
    }
    void setAccountNumber(const std::string &accountNumber)
    {
        ModelBankPayment::setAccountNumber(accountNumber);
    }
    void setBankCode(const std::string &bankCode)
    {
        ModelBankPayment::setBankCodeId(bankCode);
    }
    void setCode(const int &code)
    {
        ModelBankPayment::setCode(code);
    }
    void setType(const int &type)
    {
        ModelBankPayment::setType(type);
    }
    void setStatus(const int &status)
    {
        ModelBankPayment::setStatus(status);
    }
    void setKonstSym(const std::string &konstSym)
    {
        ModelBankPayment::setKonstSym(konstSym);
    }
    void setVarSymb(const std::string &varSymb)
    {
        ModelBankPayment::setVarSymb(varSymb);
    }
    void setSpecSymb(const std::string &specSymb)
    {
        ModelBankPayment::setSpecSymb(specSymb);
    }
    void setPrice(const Money &price)
    {
        ModelBankPayment::setPrice(price.get_string());
    }
    void setAccountEvid(const std::string &accountEvid)
    {
        ModelBankPayment::setAccountEvid(accountEvid);
    }
    void setAccountDate(const Database::Date &accountDate)
    {
        ModelBankPayment::setAccountDate(accountDate);
    }
    void setAccountMemo(const std::string &accountMemo)
    {
        ModelBankPayment::setAccountMemo(accountMemo);
    }
    void setAdvanceInvoiceId(const unsigned long long &invoiceId)
    {
        advance_invoice_id_ = invoiceId;
    }
    void setAccountName(const std::string &accountName)
    {
        ModelBankPayment::setAccountName(accountName);
    }
    void setCrTime(const Database::DateTime &crTime)
    {
        ModelBankPayment::setCrTime(crTime);
    }
    void setDestAccount(const std::string &destAccount)
    {
        dest_account=destAccount;
    }


    unsigned long long getInvoicePrefix() const
    {
        return iprefix_;
    }

    void setInvoicePrefix(const unsigned long long &_iprefix)
    {
        iprefix_ = _iprefix;
    }


    std::string toString() const
    {
        return ModelBankPayment::toString(); 
    }

    void save()
    {
        try {
            if (getId() == 0) {
                ModelBankPayment::insert();
            }
            else {
                ModelBankPayment::update();
            }
        }
        catch (std::exception &ex) {
            LOGGER(PACKAGE).error(
                    boost::format("Payment save: %1%") % ex.what());
            throw SQL_ERROR();
        }
        catch (...) {
            LOGGER(PACKAGE).error("Payment save: unknown exception");
            throw SQL_ERROR();
        }
    }

    void reload()
    {
        try {
            ModelBankPayment::reload();
            Database::Connection conn = Database::Manager::acquire();
            Database::Result dest_account_result
                = conn.exec_params("select account_name || ' ' || account_number || '/' || bank_code"
                " from bank_account where id = $1::integer "
                ,Database::query_param_list(ModelBankPayment::getAccountId()));

            if(dest_account_result.size() == 1)
            {
                dest_account = std::string(dest_account_result[0][0]);
            }
            else
            {
                throw std::runtime_error(
                        "PaymentImpl::reload unable to get account");
            }
        }
        catch (Database::NoDataFound &ex) {
            throw NOT_FOUND();
        }
        catch (...) {
            throw SQL_ERROR();
        }
    }

    Database::ID getConflictId()
    {
        if (getAccountDate().is_special() || getAccountId() == 0)
            return Database::ID(0);

        std::stringstream where_crtime("crtime", std::ios::out | std::ios::app);
        if (!getCrTime().is_special()) {
            where_crtime << " = " << Database::Value(getCrTime());
        }
        else {
            where_crtime << " IS NULL";
        }

        Database::Query query;
        query.buffer()
            << "SELECT id FROM bank_payment "
            << "WHERE account_id = " << Database::Value(getAccountId())
            << " AND account_evid = " << Database::Value(getAccountEvid());

        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec(query);
        if (result.size() == 0) {
            /* not found */
            return Database::ID(0);
        }
        else if (result.size() == 1) {
            /* found one */
            return result[0][0];
        }
        else {
            /* found multiple - should not happended */
            throw std::runtime_error(str(boost::format(
                        "ooops! found multiple conflict payments for "
                        "payment: %1%") % toString()));
        }
    }

    bool is_eligible_to_process() const
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result have_zone_result = conn.exec_params
            ("SELECT zone IS NOT NULL FROM bank_account  WHERE id =$1::bigint"
            , Database::query_param_list(getAccountId()));

        bool bank_account_have_zone = false;
        if (have_zone_result.size() == 1 ) bank_account_have_zone = have_zone_result[0][0];

        return bank_account_have_zone && (getStatus() == 1)
                && (getCode() == 1) && (getType() == 1);
    }

};

typedef std::unique_ptr<PaymentImpl> PaymentImplPtr;

COMPARE_CLASS_IMPL_NEW(PaymentImpl, Id);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, StatementId);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, AccountNumber);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, BankCode);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, Type);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, Code);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, KonstSym);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, VarSymb);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, SpecSymb);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, Price);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, AccountEvid);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, AccountDate);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, AccountMemo);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, AdvanceInvoiceId);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, AccountName);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, CrTime);

COMPARE_CLASS_IMPL_NEW(PaymentImpl, DestAccount)


PaymentImplPtr parse_xml_payment_part(const XMLnode &_node);

PaymentImplPtr payment_from_params(
        const std::string& _bank_payment,
        const std::string& _uuid,
        const std::string& _account_number,
        const std::string& _counter_account_number,
        const std::string& _counter_account_name,
        const std::string& _constant_symbol,
        const std::string& _variable_symbol,
        const std::string& _specific_symbol,
        const Money& _price,
        const boost::gregorian::date& _date,
        const std::string& _memo,
        const boost::posix_time::ptime& _creation_time);


EnumList getBankAccounts();

} // namespace Banking
} // namespace LibFred


#endif /*BANK_PAYMENT_IMPL_H_*/
