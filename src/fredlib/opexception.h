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

///parent of operation exceptions
struct OperationException
    : virtual std::exception
    , virtual ExceptionStack
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
        return ex;\
    }\
    ex_data_type BOOST_JOIN(get_,ex_data_tag)() const\
    {\
        const ex_data_type* data_ptr = get_data_ptr();\
        return data_ptr ? *data_ptr : ex_data_type();\
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

/// const array wrapper
class ConstArr
{
    const char** arr_;
    int size_;
public:
    ConstArr(const char** list /// static const char* [] instance like {"string1","string2","string3"}
            , int _size)/// sizeof(list)/sizeof(char*) - number of elements in the list , cannot be wrapped
            throw()
        :arr_(list)
        , size_(_size)
    {}

    const char* operator[](const int index) const
    {
        return arr_[index];
    }

    int size() const
    {
        return size_;
    }
};

///operation exception base class
struct OperationExceptionBase
: virtual public std::exception  //common base
{
    typedef boost::function<void (const char* str)> Func1Str;
    typedef boost::function<void (const char* param, const char* value)> FuncParamValue;

    /**
     * call func callback for all parsable params
     * callback may throw
     */
    virtual void for_params(Func1Str func) = 0;

    /**
     * call supplied function with exception params
     * callback may throw
     */
    virtual void callback_exception_params(FuncParamValue func) =0;

    /**
     * call supplied function with exception params filtered by substring search
     * callback may throw
     */
    virtual void callback_exception_params(FuncParamValue func, const char* key_substring) =0;

    /**
     * parameters of failure enumerated
     */
    virtual ConstArr get_fail_param() throw() = 0;

    virtual const char* what() const throw() = 0;
    virtual ~OperationExceptionBase() throw() {};
};



///get params data into multimap callback, throwing
///use instance by boost::ref to keep state
class GetOperationExceptionParamsDataToMmapCallback
{
public:
    typedef std::multimap<std::string, std::string> Mmap;
private:
    Mmap data_;

public:

    /**
     * callback accumulating data
     */

    void operator()(const char* param, const char* value)
    {
        data_.insert(Mmap::value_type(param, value));
    }

    /**
     * get acumulated data
     */

    Mmap get()
    {
        return data_;
    }
};

///get params data into bool callback, non throwing
///use instance by boost::ref to keep state
class GetOperationExceptionParamsDataToBoolCallback
{
    bool data_;

public:

    GetOperationExceptionParamsDataToBoolCallback() throw()
    : data_(false)
    {}

    /**
     * callback ignoring actual data, but registering presence
     */

    void operator()(const char* //param
            , const char* //value
            ) throw()
    {
        data_= true;
    }

    /**
     * get acumulated data
     */

    bool get() throw()
    {
        return data_;
    }
};


///operation error exception base class
struct OperationErrorBase
: virtual public std::exception  //common base
{
    virtual const char* what() const throw() = 0;
    virtual ~OperationErrorBase() throw() {};
};


///fixed string wapper
template <int DATASIZE///size of string
> struct FixedString
{
    char data[DATASIZE+1];///+1 for ending by \0
    //dtor
    ~FixedString() throw(){}
    //ctor
    FixedString() throw()
    :data()
    {}
    FixedString(const char* str) throw()
    :data()
    {
        push_front(str);
    }

    /**
     * push the asciiz string from "from" in front of member data with regard to size of data member
     */
    void push_front(const char * from) throw()
    {
        char *to = data;
        int size_of_to = sizeof(data);

        int len_of_to = strlen(to);
        int len_of_from = strlen(from);
        int space_left_in_to = size_of_to - 1 - len_of_to;

        if (space_left_in_to >= len_of_from)
        {//all data fits in
            memmove(to+len_of_from, to, len_of_to);//make space
            memmove(to, from, len_of_from);//copy data
            to[len_of_from+len_of_to] = '\0';//end by zero
        }
        else if (space_left_in_to > 0)
        {//store only end of the data
            memmove(to + space_left_in_to, to, len_of_to);//make space
            memmove(to, from+ (len_of_from - space_left_in_to), space_left_in_to);//copy data
            to[space_left_in_to+len_of_to] = '\0';//end by zero
        }
    }//push_front

    /**
     * push the asciiz string from "from" to the end of data with regard to size of data member
     */
    void push_back(const char * from) throw()
    {
        char *to = data;
        int size_of_to = sizeof(data);

        int len_of_to = strlen(to);
        int len_of_from = strlen(from);
        int space_left_in_to = size_of_to - 1 - len_of_to;

        if (space_left_in_to >= len_of_from)
        {//all data fits in
            memmove(to+len_of_to, from, len_of_from);//copy data
            to[len_of_to+len_of_from] = '\0';//end by zero
        }
        else if (space_left_in_to > 0)
        {//store only end of the data
            memmove(to+len_of_to, from, space_left_in_to);//copy data
            to[len_of_to+len_of_from] = '\0';//end by zero
        }
    }//push_front
};



/**
 * separation of param and value
 * with optional filtering by _key_substring search in param name
 */
template <int DATASIZE> struct SearchCallbackImpl
{
    typedef SearchCallbackImpl<DATASIZE> SearchCallbackType;
    typedef FixedString<DATASIZE> FixedStringType;

    typedef boost::function<void (const char * param, const char * value)> CallbackType;
    FixedStringType key_substring;
    bool is_key_set;
    CallbackType callback;
    ConstArr expected_params;

    SearchCallbackImpl(FixedStringType _key_substring
            , CallbackType _callback
            , ConstArr _expected_params
            )
    : key_substring(_key_substring)
    , is_key_set(strlen(_key_substring.data) == 0 ? false : true )
    , callback(_callback)
    , expected_params(_expected_params)
    {}

    void exception_callback(const char* str)
    {
        //printf("\ncheck_param: %s",str.data);

        FixedStringType expected_key;

        for(int i = 0; i < expected_params.size(); ++i)
        {
            expected_key = FixedStringType();//init
            expected_key.push_front(":");
            expected_key.push_front(expected_params[i]);

            if(strncmp(expected_key.data, str,strlen(expected_key.data)) == 0)
            {//ok is valid expected_key
                /*
                printf("\nparam: %s value: %s\n"
                        , expected_params.arr[i]
                        , str + strlen(expected_key.data)
                        );
                */
                if(!is_key_set || strstr(expected_key.data,key_substring.data))
                    callback(expected_params[i],str + strlen(expected_key.data));

            }
        }//for expected params
    }
};//SearchCallbackImpl

/**
 * operation error crtp parent template
 * throwing child of this template shall be considered part of bad path
 * intended meaning is abort operation because of unexpected reasons
 * like something is broken and need to be fixed
 * implementation shall be nothrow and using stack memory only
 */

template < int DATASIZE ///size of internal buffer for detail of failure
    >
class OperationError
: public OperationErrorBase
{
protected:
    ///any data of failure
    FixedString<DATASIZE> fs;

public:
    /**
     * dump data
     */
    virtual const char* what() const throw()
    {
        return fs.data;
    }

    /**
     * ctor
     * intended for situations where data already contains enough details of failure
     * like failure of OperationExceptionImpl instance
     */
    OperationError(const char* data) throw()
    {
        //fill exception data
        fs.push_front(data);
        fs.push_front("OperationError: ");
    }
    /**
     * ctor
     * file, line and function describe origin in source and shall have values from __FILE__, __LINE__ and __ASSERT_FUNCTION macros
     * data shall contain any details of failure
     */
    OperationError(const char* file
            , const int line
            , const char* function
            , const char* data) throw()
    {
        //fill databuffer
        fs.push_front(data);//exception data
        fs.push_front(" ");//space
        fs.push_front(function);//function
        fs.push_front(" ");//space
        char line_str[20]={'\0'};
        snprintf(line_str,sizeof(line_str), ":%d", line);
        fs.push_front(line_str);//line
        fs.push_front(file);//file
        fs.push_front("OperationError: ");
    }
};

/**
 * crtp parent template
 * throwing child of this template shall be considered part of good path
 * intended meaning is abort operation or part of operation because of anticipated reasons
 * like invalid input arguments, that shall be enumerated in parsable section of exception data viz ctor
 */

template < class EXCEPTION_CHILD ///child exception type
            , int DATASIZE ///size of internal buffer for detail of failure
    >
class OperationExceptionImpl
: public OperationExceptionBase
{
public:
    typedef FixedString<DATASIZE> FixedStringType;
    typedef OperationExceptionImpl<EXCEPTION_CHILD, DATASIZE> OperationExceptionImplType;
    typedef OperationExceptionBase::Func1Str Func1Str;
    typedef OperationExceptionBase::FuncParamValue FuncParamValue;
    typedef OperationError<DATASIZE> OperationErrorType;

    /**
     * get valid exception params from child
     */

    ConstArr get_fail_param() throw()
    {
        return static_cast<EXCEPTION_CHILD*>(this)->get_fail_param_impl();
    }

    /**
     * dump exception data
     */

    const char* what() const throw()
    {
        return fs.data;
    }

    /**
     * check str against expected parameters
     * throw if invalid
     */

    void check_key(FixedStringType str)
    {
        bool key_is_valid = false;
        //printf("\ncheck_param: %s",str.data);
        ConstArr expected_params = get_fail_param();

        FixedStringType key;
        for(int i = 0; i < expected_params.size(); ++i)
        {
            key = FixedStringType();//init
            key.push_front(":");
            key.push_front(expected_params[i]);

            //printf("\ncheck_key compare key: %s\n",key.data);
            if((strlen(str.data) >= strlen(key.data))&&(strncmp(key.data, str.data,strlen(key.data)) == 0))
            {//ok is valid key
                key_is_valid = true;
                //printf("\ncheck_key valid key: %s str: %s\n",key.data, str.data);
                break;
            }
        }//for expected params

        if(!key_is_valid)//if parsable exception content is not valid then throw error
        {
            fs.push_front(" ");//space
            fs.push_front(str.data);//err
            fs.push_front("check_key failed, invalid key: ");//err
            throw OperationErrorType(fs.data);
        }
    }

    /**
     * call supplied function with exception params
     * callback may throw
     */

    void callback_exception_params(FuncParamValue func_void2str)
    {
        SearchCallbackImpl<DATASIZE> cb ("",func_void2str,get_fail_param());//callback instance
        //run callback for data in exception
        Func1Str cbfun = std::bind1st(std::mem_fun(&SearchCallbackImpl<DATASIZE>::exception_callback), &cb);
        for_params(cbfun);
    }

    /**
     * call supplied function with exception params filtered by substring search in reason and param
     * callback may throw
     */

    void callback_exception_params(FuncParamValue func_void2str
            , const char* key_substring)
    {
        SearchCallbackImpl<DATASIZE> cb (key_substring,func_void2str,get_fail_param());//callback instance
        //run callback for data in exception
        Func1Str cbfun = std::bind1st(std::mem_fun(&SearchCallbackImpl<DATASIZE>::exception_callback), &cb);
        for_params(cbfun);
    }

    /**
     * for exception params separated by '| ' from end to begin marked by '|| 'call func callback
     * callback may throw
     */
    void for_params(Func1Str func)
    {
        int len_of_data = strlen(fs.data);
        int last_separator_index = 0;
        for(int i = len_of_data - 1; i > -1; --i)//search data backward
        {
            //printf("\nfor i: %d\n", i);
            if(fs.data[i] == '|')//if separator found
            {
                if(last_separator_index  == 0)
                {
                    last_separator_index = i;
                }
                else
                {
                    FixedStringType str;
                    char* data_ptr = fs.data+i+2;
                    int data_len = fs.data + last_separator_index - 1 - data_ptr;
                    memmove(str.data,data_ptr,data_len);//ok if str have DATASIZE same as databuffer_ or bigger
                    //printf("\n found %s at: %d\n", str.data, i);
                    if (func)
                    {
                        func(str.data);
                    }
                    else
                    {
                        fs.push_front("invalid func ");//err
                        throw OperationErrorType(fs.data);
                    }
                    //printf("\nlook_for last_separator_index: %d\n", last_separator_index);
                    last_separator_index = i;
                }
                //check border ||
                if ((i > 0) && (fs.data[i-1] == '|'))
                {
                    //printf("\nborder found | at: %d\n", i);
                    break;
                }
            }//if |
        }//for i

    }//for_params


protected:
    FixedStringType fs;

    OperationExceptionImpl(const char* file
            , const int line
            , const char* function
            , const char* data)
    {
        //fill databuffer
        fs.push_front(data);//databuffer
        fs.push_front(" ");//space
        fs.push_front(function);//function
        fs.push_front(" ");//space
        char line_str[20]={'\0'};
        snprintf(line_str,sizeof(line_str), ":%d", line);
        fs.push_front(line_str);//line
        fs.push_front(file);//file

        //run check for reason-param data in buffer
        Func1Str check_data = std::bind1st(std::mem_fun(&OperationExceptionImplType::check_key), this);
        for_params(check_data);
    }


    ~OperationExceptionImpl() throw() {};

};//OperationExceptionImpl

/**
 * common exception handler for operation exec
 * ment to be called from inside of the catch(...) block
 */


template <class OPERATION_EXCEPTION>
void handleOperationExceptions(const char* file
        , const int line
        , const char* function)
{
    typedef typename OPERATION_EXCEPTION::OperationErrorType OperationError;

    try
    {
        throw;//rethrow
    }
    //translate specific exceptions to operation specific exceptions
    catch(Database::ResultFailed& ex)
    {
        throw OPERATION_EXCEPTION(file, line, function, ex.what());
    }
    //rethrow already operation specific exceptions
    catch(OPERATION_EXCEPTION&)
    {
        throw;
    }
    //rethrow other operation exceptions
    catch(OperationExceptionBase& ex)
    {
        throw;
    }
    //rethrow already operation specific error exceptions
    catch(OperationError&)
    {
        throw;
    }
    //rethrow other operation error exceptions
    catch(OperationErrorBase& ex)
    {
        throw;
    }
    //translate other std exceptions to operation specific error exceptions
    catch(std::exception& ex)
    {
        throw OperationError(file, line, function, ex.what());
    }
    //propagate anything else
    catch(...)
    {
        throw;
    }
}//handleOperationExceptions



}//namespace Fred
#endif // OPEXCEPTION_H_
