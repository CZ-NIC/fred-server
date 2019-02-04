/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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


#include "src/deprecated/libfred/requests/request_property_name_cache.hh"



using namespace Database;

namespace LibFred {
namespace Logger {


const unsigned int RequestPropertyNameCache::PROP_NAMES_SIZE_LIMIT = 10000;

// this throws exceptions, caller must handle it (we don't want to have logging here)
RequestPropertyNameCache::RequestPropertyNameCache(Database::Connection &conn) {
    try {
        Result res = conn.exec("select id, name from request_property_name");

        if (res.size() > PROP_NAMES_SIZE_LIMIT) {
            std::string msg(" Number of entries in request_property_name is over the limit.");
            logger_error(msg.c_str());

            throw std::runtime_error(msg);
        }

        for (Result::Iterator it = res.begin(); it != res.end(); ++it) {
            Row row = *it;
            property_names[row[1]] = row[0];
        }

    } catch (Database::Exception &ex) {
        logger_error(ex.what());
    } catch (...) {
        logger_error(
                "Unexpected exception caught in RequestPropertyNameCache c-tor");
    }

}

// find ID for the given name of a property
// if the name is tool long, it's truncated to the maximal length allowed by the database
ID RequestPropertyNameCache::find_property_name_id(const std::string &name,
        Connection &conn) {
    TRACE("[CALL] LibFred::Logger::ManagerImpl::find_property_name_id");
    ID property_name_id;
    std::map<std::string, ID>::iterator iter;

    boost::mutex::scoped_lock prop_check(properties_mutex);
    iter = property_names.find(name);

    if (iter != property_names.end()) {
        property_name_id = iter->second;

        prop_check.unlock();
        // unlock
    } else {
        // if the name isn't cached in the memory, try to find it in the database

        prop_check.unlock();

        std::string s_name = conn.escape(name);

        boost::format query = boost::format(
                "select id from request_property_name where name='%1%'")
                % s_name;
        Result res = conn.exec(query.str());

        if (res.size() == 0) {
            boost::format lockq = boost::format(
                    "LOCK TABLE %1% IN SHARE ROW EXCLUSIVE MODE")
                    % ModelRequestPropertyName::getTableName();
            conn.exec(lockq.str());

            res = conn.exec(query.str());
        }

        if (res.size() > 0) {
            // okay, it was found in the database
            property_name_id = res[0][0];

            // now that we know the right database id of the name
            // we can add it to the map
            prop_check.lock();
            property_names[name] = property_name_id;
            prop_check.unlock();
        } else if (res.size() == 0) {
            // not found, we're under lock, so we can add it now
            // and let the lock release after commiting the transaction
            ModelRequestPropertyName pn;
            pn.setName(name);

            pn.insert();
            property_name_id = pn.getId();

            // we don't add the value to the map - that has to happen after the commit of this transaction
        }
        // if the name was inserted into database, we have to keep it locked until commit
    }

    return property_name_id;
}

}
}
