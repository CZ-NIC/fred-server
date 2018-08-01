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
    const std::string &getUuid() const
    {
        return ModelBankPayment::getUuid();
    }
    const std::string &getDestAccount() const
    {
        return dest_account;
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
    void setUuid(const std::string &uuid)
    {
        ModelBankPayment::setUuid(uuid);
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
        LOGGER(PACKAGE).error("Payment save: obsolete");
        throw SQL_ERROR();
    }

    void reload()
    {
        LOGGER(PACKAGE).error("Payment reload: obsolete");
        throw SQL_ERROR();
    }
};

typedef std::unique_ptr<PaymentImpl> PaymentImplPtr;

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

PaymentImplPtr make_importable_payment(
        const std::string& _uuid,
        unsigned long long _account_id,
        const std::string& _account_payment_ident,
        const std::string& _counter_account_number,
        const std::string& _counter_account_bank_code,
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
