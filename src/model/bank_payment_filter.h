#ifndef _BANK_PAYMENT_FILTER_H_
#define _BANK_PAYMENT_FILTER_H_

#include "db/query/base_filters.h"
#include "bank_statement_filter.h"

namespace Database {
namespace Filters {

class BankPayment:
    virtual public Compound {
public:
    virtual ~BankPayment()
    { }

    virtual Table &joinBankPaymentTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<std::string> &addAccountNumber() = 0;
    virtual Value<std::string> &addBankCode() = 0;
    virtual Value<int> &addCode() = 0;
    virtual Value<int> &addType() = 0;
    virtual Value<std::string> &addConstSymb() = 0;
    virtual Value<std::string> &addVarSymb() = 0;
    virtual Value<std::string> &addSpecSymb() = 0;
    virtual Value<std::string> &addAccountEvid() = 0;
    virtual Interval<Database::DateInterval> &addAccountDate() = 0;
    virtual Value<std::string> &addAccountName() = 0;
    virtual Interval<Database::DateTimeInterval> &addCrTime() = 0;
    virtual BankStatement &addBankStatement() = 0;
    virtual Value<std::string> &addAccountMemo() = 0;
    virtual Value<Database::ID> &addAccountId() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class BankPayment;

class BankPaymentImpl:
    virtual public BankPayment {
public:
    BankPaymentImpl();
    virtual ~BankPaymentImpl();

    virtual Table &joinBankPaymentTable();
    virtual Value<Database::ID> &addId();
    virtual Value<std::string> &addAccountNumber();
    virtual Value<std::string> &addBankCode();
    virtual Value<int> &addCode();
    virtual Value<int> &addType();
    virtual Value<std::string> &addConstSymb();
    virtual Value<std::string> &addVarSymb();
    virtual Value<std::string> &addSpecSymb();
    virtual Value<std::string> &addAccountEvid();
    virtual Interval<Database::DateInterval> &addAccountDate();
    virtual Value<std::string> &addAccountName();
    virtual Interval<Database::DateTimeInterval> &addCrTime();
    virtual BankStatement &addBankStatement();
    virtual Value<std::string> &addAccountMemo();
    virtual Value<Database::ID> &addAccountId();

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(BankPayment);
    }
}; // class BankPaymentImpl

} // namespace Filters
} // namespace Database

#endif // _BANK_PAYMENT_FILTER_H_
