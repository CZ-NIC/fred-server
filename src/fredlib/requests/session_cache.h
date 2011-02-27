/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef SESSION_CACHE_H_
#define SESSION_CACHE_H_

#include "session.h"
#include "model_session.h"
#include "log/logger.h"
#include <map>
#include <stdexcept>
#include <boost/thread/mutex.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>



namespace Fred {

namespace Logger {


using namespace boost::posix_time;
using namespace boost::gregorian;

const ptime current_timestamp();
    // hypotetical
class SessionCacheItem {
private:
    ptime timestamp;
    ModelSession data;

public:

    SessionCacheItem (const ptime &t, const ModelSession &s) :
        timestamp(t), 
        data(s) 
    { };

    void update_timestamp() {
        timestamp = current_timestamp();
    }

    bool is_timestamp_older(const ptime &limit) const {
        return timestamp < limit;
    }

    const ModelSession &get_data() const {
        return data;
    }
};

class CACHE_MISS : public std::runtime_error {
public:
    CACHE_MISS() : std::runtime_error("SessionCache: Item not found") 
    { };
};

/** 
 * class to cache session which are just being processed in logging 
 * backed. It needs some garbage collection mechanism since some 
 * session are not closed
 */
class SessionCache {
private:
    typedef std::map<Database::ID, SessionCacheItem> CacheType;
    CacheType cache;
    boost::mutex cache_mutex;

    const size_t max_items;
    const time_duration item_ttl;

public:

    SessionCache(size_t max_capacity, unsigned ttl_seconds) : 
        max_items(max_capacity),
        item_ttl(seconds(ttl_seconds))
    { }

    void add(const Database::ID &id, const ModelSession &s);
    void remove(const Database::ID &id);
    const ModelSession get(const Database::ID &id);

private:
    bool needGarbage();
    void performGarbage();

};

}
}

#endif /*SESSION_CACHE_H_ */
