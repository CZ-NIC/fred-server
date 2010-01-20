#ifndef USERTYPE_CONV_H_
#define USERTYPE_CONV_H_

#include <memory>
#include <corba/ccReg.hh>
#include "register/request.h"



inline std::auto_ptr<Register::Logger::RequestProperties> convert_properties(const ccReg::RequestProperties &p)
{
	std::auto_ptr<Register::Logger::RequestProperties> ret_ptr(new Register::Logger::RequestProperties(p.length()));
	Register::Logger::RequestProperties &ret = *(ret_ptr.get());

	for(unsigned i=0;i<p.length();i++) {
		ret[i].name.assign(p[i].name);
		ret[i].value.assign(p[i].value);
		ret[i].child  = p[i].child;
		ret[i].output = p[i].output;
	}

	return ret_ptr;
}

inline ccReg::RequestProperties convert_properties_d2c(boost::shared_ptr<Register::Logger::RequestProperties> props) 
{
	
	ccReg::RequestProperties res;
	
	res.length(props->size());

	for(unsigned i=0;i<props->size();i++) {
		res[i].name = CORBA::string_dup(props->at(i).name.c_str());
		res[i].value = CORBA::string_dup(props->at(i).value.c_str());
		res[i].child = props->at(i).child;
		res[i].output = props->at(i).output;
	}	

	return res;
}

#endif // USERTYPE_CONV_H_

