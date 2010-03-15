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
    for (pagetables_list::iterator mit = pagetables.begin(); mit != pagetables.end(); mit++) {

        std::list<ccReg_Logger_i*> *ptlist = mit->second;
        for (std::list<ccReg_Logger_i*>::iterator lit = ptlist->begin(); lit != ptlist->end(); lit++) {
            try {
                PortableServer::POA_ptr poa = this->_default_POA();
                poa->deactivate_object(*(poa->servant_to_id(*lit)));
            } catch (...) {

            }
        }

        delete ptlist;
    }
}

ccReg::TID ccReg_Log_i::CreateRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content_in, const ccReg::RequestProperties& props, CORBA::Long action_type, ccReg::TID session_id) {
    boost::mutex::scoped_lock slm (lmutex);

    std::auto_ptr<Register::Logger::RequestProperties> p(convert_properties(props));
    return back->i_CreateRequest(sourceIP, (Database::Filters::RequestServiceType)service, content_in, *(p.get()), action_type, session_id);
}

CORBA::Boolean ccReg_Log_i::UpdateRequest(ccReg::TID id, const ccReg::RequestProperties &props) {
    boost::mutex::scoped_lock slm (lmutex);

    std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
    return back->i_UpdateRequest(id, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::CloseRequest(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props) {
    boost::mutex::scoped_lock slm (lmutex);

    std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
    return back->i_CloseRequest(id, content_out, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::CloseRequestLogin(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props, ccReg::TID session_id) {
    boost::mutex::scoped_lock slm (lmutex);

    std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
    return back->i_CloseRequestLogin(id, content_out, *(p.get()), session_id);
}

ccReg::TID ccReg_Log_i::CreateSession(ccReg::Languages lang, const char *name) {
    boost::mutex::scoped_lock slm (lmutex);

    return back->i_CreateSession((Languages) lang, name);
}

CORBA::Boolean ccReg_Log_i::CloseSession(ccReg::TID id) {
    boost::mutex::scoped_lock slm (lmutex);

    return back->i_CloseSession(id);
}

ccReg::RequestActionList *ccReg_Log_i::GetServiceActions(ccReg::RequestServiceType service) {
    boost::mutex::scoped_lock slm (lmutex);
    
    Database::Result res = back->i_GetServiceActions((Database::Filters::RequestServiceType)service);

    int size = res.size();
    ccReg::RequestActionList_var ret = new ccReg::RequestActionList();

    ret->length(size);

    for (int i = 0; i < size; i++) {
        ret[i].id = (ccReg::RequestActionType)res[i][0];
        ret[i].status = CORBA::string_dup(((std::string)res[i][1]).c_str());
    }

    return ret._retn();
}

Registry::PageTable_ptr ccReg_Log_i::createPageTable(const char *session_id)
{    
    Registry::PageTable_ptr ret;
    pagetables_list::iterator it;

    std::auto_ptr<Register::Logger::Manager> logger_manager;

    logger_manager.reset(Register::Logger::Manager::create());
    Register::Logger::List *list = logger_manager->createList();
    ccReg_Logger_i * ret_ptr = new ccReg_Logger_i(list);
    ret = ret_ptr->_this();
 
    boost::mutex::scoped_lock slm (lmutex);

    it = pagetables.find(session_id);

    if (it != pagetables.end()) {
        
        it->second->push_back(ret_ptr);
                        
        LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: added another pagetable for client (session_id %1%) ") % session_id);
    } else {
        std::list<ccReg_Logger_i*> *ptlist = new std::list<ccReg_Logger_i*>();        
        
        ptlist->push_back(ret_ptr);
        pagetables[session_id] = ptlist;
    }

    LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: Returning a pagetable object (%1%) to a client (session %2%).") % ret % session_id);

    return ret;
}
// m_logsession = new ccReg_LogSession_i(m_logsession_manager->createList());

void ccReg_Log_i::deletePageTable(const char* session_id)
{
    pagetables_list::iterator it;

    boost::mutex::scoped_lock slm (lmutex);

    it = pagetables.find(session_id);

    if (it == pagetables.end()) {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: No pagetable found for session %1%, no action. ") % session_id);
    } else {
        LOGGER(PACKAGE).debug(boost::format("ccReg_Log_i: A pagetable found for session %1%, deleting. ") % session_id);

        for (std::list<ccReg_Logger_i*>::iterator l = it->second->begin();
                l != it->second->end();
                l++) {

            try {                
                PortableServer::POA_ptr poa = this->_default_POA();
                poa->deactivate_object(*(poa->servant_to_id(*l)));

                // this should decrease reference count to 0 thus releasing the servant
                (*l)->_remove_ref();
            } catch (...) {
                delete it->second;
                throw;
            }

        }
              
        delete it->second;
        
        pagetables.erase(it);
    }
}

Registry::Request::Detail*  ccReg_Log_i::getDetail(ccReg::TID _id) {
	
	LOGGER(PACKAGE).debug(boost::format("constructing request filter for object id=%1% detail") % _id);

        boost::mutex::scoped_lock slm (lmutex);
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

	// TODO should it contain ID?
	// detail->id		= req->id;

	detail->timeBegin 	= DUPSTRDATE(req->getTimeBegin);
	detail->timeEnd		= DUPSTRDATE(req->getTimeEnd);
	detail->sourceIp	= DUPSTRFUN(req->getSourceIp);
	detail->service_type	= DUPSTRFUN(req->getServiceType);
	detail->action_type	= DUPSTRFUN(req->getActionType);
	detail->session_id	= req->getSessionId();
	detail->is_monitoring	= req->getIsMonitoring();
	detail->raw_request	= DUPSTRFUN(req->getRawRequest);
	detail->raw_response	= DUPSTRFUN(req->getRawResponse);

        // TODO refactor - this convert function could be moved here (or sw else)
	detail->props		= convert_properties_d2c(req->getProperties());

	return detail;
}
