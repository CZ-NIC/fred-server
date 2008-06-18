#ifndef KEYSET_FILTER_H_
#define KEYSET_FILTER_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>

#include "object_filter.h"
#include "contact_filter.h"
#include "domain_filter.h"

namespace DBase {
namespace Filters {

class KeySet: virtual public Object {
public:
    virtual ~KeySet() { }

    virtual Table       &joinKeySetTable() = 0;
    virtual Value<ID>   &addId() = 0;
    virtual Contact     &addTechContact() = 0;
    virtual Domain      &addDomain() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &_ar,
            const unsigned int _version) {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Object);
    }
};

class KeySetImpl: public ObjectImpl, virtual public KeySet {
public:
    KeySetImpl();
    virtual ~KeySetImpl();

    virtual ObjectType getType() {
        return TKEYSET;
    }

    virtual Table       &joinKeySetTable();
    virtual void        _joinPolymorphicTables();

    virtual Value<ID>   &addId();
    virtual Contact     &addTechContact();
    virtual Domain      &addDomain();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &_ar,
            const unsigned int _version) {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(KeySet);
    }
};

class KeySetHistoryImpl: public ObjectHistoryImpl, virtual public KeySet {
    public:
        KeySetHistoryImpl();
        virtual ~KeySetHistoryImpl();

        virtual ObjectType getType() {
            return TKEYSET;
        }

    virtual Table       &joinKeySetTable();
    virtual void        _joinPolymorphicTables();

    virtual Value<ID>   &addId();
    virtual Contact     &addTechContact();
    virtual Domain      &addDomain();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive &_ar,
            const unsigned int _version) {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(KeySet);
    }
};

} // namespace Filters
} // namespace DBase

#endif // KEYSET_FILTER_H_
