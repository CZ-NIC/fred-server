#ifndef _STATEMENT_HEAD_FILTER_H_
#define _STATEMENT_HEAD_FILTER_H_

#include "db/query/base_filters.h"

namespace Database {
namespace Filters {

class StatementHead:
    virtual public Compound {
public:
    virtual ~StatementHead()
    { }

    virtual Table &joinStatementHeadTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<int> &addNumber() = 0;
    virtual Interval<Database::DateInterval> &addCreateDate() = 0;
    virtual Interval<Database::DateInterval> &addBalanceOldDate() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class StatementHead

class StatementHeadImpl:
    virtual public StatementHead {
public:
    StatementHeadImpl();
    virtual ~StatementHeadImpl();

    virtual Table &joinStatementHeadTable();
    virtual Value<Database::ID> &addId();
    virtual Value<int> &addNumber();
    virtual Interval<Database::DateInterval> &addCreateDate();
    virtual Interval<Database::DateInterval> &addBalanceOldDate();

    friend class boost::serialization::access;
    template<class Archive> void serialize(
            Archive &_ar, const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(StatementHead);
    }
}; // class StatementHeadImpl

} // namespace Filters
} // namespace Database

#endif // _STATEMENT_HEAD_FILTER_H_
