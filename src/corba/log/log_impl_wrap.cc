#include "log_impl_wrap.h"
#include "register/request.h"

#include <corba/ccReg.hh>
#include <boost/thread.hpp>


ccReg_Log_i::ccReg_Log_i(const std::string database, const std::string &monitoring_hosts_file) : pagetables()
{
	back.reset(Register::Logger::Manager::create(database, monitoring_hosts_file));
}

  // ccReg_Log_i(const std::string database) throw (Register::Logger::Manager::DB_CONNECT_FAILED): Register::Logger::Manager(database) {};
ccReg_Log_i::~ccReg_Log_i() 
{
    PortableServer::POA_ptr poa = this->_default_POA();
    
    for (pagetables_list::iterator mit = pagetables.begin(); mit != pagetables.end(); mit++) {

        try {

            poa->deactivate_object(*(poa->servant_to_id(mit->second)));
        } catch (...) {

        }
    }
}

ccReg::TID ccReg_Log_i::CreateRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content_in, const ccReg::RequestProperties& props, CORBA::Long request_type_id, ccReg::TID session_id) {    
    std::auto_ptr<Register::Logger::RequestProperties> p(convert_properties(props));
    return back->i_CreateRequest(sourceIP, (Database::Filters::ServiceType)service, content_in, *(p.get()), request_type_id, session_id);
}

CORBA::Boolean ccReg_Log_i::UpdateRequest(ccReg::TID id, const ccReg::RequestProperties &props) {
    std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
    return back->i_UpdateRequest(id, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::CloseRequest(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props, const CORBA::Long result_code) {
    std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
    return back->i_CloseRequest(id, content_out, *(p.get()), result_code);
}

CORBA::Boolean ccReg_Log_i::CloseRequestLogin(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props, ccReg::TID session_id, const CORBA::Long result_code) {
    std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
    return back->i_CloseRequestLogin(id, content_out, *(p.get()), session_id, result_code);
}

ccReg::TID ccReg_Log_i::CreateSession(ccReg::Languages lang, const char *name) {    
    return back->i_CreateSession((Languages) lang, name);
}

CORBA::Boolean ccReg_Log_i::CloseSession(ccReg::TID id) {    
    return back->i_CloseSession(id);
}

ccReg::RequestTypeList *ccReg_Log_i::GetRequestTypesByService(ccReg::RequestServiceType service) {    
    Database::Result res = back->i_GetRequestTypesByService((Database::Filters::ServiceType)service);

    int size = res.size();
    ccReg::RequestTypeList_var ret = new ccReg::RequestTypeList();

    ret->length(size);

    for (int i = 0; i < size; i++) {
        ret[i].id = (ccReg::RequestType)res[i][0];
        ret[i].name = CORBA::string_dup(((std::string)res[i][1]).c_str());
    }

    return ret._retn();
}

ccReg::RequestServiceList* ccReg_Log_i::GetServices()
{
    Database::Result data = back->i_GetServices();
    unsigned int size = data.size();

    ccReg::RequestServiceList_var ret = new ccReg::RequestServiceList();
    ret->length(size);
    for (unsigned int i = 0; i < size; ++i) {
        ret[i].id = static_cast<ccReg::RequestType>(data[i][0]);
        ret[i].name = CORBA::string_dup(static_cast<std::string>(data[i][1]).c_str());
    }

    return ret._retn();
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
        std::auto_ptr<Register::Logger::Manager> logger_manager;

        logger_manager.reset(Register::Logger::Manager::create());
        Register::Logger::List *list = logger_manager->createList();
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

Registry::Request::Detail*  ccReg_Log_i::getDetail(ccReg::TID _id) {
	
	LOGGER(PACKAGE).debug(boost::format("constructing request filter for object id=%1% detail") % _id);

        boost::mutex::scoped_lock slm (pagetables_mutex);
	std::auto_ptr<Register::Logger::List> request_list(back->createList());

	Database::Filters::Union union_filter;
	// where is it deleted? TODO
	Database::Filters::Request *filter = new Database::Filters::RequestImpl();

	filter->addId().setValue(Database::ID(_id));
	union_filter.addFilter(filter);

	request_list->setPartialLoad(false);
	// TODO make sure the db_manager is OK
	//
	// request_list->reload(union_filter, &m_db_manager);
	request_list->reload(union_filter);

	if(request_list->size() != 1) {            
		throw ccReg::Admin::ObjectNotFound();
	}
	return createRequestDetail(request_list->get(0));

}

Registry::Request::Detail *ccReg_Log_i::createRequestDetail(Register::Logger::Request *req) {
	Registry::Request::Detail *detail = new Registry::Request::Detail();

        detail->id		        = req->getId();
        detail->timeBegin       = DUPSTRDATE(req->getTimeBegin);
        detail->timeEnd         = DUPSTRDATE(req->getTimeEnd); 
        detail->sourceIp        = DUPSTRFUN(req->getSourceIp);
        detail->service_type    = DUPSTRFUN(req->getServiceType);
        detail->action_type     = DUPSTRFUN(req->getActionType);
        detail->session_id      = req->getSessionId();
        detail->user_name       = DUPSTRFUN(req->getUserName);
        detail->is_monitoring   = req->getIsMonitoring();
        detail->raw_request     = DUPSTRFUN(req->getRawRequest);
        detail->raw_response    = DUPSTRFUN(req->getRawResponse);

        // TODO refactor - this convert function could be moved here (or sw else)
	detail->props		= convert_properties_d2c(req->getProperties());

	return detail;
}
