#ifndef _ZONE_FILTER_H_
#define _ZONE_FILTER_H_

#include "db/query/base_filters.h"

#include "zone_ns_filter.h"


namespace Database {
namespace Filters {

class Zone:
    virtual public Compound {
public:
    virtual ~Zone()
    { }

    virtual Table &joinZoneTable() = 0;
    virtual Value<Database::ID> &addId() = 0;
    virtual Value<std::string> &addFqdn() = 0;
    virtual Value<int> &addExPeriodMin() = 0;
    virtual Value<int> &addExPeriodMax() = 0;
    virtual Value<int> &addValPeriod() = 0;
    virtual Value<int> &addDotsMax() = 0;
    virtual Value<bool> &addEnumZone() = 0;
    virtual ZoneNs& addZoneNs() = 0;

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version)
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
    }
}; // class Zone

class ZoneImpl:
    virtual public Zone {
public:
    ZoneImpl(bool set_active = false);
    virtual ~ZoneImpl();

    virtual Table &joinZoneTable();
    virtual Value<Database::ID> &addId();
    virtual Value<std::string> &addFqdn();
    virtual Value<int> &addExPeriodMin();
    virtual Value<int> &addExPeriodMax();
    virtual Value<int> &addValPeriod();
    virtual Value<int> &addDotsMax();
    virtual Value<bool> &addEnumZone();
    virtual ZoneNs& addZoneNs();

    friend class boost::serialization::access;
    template<class Archive> void serialize(Archive& _ar,
            const unsigned int _version) 
    {
        _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Zone);
    }
}; // class ZoneImpl

} // namespace Filters
} // namespace Database

#endif // _ZONE_FILTER_H_
