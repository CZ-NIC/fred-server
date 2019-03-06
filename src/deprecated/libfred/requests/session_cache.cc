/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/deprecated/libfred/requests/session_cache.hh"

namespace LibFred {
namespace Logger {


const ptime current_timestamp() {
    return second_clock::universal_time();
} 

void SessionCache::add(const Database::ID &id, const std::shared_ptr<ModelSession> s)
{
    ptime current = current_timestamp();

    if (needGarbage()) {
        performGarbage();
    }

    if (needGarbage()) {
        // TODO avoid this
#ifdef HAVE_LOGGER
        LOGGER.warning ("Garbage collection wasn't sufficient...");
#endif
        return; 
    }

    boost::mutex::scoped_lock lock(cache_mutex);
    cache.insert(std::make_pair(id, SessionCacheItem(current, s)));

    // TODO DEBUG
#ifdef HAVE_LOGGER
    LOGGER.info(boost::format("SCACHE Added a record, size: %1%") % cache.size());
#endif

}

/**
 * removing a nonexistent item is not an error
 * since it could've been removed by garbage procedure
 */
void SessionCache::remove(const Database::ID id)
{
    boost::mutex::scoped_lock lock(cache_mutex);
    CacheType::const_iterator it = cache.find(id);
    if(it != cache.end()) {
        cache.erase(id);
        // TODO DEBUG
#ifdef HAVE_LOGGER
        LOGGER.info(boost::format("SCACHE Removed a record, size: %1%") % cache.size());
#endif
    }
}

/** 
 * We cannot return a reference here since it can become invalid
 */
std::shared_ptr<ModelSession> SessionCache::get(const Database::ID id)
{
    boost::mutex::scoped_lock lock(cache_mutex);
    CacheType::iterator i = cache.find(id);
    if(i != cache.end()) {
        SessionCacheItem &item = i->second;
        item.update_timestamp();
        return item.get_data();
    }
#ifdef HAVE_LOGGER
        LOGGER.info("SCACHE: Cache miss");
#endif
    throw CACHE_MISS();
}

bool SessionCache::needGarbage() 
{
    boost::mutex::scoped_lock lock(cache_mutex);
    return cache.size() > max_items;
}

void SessionCache::performGarbage() 
{
    unsigned deleted_count = 0;
    ptime min = current_timestamp();
    ptime max_timestamp = min - item_ttl;

    //TODO DEBUG
#ifdef HAVE_LOGGER 
    LOGGER.info(boost::format(
        "SCACHE: Performing garbage collection with timestamp %1% ") % max_timestamp);
#endif

    boost::mutex::scoped_lock lock(cache_mutex);

    // records in interval <oldest_tstamp, max_timestamp) could be removed
    if(oldest_tstamp >= max_timestamp) {
        return;
    }

    CacheType::const_iterator i = cache.begin();
    while (i != cache.end()) {
        if ((i->second).is_timestamp_older(min)) {
            min = (i->second).get_timestamp();
        }
        if ((i->second).is_timestamp_older(max_timestamp)) {
            Database::ID to_delete = i->first;

            i++;
            cache.erase(to_delete);
            deleted_count++;
        } else {

            i++;
        }
    }

    // this must be also done under a lock
    oldest_tstamp = min;

#ifdef HAVE_LOGGER
    LOGGER.info(boost::format("SCACHE: Garbage finished, deleted %1% records")
            % deleted_count);
#endif 
       
}

}
}
