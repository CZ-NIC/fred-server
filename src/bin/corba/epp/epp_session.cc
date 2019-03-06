/*
 * Copyright (C) 2012-2019  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/epp/epp_session.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/db_settings.hh"

// logger
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/deprecated/util/log.hh"

#include <boost/thread/thread.hpp>

/// throws NumberSessionLimit when session number limit is exceeded
/// returns loginID, begins with garbage procedure
unsigned long long EppSessionContainer::login_session(int regId, int lang)
{
    boost::mutex::scoped_lock lock(session_mutex);

    garbage_session_internal();

    if(sessions.size() >= max_total_sessions) {
        throw NumberSessionLimit("maximal total number of sessions reached");
    }

    // find registrar session count
    std::map<int, int>::iterator found = registrar_session_count.find(regId);

    unsigned reg_sessions_count = 0;
    if (found != registrar_session_count.end()) {
        reg_sessions_count = found->second;
    }

    if(reg_sessions_count >= max_reg_sessions) {
        throw NumberSessionLimit("max registrar session count reached");
    }

    // generate new ID and check for collision. this way we are sure the sequence
    // does not start with 0 which might be a special value

    Database::Connection conn = Database::Manager::acquire();
    Database::Result res = conn.exec("SELECT nextval('epp_login_id_seq'::regclass)");

    unsigned long long loginId = res[0][0];

    std::map<unsigned long long, Session>::iterator s_found = sessions.find(loginId);
    if(s_found != sessions.end()) {
        std::string msg("login ID collision");

        LOGGER.error(msg);
        throw std::runtime_error(msg);
    }

    /// fill sessions
    Session new_session(regId, lang, (long long) time(NULL));

    LOGGER.debug( boost::format("Session with clientID %1% for registrarID %2%, lang %3% created, timestamp %4%") % loginId % regId % lang % new_session.timestamp );

    sessions[loginId] = new_session;

    /// fill registrar sessoin count
    try {
        registrar_session_count[regId] = reg_sessions_count + 1;
    } catch(...) {
        // rollback the first operation in case of a failure
        sessions.erase(loginId);
        throw;
    }

    return loginId;
}

void EppSessionContainer::logout_session(unsigned long long loginId)
{
    boost::mutex::scoped_lock lock(session_mutex);

    logout_session_worker(loginId);
}

void EppSessionContainer::destroy_all_registrar_sessions(long regId)
{

    LOGGER.notice( boost::format("Destroying all session for registrar ID %1% ") % regId );

    boost::mutex::scoped_lock lock(session_mutex);

    for (std::map<unsigned long long, Session>::iterator it = sessions.begin();
            it != sessions.end();
            ) {

        std::map<unsigned long long, Session>::iterator it_next = it;
        it_next++;
        if(it->second.registrarID == regId) {
            LOGGER.notice( boost::format("Disconnecting session clientID %1%, registrar %2% ")
                                              % it->first % regId );

            logout_session_worker(it->first);
        }
        it = it_next;
    }

    std::map<int, int>::iterator found = registrar_session_count.find(regId);

    if (found != registrar_session_count.end()) {
        if(found->second != 0) {
            LOGGER.error( boost::format("ERROR: all registrar's sessions destroyed but session count still reads %1%") % found->second );
        }
    }
}

int EppSessionContainer::get_registrar_id(unsigned long long loginId)
{
    boost::mutex::scoped_lock lock(session_mutex);

    const std::map< unsigned long long, Session >::const_iterator session_ptr = sessions.find(loginId);

    const bool found = (session_ptr != sessions.end());
    if (!found) {
        LOGGER.debug(boost::format("get_registrar_id: Session with loginId %1% doesn't exist") % loginId );
        // throw std::runtime_error("Invalid loginId");
        return 0;
    }

    const unsigned long long registrar_id = session_ptr->second.registrarID;

    LOGGER.debug(boost::format("get_registrar_id: loginID %1% has regID %2%") % loginId % registrar_id);

    update_timestamp(loginId);

    return registrar_id;
}

int EppSessionContainer::get_registrar_lang(unsigned long long loginId)
{
    boost::mutex::scoped_lock lock(session_mutex);

    std::map<unsigned long long, Session>::iterator found = sessions.find(loginId);

    if(found == sessions.end()) {
        LOGGER.debug( boost::format("get_registrar_lang: Invalid loginId %1% ") % loginId );
        /// throw std::runtime_error("Invalid loginId");
        return 0;
    }

    LOGGER.debug( boost::format("get_registrar_lang: loginID %1% has lang %2%") % loginId % found->second.language );

    return found->second.language;
}

/// session_mutext must already be acquired
void EppSessionContainer::update_timestamp(unsigned long long loginId)
{

    std::map<unsigned long long, Session>::iterator found = sessions.find(loginId);

    if(found == sessions.end()) {
        LOGGER.error( boost::format ("update_timestamp: Session with loginId %1% not found") % loginId);
        throw std::runtime_error("loginId not found.");
    }

    found->second.timestamp = time(NULL);

    LOGGER.debug( boost::format("Timestamp for loginId %1% updated to %2%") % loginId % found->second.timestamp);
}

// session_mutex must already be acquired
void EppSessionContainer::logout_session_worker(unsigned long long loginId)
{

    std::map<unsigned long long, Session>::iterator found = sessions.find(loginId);

    if(found == sessions.end()) {
        LOGGER.warning( boost::format("logout_session_worker: loginID %1% not found") % loginId );
        // not considered a serious error, not throwing
        return;
    }

    unsigned regId = found->second.registrarID;

    /// subtract session count:
    std::map<int, int>::iterator reg_found = registrar_session_count.find(regId);

    unsigned reg_sessions_count = 0;
    if (reg_found != registrar_session_count.end()) {
        reg_sessions_count = reg_found->second;
    }

    if(reg_sessions_count == 0) {
        std::string msg("Internal error - number of registrar sessions inconsistent");
        LOGGER.error(msg);
        throw std::runtime_error(msg);
    }

    registrar_session_count[regId] = reg_sessions_count - 1;
    //this operation is nothrow thus we already have the strong guarantee
    sessions.erase(found);

    LOGGER.debug( boost::format("logout_session_worker: loginId %1% logged out, actual number of all sessions: %2%") % loginId % sessions.size() );
}

// session_mutex must already be acquired
void EppSessionContainer::garbage_session_internal()
{
    long long current_timestamp = time(NULL);

    for(std::map<unsigned long long, Session>::iterator it = sessions.begin();
            it != sessions.end();
            ) {

        std::map<unsigned long long, Session>::iterator it_next = it;
        it_next++;
        if (it->second.timestamp + session_timeout_sec < current_timestamp) {
            LOGGER.debug( boost::format("garbage session: loginId %1% with timestamp %2%, clock %3%, timeout %4%")
                                % it->first % it->second.timestamp % current_timestamp % session_timeout_sec );

            logout_session_worker(it->first);
            // it invalidated
        }
        it = it_next;
    }
}
