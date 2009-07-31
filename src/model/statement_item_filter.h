#ifndef _STATEMENT_ITEM_FILTER_H_
#define _STATEMENT_ITEM_FILTER_H_

#include "db/query/base_filters.h"
#include "statement_head_filter.h"

namespace Database {
namespace Filters {

class StatementItem:
    virtual public Compound {
public:
    virtual ~StatementItem()
    { }

    virtual Table &joinStatementItemTable() = 0;
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
    virtual Value<Database::ID> &addInvoiceId() = 0;
    virtual Value<std::string> &addAccountName() = 0;
    virtual Interval<Database::DateTimeInterval> &addCrTime() = 0;
    virtual StatementHead &addStatementHead() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class StatementItem;

class StatementItemImpl:
    virtual public StatementItem {
public:
    StatementItemImpl();
    virtual ~StatementItemImpl();

    virtual Table &joinStatementItemTable();
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
    virtual Value<Database::ID> &addInvoiceId();
    virtual Value<std::string> &addAccountName();
    virtual Interval<Database::DateTimeInterval> &addCrTime();
    virtual StatementHead &addStatementHead();

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(StatementItem);
    }
}; // class StatementItemImpl

} // namespace Filters
} // namespace Database

#endif // _STATEMENT_ITEM_FILTER_H_
