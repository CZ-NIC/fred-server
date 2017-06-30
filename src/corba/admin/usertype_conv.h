#ifndef USERTYPE_CONV_H_
#define USERTYPE_CONV_H_

#include <memory>
#include "src/corba/Admin.hh"
#include "src/fredlib/requests/request.h"

inline std::unique_ptr<Fred::Logger::ObjectReferences> convert_obj_references(const ccReg::ObjectReferences &r)
{
    std::unique_ptr<Fred::Logger::ObjectReferences> ret_ptr(new Fred::Logger::ObjectReferences(r.length()));
    Fred::Logger::ObjectReferences &ret = *(ret_ptr.get());

    for(unsigned i=0;i<r.length();i++) {
        // ret[i].type = r[i].type;
        ret[i].type.assign(r[i].type);
        ret[i].id = r[i].id;
    }

    return ret_ptr;
}

inline std::unique_ptr<Fred::Logger::RequestProperties> convert_properties(const ccReg::RequestProperties &p)
{
	std::unique_ptr<Fred::Logger::RequestProperties> ret_ptr(new Fred::Logger::RequestProperties(p.length()));
	Fred::Logger::RequestProperties &ret = *(ret_ptr.get());

	for(unsigned i=0;i<p.length();i++) {
		ret[i].name.assign(p[i].name);
		ret[i].value.assign(p[i].value);
		ret[i].child  = p[i].child;
	}

	return ret_ptr;
}

inline ccReg::RequestPropertiesDetail convert_properties_detail_d2c(boost::shared_ptr<Fred::Logger::RequestPropertiesDetail> props)
{

    ccReg::RequestPropertiesDetail res;

    res.length(props->size());

    for(unsigned i=0;i<props->size();i++) {
        res[i].name = CORBA::string_dup(props->at(i).name.c_str());
        res[i].value = CORBA::string_dup(props->at(i).value.c_str());
        res[i].output = props->at(i).output;
        res[i].child = props->at(i).child;
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

