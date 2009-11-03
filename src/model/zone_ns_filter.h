#ifndef _ZONE_NS_FILTER_H_
#define _ZONE_NS_FILTER_H_

#include "db/query/base_filters.h"

namespace Database {
namespace Filters {

class ZoneNs:
    virtual public Compound {
public:
    virtual ~ZoneNs()
    { }

    virtual Table &joinZoneNsTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<Database::ID> &addZoneId() = 0;
    virtual Value<std::string> &addFqdn() = 0;
    virtual Value<std::string> &addAddrs() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class ZoneNs

class ZoneNsImpl:
    virtual public ZoneNs {
public:
    ZoneNsImpl(bool set_active = false);
    virtual ~ZoneNsImpl();

    virtual Table &joinZoneNsTable();
    virtual Value<Database::ID> &addId();
    virtual Value<Database::ID> &addZoneId();
    virtual Value<std::string> &addFqdn();
    virtual Value<std::string> &addAddrs();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version) 
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(ZoneNs);
    }
}; // class ZoneNsImpl

} // namespace Filters
} // namespace Database

#endif // _ZONE_NS_FILTER_H_
