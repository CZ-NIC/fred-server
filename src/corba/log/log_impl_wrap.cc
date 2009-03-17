#include "log_impl_wrap.h"

#include <corba/ccReg.hh>

std::auto_ptr<LogProperties> convert_properties(const ccReg::LogProperties &p)
{
	std::auto_ptr<LogProperties> ret_ptr(new LogProperties(p.length()));

	LogProperties &ret = *(ret_ptr.get());

	for(int i=0;i<p.length();i++) {
		ret[i].name.assign(p[i].name);
		ret[i].value.assign(p[i].value);
		ret[i].child  = p[i].child;
		ret[i].output = p[i].output;
	}

	return ret_ptr;
}

ccReg::TID ccReg_Log_i::new_event(const char *sourceIP, ccReg::LogServiceType service, const char *content_in, const ccReg::LogProperties& props)
{
	std::auto_ptr<LogProperties> p (convert_properties(props));
	return i_new_event(sourceIP, (LogServiceType)service, content_in, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::update_event(ccReg::TID id, const ccReg::LogProperties &props)
{
	std::auto_ptr<LogProperties> p = convert_properties(props);
	return i_update_event(id, *(p.get()));
}

CORBA::Boolean ccReg_Log_i::update_event_close(ccReg::TID id, const char *content_out, const ccReg::LogProperties &props)
{
	std::auto_ptr<LogProperties> p = convert_properties(props);
	return i_update_event_close(id, content_out, *(p.get()));
}

ccReg::TID ccReg_Log_i::new_session(ccReg::Languages lang, const char *name, const char *clTRID)
{
	return i_new_session((Languages)lang, name, clTRID);
}

CORBA::Boolean ccReg_Log_i::end_session(ccReg::TID id, const char *clTRID)
{
	return i_end_session(id, clTRID);
}



