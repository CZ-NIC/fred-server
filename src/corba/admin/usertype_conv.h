#ifndef USERTYPE_CONV_H_
#define USERTYPE_CONV_H_

#include <memory>
#include <corba/Admin.hh>
#include "fredlib/requests/request.h"

inline std::auto_ptr<Fred::Logger::ObjectReferences> convert_obj_references(const ccReg::ObjectReferences &r)
{
    std::auto_ptr<Fred::Logger::ObjectReferences> ret_ptr(new Fred::Logger::ObjectReferences(r.length()));
    Fred::Logger::ObjectReferences &ret = *(ret_ptr.get());

    for(unsigned i=0;i<r.length();i++) {
        // ret[i].type = r[i].type;
        ret[i].type.assign(r[i].type);
        ret[i].id = r[i].id;
    }

    return ret_ptr;
}

inline std::auto_ptr<Fred::Logger::RequestProperties> convert_properties(const ccReg::RequestProperties &p)
{
	std::auto_ptr<Fred::Logger::RequestProperties> ret_ptr(new Fred::Logger::RequestProperties(p.length()));
	Fred::Logger::RequestProperties &ret = *(ret_ptr.get());

	for(unsigned i=0;i<p.length();i++) {
		ret[i].name.assign(p[i].name);
		ret[i].value.assign(p[i].value);
		ret[i].child  = p[i].child;
		ret[i].output = p[i].output;
	}

	return ret_ptr;
}

inline ccReg::RequestProperties convert_properties_d2c(boost::shared_ptr<Fred::Logger::RequestProperties> props)
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

inline ccReg::ObjectReferences convert_obj_references_d2c(boost::shared_ptr<Fred::Logger::ObjectReferences> refs)
{
        ccReg::ObjectReferences ret;

        ret.length(refs->size());

        for(unsigned i=0;i<refs->size();i++) {
                ret[i].type = CORBA::string_dup(refs->at(i).type.c_str());
                ret[i].id = refs->at(i).id;
        }

        return ret;
}

#endif // USERTYPE_CONV_H_

