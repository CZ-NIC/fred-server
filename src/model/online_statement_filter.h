#ifndef _ONLINE_STATATEMENT_FILTER_H_
#define _ONLINE_STATATEMENT_FILTER_H_

#include "db/query/base_filters.h"

namespace Database {
namespace Filters {

class OnlineStatement: virtual public Compound {
public:
    virtual ~OnlineStatement() 
    { }

    virtual Table &joinOnlineStatementTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<Database::ID> &addAccountId() = 0;
    virtual Interval<Database::DateTimeInterval> &addCreateTime() = 0;
    virtual Value<std::string> &addAccountNumber() = 0;
    virtual Value<std::string> &addBankCode() = 0;
    virtual Value<std::string> &addConstSymbol() = 0;
    virtual Value<std::string> &addVarSymbol() = 0;
    virtual Value<std::string> &addAccountName() = 0;
    virtual Value<std::string> &addIdentifier() = 0;
    // TODO maybe other items from bank_ebanka_list

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class OnlineStatement

class OnlineStatementImpl: virtual public OnlineStatement {
public:
    OnlineStatementImpl();
    virtual ~OnlineStatementImpl();

    virtual Table &joinOnlineStatementTable();
    virtual Value<Database::ID> &addId();
    virtual Value<Database::ID> &addAccountId();
    virtual Interval<Database::DateTimeInterval> &addCreateTime();
    virtual Value<std::string> &addAccountNumber();
    virtual Value<std::string> &addBankCode();
    virtual Value<std::string> &addConstSymbol();
    virtual Value<std::string> &addVarSymbol();
    virtual Value<std::string> &addAccountName();
    virtual Value<std::string> &addIdentifier();

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(OnlineStatement);
    }
}; // class OnlineStatementImpl

} // namespace Filters
} // namespace Database

#endif // _ONLINE_STATATEMENT_FILTER_H_

