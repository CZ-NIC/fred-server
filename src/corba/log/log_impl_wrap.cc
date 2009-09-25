#include "log_impl_wrap.h"

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
	ccReg::RequestActionList *ret = new ccReg::RequestActionList(size);

	for (int i = 0; i< size; i++) {
		(*ret)[i].id 	 = (ccReg::RequestActionType)res[i][1];
		(*ret)[i].status = ((std::string)res[i][2]).c_str();
	}

	return ret;
}




