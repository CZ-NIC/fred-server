/*
 * Copyright (C) 2009-2020  CZ.NIC, z. s. p. o.
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
#include "src/bin/corba/log/log_impl_wrap.hh"

#include "corba/Logger.hh"
#include <boost/thread.hpp>
#include <utility>


ccReg_Log_i::ccReg_Log_i(const std::string database, const std::string &monitoring_hosts_file) : pagetables()
{
	back.reset(LibFred::Logger::Manager::create(database, monitoring_hosts_file));
}

  // ccReg_Log_i(const std::string database) throw (LibFred::Logger::Manager::DB_CONNECT_FAILED): LibFred::Logger::Manager(database) {};
ccReg_Log_i::~ccReg_Log_i()
{
    try {
        PortableServer::POA_ptr poa = this->_default_POA();

        for (pagetables_list::iterator mit = pagetables.begin(); mit != pagetables.end(); mit++) {
            poa->deactivate_object(*(poa->servant_to_id(mit->second)));
        }
    } catch (std::exception &ex) {
        logger_error(boost::format("Exception in ~ccReg_Log_i(): %1%") % ex.what() );
    } catch (...) {
        logger_error("Unknown exception in ~ccReg_Log_i()");
    }
}


// use ONLY in exception handlers
void Logger_common_exception_handler(const std::string &method_name)
{
    try {
        throw;
    } catch (InternalServerError &ex) {
        logger_error(boost::format("Internal server error in %1%: %2%") % method_name % ex.what());
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    } catch (Database::Exception &ex) {
        logger_error(boost::format("Database error in %1%: %2%") % method_name % ex.what());
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    } catch (std::exception &ex) {
        logger_error(boost::format("Exception in %1%: %2%" ) % method_name % ex.what());
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    } catch (...) {
        logger_error(boost::format("Unknown exception in %1%") % method_name);
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::TID ccReg_Log_i::createRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content, const ccReg::RequestProperties& props, const ccReg::ObjectReferences &refs, CORBA::Long request_type_id, ccReg::TID session_id)
{
    try {
        std::unique_ptr<LibFred::Logger::RequestProperties> p(convert_properties(props));
        std::unique_ptr<LibFred::Logger::ObjectReferences> r(convert_obj_references(refs));
        return back->i_createRequest(sourceIP, (Database::Filters::ServiceType)service, content, *(p.get()), *(r.get()), request_type_id, session_id);

    } catch(...) {
        Logger_common_exception_handler("createRequest");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }

}

void ccReg_Log_i::closeRequest(ccReg::TID id, const char *content, const ccReg::RequestProperties &props, const ccReg::ObjectReferences &refs, const CORBA::Long result_code, ccReg::TID session_id)
{
    try {
        std::unique_ptr<LibFred::Logger::RequestProperties> p = convert_properties(props);
        std::unique_ptr<LibFred::Logger::ObjectReferences> r(convert_obj_references(refs));

        if( back->i_closeRequest(id, content, *(p.get()), *(r.get()), result_code, session_id) == false) {
            throw ccReg::Logger::REQUEST_NOT_EXISTS();
        }
    }
    catch (...) {
        Logger_common_exception_handler("closeRequest");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }

}

ccReg::TID ccReg_Log_i::createSession(ccReg::TID user_id, const char *name)
{
    ConnectionReleaser releaser;

    try {
        ccReg::TID ret = back->i_createSession(user_id, name);

        if(ret == 0) {
            throw ccReg::Logger::SESSION_NOT_EXISTS();
        }
        else {
            return ret;
        }
    }
    catch (...) {
        Logger_common_exception_handler("createSession");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

void ccReg_Log_i::closeSession(ccReg::TID id)
{
    try {
        if( back->i_closeSession(id) == false) {
            throw ccReg::Logger::SESSION_NOT_EXISTS();
        }
    }
    catch (...) {
        Logger_common_exception_handler("closeSession");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::RequestTypeList *ccReg_Log_i::getRequestTypesByService(ccReg::RequestServiceType service)
{
    ConnectionReleaser releaser;

    try {
        Database::Result res = back->i_getRequestTypesByService((Database::Filters::ServiceType)service);

        int size = res.size();
        ccReg::RequestTypeList_var ret = new ccReg::RequestTypeList();

        ret->length(size);

        for (int i = 0; i < size; i++) {
            ret[i].id = (ccReg::RequestType)res[i][0];
            ret[i].name = CORBA::string_dup(((std::string)res[i][1]).c_str());
        }

        return ret._retn();
    }
    catch (...) {
        Logger_common_exception_handler("getRequestTypesByService");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::RequestServiceList* ccReg_Log_i::getServices()
{
    ConnectionReleaser releaser;

    try {
        Database::Result data = back->i_getServices();
        unsigned int size = data.size();

        ccReg::RequestServiceList_var ret = new ccReg::RequestServiceList();
        ret->length(size);
        for (unsigned int i = 0; i < size; ++i) {
            ret[i].id = static_cast<ccReg::RequestType>(data[i][0]);
            ret[i].name = CORBA::string_dup(static_cast<std::string>(data[i][1]).c_str());
        }

        return ret._retn();
    }
    catch (...) {
        Logger_common_exception_handler("getServices");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::ResultCodeList* ccReg_Log_i::getResultCodesByService(ccReg::RequestServiceType service)
{
    ConnectionReleaser releaser;

    try {
        Database::Result res = back->i_getResultCodesByService
                (static_cast<Database::Filters::ServiceType>(service));
        int size = res.size();
        ccReg::ResultCodeList_var ret = new ccReg::ResultCodeList();
        ret->length(size);
        for (int i = 0; i < size; i++)
        {
            ret[i].result_code = static_cast<ccReg::ResultCode>(res[i][0]);
            ret[i].name = CORBA::string_dup(std::string(res[i][1]).c_str());
        }
        return ret._retn();
    }
    catch (...) {
        Logger_common_exception_handler("getResultCodesByService");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::Logger::ObjectTypeList* ccReg_Log_i::getObjectTypes()
{
    ConnectionReleaser releaser;

    try {
        Database::Result res = back->i_getObjectTypes();

        ccReg::Logger::ObjectTypeList_var ret = new ccReg::Logger::ObjectTypeList();
        ret->length(res.size());

        for (unsigned i=0; i<res.size(); i++) {
            /*
             * TODO
            ret[i].id =   static_cast<ccReg::TID>(res[i][0]);
            ret[i].type = CORBA::string_dup(res[i][1]);
            */
            ret[i]  = CORBA::string_dup((std::string(res[i][0])).c_str());
        }

        // TODO impl
        return ret._retn();
    }
    catch (...) {
        Logger_common_exception_handler("getObjectTypes");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

Registry::PageTable_ptr ccReg_Log_i::createPageTable(const char *session_id)
{
    Registry::PageTable_ptr ret;
    pagetables_list::iterator it;

    try {
        // this method doesn't call logger implementation, we have to init context here
        // and also declare connection releaser
        logd_ctx_init ctx;
        TRACE("[CALL] ccReg_Log_i::createPageTable");
        ConnectionReleaser releaser;

        boost::mutex::scoped_lock slm (pagetables_mutex);

        it = pagetables.find(session_id);

        if (it != pagetables.end()) {
            LOGGER.debug(boost::format("ccReg_Log_i: returning existing pagetable for client (session_id %1%) ") % session_id);
            return it->second->_this();

        } else {
            std::unique_ptr<LibFred::Logger::Manager> logger_manager;

            LibFred::Logger::List *list = back->createList();
            ccReg_Logger_i * ret_ptr = new ccReg_Logger_i(list);
            ret = ret_ptr->_this();

            pagetables[session_id] = ret_ptr;
        }

        LOGGER.debug(boost::format("ccReg_Log_i: Returning a pagetable object (%1%) to a client (session %2%).") % ret % session_id);

        return ret;
    } catch(...) {
        Logger_common_exception_handler("createPageTable");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}
// m_logsession = new ccReg_LogSession_i(m_logsession_manager->createList());

void ccReg_Log_i::deletePageTable(const char* session_id)
{
    pagetables_list::iterator it;

    try {
        // this method doesn't call logger implementation, we have to init context here
        logd_ctx_init ctx;
        TRACE("[CALL] ccReg_Log_i::deletePageTable");

        boost::mutex::scoped_lock slm (pagetables_mutex);

        it = pagetables.find(session_id);

        PortableServer::POA_ptr poa = this->_default_POA();

        if (it == pagetables.end()) {
            LOGGER.debug(boost::format("ccReg_Log_i: No pagetable found for session %1%, no action. ") % session_id);
        } else {
            LOGGER.debug(boost::format("ccReg_Log_i: A pagetable found for session %1%, deleting. ") % session_id);

            poa->deactivate_object(*(poa->servant_to_id(it->second)));

            it->second->_remove_ref();

            pagetables.erase(it);
        }
    } catch(...) {
        Logger_common_exception_handler("deletePageTable");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}


// huge TODO  - exceptions
ccReg::Logger::Detail*  ccReg_Log_i::getDetail(ccReg::TID _id)
{
    try {
        // this method doesn't call logger implementation, we have to init context here
        // and also declare connection releaser
        logd_ctx_init ctx;
        ConnectionReleaser releaser;

        LOGGER.debug(boost::format("constructing request filter for object id=%1% detail") % _id);

        boost::mutex::scoped_lock slm (pagetables_mutex);
        std::unique_ptr<LibFred::Logger::List> request_list(back->createList());

        Database::Filters::Union union_filter;
        // where is it deleted? TODO
        Database::Filters::Request *filter = new Database::Filters::RequestImpl();

        filter->addId().setValue(Database::ID(_id));
        union_filter.addFilter(filter);

        request_list->setPartialLoad(false);
        // TODO make sure the db_manager is OK
        //
        // request_list->reload(union_filter, &m_db_manager);
        try {
            request_list->reload(union_filter);
        } catch(Database::Exception &ex) {
            std::string message = ex.what();
            if(message.find("statement timeout") != std::string::npos) {
                LOGGER.info("Statement timeout in request list.");
                throw Registry::SqlQueryTimeout();
            }
            throw;
        }

        if(request_list->size() != 1) {
            LOGGER.info("throwing OBJECT_NOT_FOUND(): number of items found is not 1");
            throw ccReg::Logger::OBJECT_NOT_FOUND();
        }
        return createRequestDetail(request_list->get(0));
    } catch (const Registry::SqlQueryTimeout&) {
        throw;
    } catch (const ccReg::Logger::OBJECT_NOT_FOUND&) {
        throw;
    } catch (...) {
        Logger_common_exception_handler("getDetail");
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }

}

ccReg::Logger::Detail *ccReg_Log_i::createRequestDetail(LibFred::Logger::Request *req)
{
	ccReg::Logger::Detail *detail = new ccReg::Logger::Detail();

        detail->id              = req->getId();
        detail->timeBegin       = DUPSTRDATE(req->getTimeBegin);
        detail->timeEnd         = DUPSTRDATE(req->getTimeEnd);
        detail->sourceIp        = DUPSTRFUN(req->getSourceIp);
        detail->service_type    = DUPSTRFUN(req->getServiceType);
        detail->action_type     = DUPSTRFUN(req->getActionType);
        detail->session_id      = req->getSessionId();
        detail->user_name       = DUPSTRFUN(req->getUserName);
        detail->user_id         = req->getUserId();
        detail->is_monitoring   = req->getIsMonitoring();
        detail->raw_request     = DUPSTRFUN(req->getRawRequest);
        detail->raw_response    = DUPSTRFUN(req->getRawResponse);

        std::pair<int , std::string> rc = req->getResultCode();
        detail->result_code     = rc.first;
        detail->result_name     = CORBA::string_dup(rc.second.c_str());

        // TODO refactor - this convert function could be moved here (or sw else)
	detail->props		= convert_properties_detail_d2c(req->getProperties());
        detail->refs            = convert_obj_references_d2c(req->getReferences());

	return detail;
}
