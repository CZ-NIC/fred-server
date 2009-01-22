#ifndef _STATEMENT_FILTER_H_
#define _STATEMENT_FILTER_H_

#include "db/base_filters.h"

namespace Database {
namespace Filters {

class Statement: virtual public Compound {
public:
    virtual ~Statement() 
    { }

    virtual Table &joinStatementTable() = 0;
    virtual Table &joinStatementItemTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<Database::ID> &addAccountId() = 0;
    virtual Interval<Database::DateInterval> &addCreateDate() = 0;
    virtual Interval<Database::DateInterval> &addBalanceOldDate() = 0;
    // TODO maybe other items from bank_statement_head and bank_statement_item
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
}; // class Statement

class StatementImpl: virtual public Statement {
public:
    StatementImpl();
    virtual ~StatementImpl();

    virtual Table &joinStatementTable();
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
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Statement);
    }
}; // class StatementImpl

} // namespace Filtes
} // namespace Database

#endif // _STATEMENT_FILTER_H_

