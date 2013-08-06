/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file opexception.h
 *  operation exceptions
 */

#ifndef OPEXCEPTION_H_
#define OPEXCEPTION_H_

#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <assert.h>
#include <string>
#include <vector>
#include <map>

#include <algorithm>
#include <functional>
#include <boost/function.hpp>
#include <boost/exception/all.hpp>

#include "fredlib/db_settings.h"
#include "util/db/nullable.h"

namespace Fred
{

///exception stack context info
struct ExceptionStack
: virtual boost::exception
{
    ///string tag to record/append operation stack context
    typedef boost::error_info<struct OperationStackTag,std::string> ErrorInfoOperationStack;

    ///append operation stack context
    void add_exception_stack_info(const std::string& info)
    {
        std::string operation_stack_info;
        const std::string* ptr = 0;
        ptr = get_data_ptr();
        if(ptr) operation_stack_info += *ptr;
        operation_stack_info += info;
        (*this) << ErrorInfoOperationStack(operation_stack_info);
    }

    ///get exception stack info data
    std::string get_exception_stack_info() const
    {
        const std::string* data_ptr = get_data_ptr();
        return data_ptr ? *data_ptr : std::string();
    }

    ///check if exception stack info is set
    bool is_set_exception_stack_info() const
    {
        const std::string* data_ptr = get_data_ptr();
        return data_ptr;
    }

    virtual ~ExceptionStack() throw() {}

private:

    const std::string* get_data_ptr() const
    {
        return boost::get_error_info<ErrorInfoOperationStack>(*this);
    }
};

///exception data set flag
struct ThrowMeFlagImpl
{
private:
    bool throw_me_;
public:
    ///if exception data is set, throw exception
    bool throw_me()
    {
        return throw_me_;
    }
    ///setter is called by exception data setters or directly if using operator<< with error_info type
    void set_throw_me()
    {
        throw_me_ = true;
    }
protected:
    ThrowMeFlagImpl()
    : throw_me_(false)
    {}
    ~ThrowMeFlagImpl() throw () {}
};

///parent of operation exceptions
struct OperationException
    : virtual std::exception
    , virtual ExceptionStack
    , virtual ThrowMeFlagImpl
{
    ///std::exception content override
    const char* what() const throw()
    {
        return "OperationException";
    }
    virtual ~OperationException() throw() {}
};

///internal error exception with describing message
struct InternalError
: virtual std::exception
, virtual ExceptionStack
{
    explicit InternalError(std::string const& message)
        : msg_(message)
        {}
    explicit InternalError(const char* message)
    : msg_(message)
    {}
    ///std::exception content override
    virtual const char* what() const throw ()
    {
        try
        {
            return msg_.c_str();
        }
        catch(...)
        {}
        return "error: std::string::c_str() exception";
    }

    virtual ~InternalError() throw() try{}catch(...){}
private:
    std::string msg_;
};



///declaration of exception tag related methods getter and chaining setter with error_info type
#define DECLARE_EXCEPTION_DATA(ex_data_tag, ex_data_type) \
typedef boost::error_info<BOOST_JOIN(struct ExceptionTag_,ex_data_tag),ex_data_type> BOOST_JOIN(ErrorInfo_,ex_data_tag);\
template <class DERIVED_EXCEPTION> struct BOOST_JOIN(ExceptionData_,ex_data_tag)\
{\
public:\
    typedef BOOST_JOIN(ErrorInfo_,ex_data_tag) error_info_type;\
private:\
\
    const DERIVED_EXCEPTION* get_derived_ptr() const\
    {\
        return static_cast<const DERIVED_EXCEPTION*>(this);\
    }\
    const ex_data_type* get_data_ptr() const\
    {\
        return boost::get_error_info<error_info_type>(*(get_derived_ptr()));\
    }\
public:\
    DERIVED_EXCEPTION& BOOST_JOIN(set_,ex_data_tag)(const ex_data_type& arg)\
    {\
        DERIVED_EXCEPTION& ex = *static_cast<DERIVED_EXCEPTION*>(this);\
        ex << error_info_type(arg);\
        ex.set_throw_me();\
        return ex;\
    }\
    ex_data_type BOOST_JOIN(get_,ex_data_tag)() const\
    {\
        const ex_data_type* data_ptr = get_data_ptr();\
        return data_ptr ? *data_ptr : error_info_type::value_type();\
    }\
    bool BOOST_JOIN(is_set_,ex_data_tag)() const\
    {\
        const ex_data_type* data_ptr = get_data_ptr();\
        return data_ptr;\
    }\
protected:\
    BOOST_JOIN(ExceptionData_,ex_data_tag)(){}\
    BOOST_JOIN(~ExceptionData_,ex_data_tag)() throw () {}\
}\

///declaration of std::vector based exception tag, related methods, getter and chaining setter with error_info type
#define DECLARE_VECTOR_OF_EXCEPTION_DATA(ex_data_tag, ex_data_type) \
typedef boost::error_info<BOOST_JOIN(struct ExceptionTag_vector_of_,ex_data_tag), std::vector<ex_data_type> > BOOST_JOIN(ErrorInfo_vector_of_,ex_data_tag);\
\
static std::string to_string(const BOOST_JOIN(ErrorInfo_vector_of_,ex_data_tag)& info)\
{\
    std::ostringstream oss;\
    oss << "vector data:";\
    for(BOOST_JOIN(ErrorInfo_vector_of_,ex_data_tag)::value_type::const_iterator ci = info.value().begin()\
            ; ci != info.value().end(); ++ci) oss << ' ' << *ci;\
    return oss.str();\
}\
\
template <class DERIVED_EXCEPTION> struct BOOST_JOIN(ExceptionData_vector_of_,ex_data_tag)\
{\
public:\
    typedef BOOST_JOIN(ErrorInfo_vector_of_,ex_data_tag) error_info_type;\
private:\
\
    const DERIVED_EXCEPTION* get_derived_ptr() const\
    {\
        return static_cast<const DERIVED_EXCEPTION*>(this);\
    }\
    const std::vector<ex_data_type>* get_data_ptr() const\
    {\
        return boost::get_error_info<error_info_type>(*(get_derived_ptr()));\
    }\
public:\
    DERIVED_EXCEPTION& BOOST_JOIN(set_vector_of_,ex_data_tag)(const std::vector<ex_data_type>& arg)\
    {\
        DERIVED_EXCEPTION& ex = *static_cast<DERIVED_EXCEPTION*>(this);\
        ex << error_info_type(arg);\
        ex.set_throw_me();\
        return ex;\
    }\
    std::vector<ex_data_type> BOOST_JOIN(get_vector_of_,ex_data_tag)() const\
    {\
        const std::vector<ex_data_type>* data_ptr = get_data_ptr();\
        return data_ptr ? *data_ptr : std::vector<ex_data_type>();\
    }\
    bool BOOST_JOIN(is_set_vector_of_,ex_data_tag)() const\
    {\
        const std::vector<ex_data_type>* data_ptr = get_data_ptr();\
        return data_ptr;\
    }\
    DERIVED_EXCEPTION& BOOST_JOIN(add_,ex_data_tag)(const ex_data_type& arg)\
    {\
        std::vector<ex_data_type> data_vector = BOOST_JOIN(get_vector_of_,ex_data_tag)();\
        data_vector.push_back(arg);\
        return BOOST_JOIN(set_vector_of_,ex_data_tag)(data_vector);\
    }\
protected:\
    BOOST_JOIN(ExceptionData_vector_of_,ex_data_tag)(){}\
    BOOST_JOIN(~ExceptionData_vector_of_,ex_data_tag)() throw () {}\
}\

///common exception data tags
DECLARE_EXCEPTION_DATA(unknown_contact_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_registry_object_identifier, std::string);
DECLARE_EXCEPTION_DATA(unknown_domain_fqdn, std::string);
DECLARE_EXCEPTION_DATA(unknown_nsset_handle, Nullable<std::string>);
DECLARE_EXCEPTION_DATA(unknown_keyset_handle, Nullable<std::string>);
DECLARE_EXCEPTION_DATA(unknown_registrant_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_object_type, std::string);
DECLARE_EXCEPTION_DATA(unknown_admin_contact_handle, std::string);
DECLARE_EXCEPTION_DATA(already_set_admin_contact_handle, std::string);
DECLARE_EXCEPTION_DATA(unknown_technical_contact_handle, std::string);
DECLARE_EXCEPTION_DATA(already_set_technical_contact_handle, std::string);

}//namespace Fred
#endif // OPEXCEPTION_H_
