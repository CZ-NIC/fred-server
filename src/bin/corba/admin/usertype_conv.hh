/*
 * Copyright (C) 2009-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef USERTYPE_CONV_HH_63906E0168FA496B959A95DF14200295
#define USERTYPE_CONV_HH_63906E0168FA496B959A95DF14200295

#include <memory>
#include "corba/Admin.hh"
#include "src/deprecated/libfred/requests/request.hh"

inline std::unique_ptr<LibFred::Logger::ObjectReferences> convert_obj_references(const ccReg::ObjectReferences &r)
{
    std::unique_ptr<LibFred::Logger::ObjectReferences> ret_ptr(new LibFred::Logger::ObjectReferences(r.length()));
    LibFred::Logger::ObjectReferences &ret = *(ret_ptr.get());

    for(unsigned i=0;i<r.length();i++) {
        // ret[i].type = r[i].type;
        ret[i].type.assign(r[i].type);
        ret[i].id = r[i].id;
    }

    return ret_ptr;
}

inline std::unique_ptr<LibFred::Logger::RequestProperties> convert_properties(const ccReg::RequestProperties &p)
{
	std::unique_ptr<LibFred::Logger::RequestProperties> ret_ptr(new LibFred::Logger::RequestProperties(p.length()));
	LibFred::Logger::RequestProperties &ret = *(ret_ptr.get());

	for(unsigned i=0;i<p.length();i++) {
		ret[i].name.assign(p[i].name);
		ret[i].value.assign(p[i].value);
		ret[i].child  = p[i].child;
	}

	return ret_ptr;
}

inline ccReg::RequestPropertiesDetail convert_properties_detail_d2c(std::shared_ptr<LibFred::Logger::RequestPropertiesDetail> props)
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


inline ccReg::ObjectReferences convert_obj_references_d2c(std::shared_ptr<LibFred::Logger::ObjectReferences> refs)
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

