#include "log_impl_wrap.h"

#include <corba/Logger.hh>
#include <boost/thread.hpp>


ccReg_Log_i::ccReg_Log_i(const std::string database, const std::string &monitoring_hosts_file) : pagetables()
{
	back.reset(Fred::Logger::Manager::create(database, monitoring_hosts_file));
}

  // ccReg_Log_i(const std::string database) throw (Fred::Logger::Manager::DB_CONNECT_FAILED): Fred::Logger::Manager(database) {};
ccReg_Log_i::~ccReg_Log_i()
{
    PortableServer::POA_ptr poa = this->_default_POA();

    for (pagetables_list::iterator mit = pagetables.begin(); mit != pagetables.end(); mit++) {
        try {
            poa->deactivate_object(*(poa->servant_to_id(mit->second)));
        }
        catch (...) {
        }
    }
}

ccReg::TID ccReg_Log_i::createRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content, const ccReg::RequestProperties& props, const ccReg::ObjectReferences &refs, CORBA::Long request_type_id, ccReg::TID session_id)
{
    try {
        std::auto_ptr<Fred::Logger::RequestProperties> p(convert_properties(props));
        std::auto_ptr<Fred::Logger::ObjectReferences> r(convert_obj_references(refs));
        return back->i_createRequest(sourceIP, (Database::Filters::ServiceType)service, content, *(p.get()), *(r.get()), request_type_id, session_id);
    }
    catch (...) {
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

void ccReg_Log_i::addRequestProperties(ccReg::TID id, const ccReg::RequestProperties &props)
{
    try {
        std::auto_ptr<Fred::Logger::RequestProperties> p = convert_properties(props);
        if( back->i_addRequestProperties(id, *(p.get())) == false) {
            throw ccReg::Logger::REQUEST_NOT_EXISTS();
        }
    }
    catch (...) {
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

void ccReg_Log_i::closeRequest(ccReg::TID id, const char *content, const ccReg::RequestProperties &props, const ccReg::ObjectReferences &refs, const CORBA::Long result_code, ccReg::TID session_id)
{
    try {
        std::auto_ptr<Fred::Logger::RequestProperties> p = convert_properties(props);
        std::auto_ptr<Fred::Logger::ObjectReferences> r(convert_obj_references(refs));

        if( back->i_closeRequest(id, content, *(p.get()), *(r.get()), result_code, session_id) == false) {
            throw ccReg::Logger::REQUEST_NOT_EXISTS();
        }
    }
    catch (...) {
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }

}

ccReg::TID ccReg_Log_i::createSession(ccReg::TID user_id, const char *name)
{
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
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::RequestTypeList *ccReg_Log_i::getRequestTypesByService(ccReg::RequestServiceType service)
{
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
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::RequestServiceList* ccReg_Log_i::getServices()
{
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
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::ResultCodeList* ccReg_Log_i::getResultCodesByService(ccReg::RequestServiceType service)
{
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
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

ccReg::Logger::ObjectTypeList* ccReg_Log_i::getObjectTypes()
{
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
        throw ccReg::Logger::INTERNAL_SERVER_ERROR();
    }
}

Registry::PageTable_ptr ccReg_Log_i::createPageTable(const char *session_id)
{
    Registry::PageTable_ptr ret;
    pagetables_list::iterator it;

    boost::mutex::scoped_lock slm (pagetables_mutex);

    it = pagetables.find(session_id);

    if (it != pagetables.end()) {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: returning existing pagetable for client (session_id %1%) ") % session_id);
        return it->second->_this();

    } else {
        std::auto_ptr<Fred::Logger::Manager> logger_manager;

        logger_manager.reset(Fred::Logger::Manager::create());
        Fred::Logger::List *list = logger_manager->createList();
        ccReg_Logger_i * ret_ptr = new ccReg_Logger_i(list);
        ret = ret_ptr->_this();

        pagetables[session_id] = ret_ptr;
    }

    LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: Returning a pagetable object (%1%) to a client (session %2%).") % ret % session_id);

    return ret;
}
// m_logsession = new ccReg_LogSession_i(m_logsession_manager->createList());

void ccReg_Log_i::deletePageTable(const char* session_id)
{
    pagetables_list::iterator it;

    boost::mutex::scoped_lock slm (pagetables_mutex);

    it = pagetables.find(session_id);

    PortableServer::POA_ptr poa = this->_default_POA();

    if (it == pagetables.end()) {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: No pagetable found for session %1%, no action. ") % session_id);
    } else {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: A pagetable found for session %1%, deleting. ") % session_id);

        poa->deactivate_object(*(poa->servant_to_id(it->second)));

        it->second->_remove_ref();

        pagetables.erase(it);
    }
}

ccReg::Logger::Detail*  ccReg_Log_i::getDetail(ccReg::TID _id)
{
	LOGGER(PACKAGE).debug(boost::format("constructing request filter for object id=%1% detail") % _id);

        boost::mutex::scoped_lock slm (pagetables_mutex);
	std::auto_ptr<Fred::Logger::List> request_list(back->createList());

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
                    LOGGER(PACKAGE).info("Statement timeout in request list.");
                    throw Registry::SqlQueryTimeout();
                }
                LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
                throw ccReg::Logger::INTERNAL_SERVER_ERROR();
        }

	if(request_list->size() != 1) {
	    LOGGER(PACKAGE).info("throwing OBJECT_NOT_FOUND(): number of items found is not 1");
        throw ccReg::Logger::OBJECT_NOT_FOUND();
	}
	return createRequestDetail(request_list->get(0));

}

ccReg::Logger::Detail *ccReg_Log_i::createRequestDetail(Fred::Logger::Request *req)
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
	detail->props		= convert_properties_d2c(req->getProperties());
        detail->refs            = convert_obj_references_d2c(req->getReferences());

	return detail;
}
