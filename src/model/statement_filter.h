#ifndef _STATEMENT_FILTER_H_
#define _STATEMENT_FILTER_H_

#include "db/query/base_filters.h"

namespace Database {
namespace Filters {

class StatementHead: virtual public Compound {
public:
    virtual ~StatementHead()
    { }

    virtual Table &joinStatementHeadTable() = 0;
    virtual Table &joinStatementItemTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<Database::ID> &addAccountId() = 0;
    virtual Interval<Database::DateInterval> &addCreateDate() = 0;
    virtual Interval<Database::DateInterval> &addBalanceOldDate() = 0;
    // TODO maybe other items from bank_head and bank_item
    virtual Value<std::string> &addAccountNumber() = 0;
    virtual Value<std::string> &addBankCode() = 0;
    virtual Value<std::string> &addConstSymbol() = 0;
    virtual Value<std::string> &addVarSymbol() = 0;
    virtual Value<std::string> &addSpecSymbol() = 0;
    virtual Value<Database::ID> &addInvoiceId() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class StatementHead

class StatementHeadImpl: virtual public StatementHead {
public:
    StatementHeadImpl();
    virtual ~StatementHeadImpl();

    virtual Table &joinStatementHeadTable();
    virtual Table &joinStatementItemTable();
    virtual Value<Database::ID> &addId();
    virtual Value<Database::ID> &addAccountId();
    virtual Interval<Database::DateInterval> &addCreateDate();
    virtual Interval<Database::DateInterval> &addBalanceOldDate();
    virtual Value<std::string> &addAccountNumber();
    virtual Value<std::string> &addBankCode();
    virtual Value<std::string> &addConstSymbol();
    virtual Value<std::string> &addVarSymbol();
    virtual Value<std::string> &addSpecSymbol();
    virtual Value<Database::ID> &addInvoiceId();

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(StatementHead);
    }
}; // class StatementImpl

} // namespace Filtes
} // namespace Database

#endif // _STATEMENT_FILTER_H_

