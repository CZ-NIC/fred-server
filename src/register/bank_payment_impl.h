#ifndef BANK_PAYMENT_IMPL_H_
#define BANK_PAYMENT_IMPL_H_

#include "bank_common.h"
#include "bank_payment.h"
#include "common_impl_new.h"
#include "model_bank_payment.h"
#include "db_settings.h"

namespace Register {
namespace Banking {


class PaymentImpl : virtual public Payment,
                    public Register::CommonObjectImplNew,
                    private ModelBankPayment
{
private:
    unsigned long long iprefix_;


public:
    PaymentImpl() : ModelBankPayment()
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
    const Database::Money &getPrice() const
    {
        return ModelBankPayment::getPrice();
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
    const unsigned long long &getInvoiceId() const
    {
        return ModelBankPayment::getInvoiceId();
    }
    const std::string &getAccountName() const
    {
        return ModelBankPayment::getAccountName();
    }
    const Database::DateTime &getCrTime() const
    {
        return ModelBankPayment::getCrTime();
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
    void setPrice(const Database::Money &price)
    {
        ModelBankPayment::setPrice(price);
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
    void setInvoiceId(const unsigned long long &invoiceId)
    {
        ModelBankPayment::setInvoiceId(invoiceId);
    }
    void setAccountName(const std::string &accountName)
    {
        ModelBankPayment::setAccountName(accountName);
    }
    void setCrTime(const Database::DateTime &crTime)
    {
        ModelBankPayment::setCrTime(crTime);
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

    void save() throw (SQL_ERROR)
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

    void reload() throw (SQL_ERROR, NOT_FOUND)
    {
        try {
            ModelBankPayment::reload();
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
};

typedef std::auto_ptr<PaymentImpl> PaymentImplPtr;

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
COMPARE_CLASS_IMPL_NEW(PaymentImpl, InvoiceId);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, AccountName);
COMPARE_CLASS_IMPL_NEW(PaymentImpl, CrTime);


PaymentImplPtr parse_xml_payment_part(const XMLnode &_node)
{
    TRACE("[CALL] Register::Banking::payment_from_xml(...)");

    /* manual xml validation */
    if (!_node.hasChild(ITEM_IDENT)
            || !_node.hasChild(ITEM_ACCOUNT_NUMBER)
            || !_node.hasChild(ITEM_ACCOUNT_BANK_CODE)
            || !_node.hasChild(ITEM_CONST_SYMBOL)
            || !_node.hasChild(ITEM_VAR_SYMBOL)
            || !_node.hasChild(ITEM_SPEC_SYMBOL)
            || !_node.hasChild(ITEM_PRICE)
            || !_node.hasChild(ITEM_TYPE)
            || !_node.hasChild(ITEM_CODE)
            || !_node.hasChild(ITEM_MEMO)
            || !_node.hasChild(ITEM_DATE)
            || !_node.hasChild(ITEM_CRTIME)
            || !_node.hasChild(ITEM_NAME)) {

        throw std::runtime_error("not valid payment xml");
    }
    PaymentImplPtr payment(new PaymentImpl());
    if (!_node.getChild(ITEM_IDENT).isEmpty()) {
        std::string value = _node.getChild(ITEM_IDENT).getValue();
        payment->setAccountEvid(value);
    }
    else {
        throw std::runtime_error("no valid payment xml; "
                "cannot identify payment with no accountevid");
    }
    // if (!_node.getChild(ITEM_ACCOUNT_NUMBER).isEmpty()) {
    {
        std::string value = _node.getChild(ITEM_ACCOUNT_NUMBER).getValue();
        payment->setAccountNumber(value);
    }
    // }
    // if (!_node.getChild(ITEM_ACCOUNT_BANK_CODE).isEmpty()) {
    {
        std::string value = _node.getChild(ITEM_ACCOUNT_BANK_CODE).getValue();
        payment->setBankCode(value);
    }
        /* test if is in databaase? */
    // }
    // if (!_node.getChild(ITEM_PRICE).isEmpty()) {
    {
        Database::Money value;
        value.format(_node.getChild(ITEM_PRICE).getValue());
        payment->setPrice(value);
    }
    // }

    if (!_node.getChild(ITEM_CONST_SYMBOL).isEmpty()) {
        std::string value = _node.getChild(ITEM_CONST_SYMBOL).getValue();
        payment->setKonstSym(value);
    }
    if (!_node.getChild(ITEM_VAR_SYMBOL).isEmpty()) {
        std::string value = _node.getChild(ITEM_VAR_SYMBOL).getValue();
        payment->setVarSymb(value);
    }
    if (!_node.getChild(ITEM_SPEC_SYMBOL).isEmpty()) {
        std::string value = _node.getChild(ITEM_SPEC_SYMBOL).getValue();
        payment->setSpecSymb(value);
    }
    if (!_node.getChild(ITEM_MEMO).isEmpty()) {
        std::string value = _node.getChild(ITEM_MEMO).getValue();
        payment->setAccountMemo(value);
    }
    if (!_node.getChild(ITEM_DATE).isEmpty()) {
        Database::Date value = _node.getChild(ITEM_DATE).getValue();
        payment->setAccountDate(value);
    }
    if (!_node.getChild(ITEM_CRTIME).isEmpty()) {
        Database::DateTime value = _node.getChild(ITEM_CRTIME).getValue();
        payment->setCrTime(value);
    }
    if (!_node.getChild(ITEM_NAME).isEmpty()) {
        std::string value = _node.getChild(ITEM_NAME).getValue();
        payment->setAccountName(value);
    }


    if (!_node.getChild(ITEM_TYPE).isEmpty()) {
        int value = atoi(_node.getChild(ITEM_TYPE).getValue().c_str());
        payment->setType(value);
    }
    if (!_node.getChild(ITEM_CODE).isEmpty()) {
        int value = atoi(_node.getChild(ITEM_CODE).getValue().c_str());
        payment->setCode(value);
    }
    if (!_node.getChild(ITEM_STATUS).isEmpty()) {
        int value = atoi(_node.getChild(ITEM_STATUS).getValue().c_str());
        payment->setStatus(value);
    }

    return payment;
}

}
}

#endif /*BANK_PAYMENT_IMPL_H_*/

