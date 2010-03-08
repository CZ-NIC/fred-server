#include "log_impl_wrap.h"
#include "register/request.h"

#include <corba/ccReg.hh>


ccReg_Log_i::ccReg_Log_i(const std::string database, const std::string &monitoring_hosts_file) 
{
	back.reset(Register::Logger::Manager::create(database, monitoring_hosts_file));
}

  // ccReg_Log_i(const std::string database) throw (Register::Logger::Manager::DB_CONNECT_FAILED): Register::Logger::Manager(database) {};
ccReg_Log_i::~ccReg_Log_i() 
{
}

ccReg::TID ccReg_Log_i::CreateRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content_in, const ccReg::RequestProperties& props, CORBA::Long action_type, ccReg::TID session_id)
{
	std::auto_ptr<Register::Logger::RequestProperties> p (convert_properties(props));
	return back->i_CreateRequest(sourceIP, (Database::Filters::RequestServiceType)service, content_in, *(p.get()), action_type, session_id);
}

CORBA::Boolean ccReg_Log_i::UpdateRequest(ccReg::TID id, const ccReg::RequestProperties &props)
{
	std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
	return back->i_UpdateRequest(id, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::CloseRequest(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props)
{
	std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
	return back->i_CloseRequest(id, content_out, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::CloseRequestLogin(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props, ccReg::TID session_id)
{
	std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
	return back->i_CloseRequestLogin(id, content_out, *(p.get()), session_id);
}

ccReg::TID ccReg_Log_i::CreateSession(ccReg::Languages lang, const char *name)
{
	return back->i_CreateSession((Languages)lang, name);
}

CORBA::Boolean ccReg_Log_i::CloseSession(ccReg::TID id)
{
	return back->i_CloseSession(id);
}

ccReg::RequestActionList *ccReg_Log_i::GetServiceActions(ccReg::RequestServiceType service)
{
	Database::Result res = back->i_GetServiceActions((Database::Filters::RequestServiceType)service);

	int size  = res.size();
	ccReg::RequestActionList_var ret = new ccReg::RequestActionList();

	ret->length(size);

	for (int i = 0; i< size; i++) {
		
		ret[i].id 	 = (ccReg::RequestActionType)res[i][0];
		ret[i].status = CORBA::string_dup(((std::string)res[i][1]).c_str());
	}

	return ret._retn();
}

Registry::PageTable_ptr ccReg_Log_i::getPageTable(const char *session_id)
{
    std::auto_ptr<Register::Logger::Manager> logger_manager;

    logger_manager.reset(Register::Logger::Manager::create());
    Register::Logger::List *list = logger_manager->createList();
    ccReg_Logger_i * ret_ptr = new ccReg_Logger_i(list);
    Registry::PageTable_ptr ret = ret_ptr->_this();
    
    pagetables[session_id] = ret;

    return ret;
}
// m_logsession = new ccReg_LogSession_i(m_logsession_manager->createList());

void ccReg_Log_i::deletePageTable(const char* session_id)
{
    pagetables_list::iterator it;

    it = pagetables.find(session_id);

    if (it == pagetables.end()) {
        LOGGER("fred-server").error(boost::format(" Logger pagetable for session %1% should be deleted but it was not found") % session_id);
    } else {
        CORBA::release(it->second);
        // ccReg::Logger_Helper::release
        pagetables.erase(it);
    }
}

Registry::Request::Detail*  ccReg_Log_i::getDetail(ccReg::TID _id) {
	// Register::Logger::Request *request = m_logger->findId(_id);
	// if (request) {
	//		return createRequestDetail(request);
	// } else {



	LOGGER(PACKAGE).debug(boost::format("constructing request filter for object id=%1% detail") % _id);

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