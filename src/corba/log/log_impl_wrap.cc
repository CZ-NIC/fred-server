#include "log_impl_wrap.h"

#include <corba/ccReg.hh>


ccReg::TID ccReg_Log_i::CreateRequest(const char *sourceIP, ccReg::RequestServiceType service, const char *content_in, const ccReg::RequestProperties& props, CORBA::Long action_type, ccReg::TID session_id)
{
	std::auto_ptr<Register::Logger::RequestProperties> p (convert_properties(props));
	return i_CreateRequest(sourceIP, (RequestServiceType)service, content_in, *(p.get()), action_type, session_id);
}

CORBA::Boolean ccReg_Log_i::UpdateRequest(ccReg::TID id, const ccReg::RequestProperties &props)
{
	std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
	return i_UpdateRequest(id, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::CloseRequest(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props)
{
	std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
	return i_CloseRequest(id, content_out, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::CloseRequestLogin(ccReg::TID id, const char *content_out, const ccReg::RequestProperties &props, ccReg::TID session_id)
{
	std::auto_ptr<Register::Logger::RequestProperties> p = convert_properties(props);
	return i_CloseRequestLogin(id, content_out, *(p.get()), session_id);
}

ccReg::TID ccReg_Log_i::CreateSession(ccReg::Languages lang, const char *name)
{
	return i_CreateSession((Languages)lang, name);
}

CORBA::Boolean ccReg_Log_i::CloseSession(ccReg::TID id)
{
	return i_CloseSession(id);
}

ccReg::RequestActionList *ccReg_Log_i::GetServiceActions(ccReg::RequestServiceType service)
{
	Result res = i_GetServiceActions((RequestServiceType)service);

	int size  = res.size();
	ccReg::RequestActionList *ret = new ccReg::RequestActionList(size);

	for (int i = 0; i< size; i++) {
		(*ret)[i].service= (ccReg::RequestServiceType)res[i][0];
		(*ret)[i].id 	 = (ccReg::RequestActionType)res[i][1];
		(*ret)[i].status = ((std::string)res[i][2]).c_str();
	}

	return ret;
}




