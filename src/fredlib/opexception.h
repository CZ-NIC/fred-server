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

#include <algorithm>
#include <functional>
#include <boost/function.hpp>


///operation exception base class
struct OperationExceptionBase
: virtual public std::exception  //common base
{
    virtual const char* what() const throw() = 0;
    virtual ~OperationExceptionBase() throw() {};
};

/// const array wrapper
struct ConstArr
{
    const char** arr;
    int size;
    ConstArr(const char** list /// static const char* [] instance like {"string1","string2","string3"}
            , int _size)/// sizeof(list)/sizeof(char*) - number of elements in the list , cannot be wrapped
            throw()
        :arr(list)
        , size(_size)
    {}
};


struct CopyEndImpl
{
    /**
     * copy end of the asciiz string first from "from" to "to" with regard to "size_of_to"
     */
    void copy_end(char * to, const char * from , int size_of_to ) throw()
    {
        int len_of_to = strlen(to);
        int len_of_from = strlen(from);
        int space_left_in_to = size_of_to - len_of_to;

        if (space_left_in_to >= len_of_from)
        {//all data fits in
            memmove(to+len_of_from, to, len_of_to);//make space
            memmove(to, from, len_of_from);//copy data
            to[len_of_from+len_of_to] = '\0';//end by zero
        }
        else
        {//store only end of the data
            memmove(to + space_left_in_to, to, len_of_to);//make space
            memmove(to, from+ (len_of_from - space_left_in_to), space_left_in_to);//copy data
            to[space_left_in_to+len_of_to] = '\0';//end by zero
        }
    }
protected:
    ~CopyEndImpl() throw() {}
};

///fixed string wapper
template <int DATASIZE///size of string
> struct FixedString
: private CopyEndImpl
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
        copy_end(data, str, sizeof(data));
    }
};

/**
 * separation of reason, param and value
 * with optional filtering by _key_substring search in reason and param
 */

template <class EXCEPTION> struct SearchCallback
: private CopyEndImpl
{
    typedef SearchCallback<EXCEPTION> SearchCallbackType;
    typedef typename EXCEPTION::FixedStringType FixedStringType;

    typedef boost::function<void (FixedStringType reason, FixedStringType param, FixedStringType value)> CallbackType;
    FixedStringType key_substring;
    bool is_key_set;
    CallbackType callback;
    EXCEPTION ex;

    SearchCallback(FixedStringType _key_substring
            , CallbackType _callback
            , EXCEPTION _ex)
    : key_substring(_key_substring)
    , is_key_set(strlen(_key_substring.data) == 0 ? false : true )
    , callback(_callback)
    , ex(_ex)
    {}

    void exception_callback(FixedStringType str)
    {
        //printf("\ncheck_param: %s",str.data);
        ConstArr expected_reasons = ex.get_fail_reason();
        ConstArr expected_params = ex.get_fail_param();

        FixedStringType expected_key;
        for(int i = 0; i < expected_reasons.size ; ++i)
        {
            for(int j = 0; j < expected_params.size; ++j)
            {
                expected_key = FixedStringType();//init
                copy_end(expected_key.data,":",sizeof(expected_key.data));
                copy_end(expected_key.data,expected_params.arr[j],sizeof(expected_key.data));
                copy_end(expected_key.data,":",sizeof(expected_key.data));
                copy_end(expected_key.data,expected_reasons.arr[i],sizeof(expected_key.data));

                if(strncmp(expected_key.data, str.data,strlen(expected_key.data)) == 0)
                {//ok is valid expected_key
                    /*
                    printf("\nreason: %s param: %s value: %s\n"
                            ,expected_reasons.arr[i]
                            , expected_params.arr[j]
                            , str.data + strlen(expected_key.data)
                            );
                    */
                    if(!is_key_set || strstr(expected_key.data,key_substring.data))
                        callback(expected_reasons.arr[i],expected_params.arr[j],str.data + strlen(expected_key.data));

                }
            }//for expected params
        }//for expected reasons
    }

    void run()
    {
        //run callback for data in exception
        typename EXCEPTION::FixedStringFunc cbfun = std::bind1st(std::mem_fun(&SearchCallbackType::exception_callback), this);
        ex.for_params(cbfun);
    }
};

/**
 * operation error template, able of copying
 * throwing instance of this template shall be considered part of bad path
 * intended meaning is abort operation because of unexpected reasons
 * like something is broken and need to be fixed
 * implementation shall be nothrow and using stack memory only
 */

template <
    int DATASIZE ///size of internal buffer for detail of failure
    , class EXCEPTION_BASE ///additional exception types
    >
class OperationError
    : virtual public OperationExceptionBase
    , virtual public EXCEPTION_BASE
    , private CopyEndImpl
{
    ///any data of failure
    char databuffer_[DATASIZE+1];///+1 for ending by \0

public:
    /**
     * dump data
     */
    const char* what() const throw()
    {
        return databuffer_;
    }

    /**
     * ctor
     * intended for situations where data already contains enough details of failure
     * like failure of OperationException instance
     */
    OperationError(const char* data) throw()
        : databuffer_()
    {
        //fill databuffer
        copy_end(databuffer_, data, sizeof(databuffer_));//databuffer
        copy_end(databuffer_, "OperationError: ", sizeof(databuffer_));
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
    : databuffer_()
    {
        //fill databuffer
        copy_end(databuffer_, data, sizeof(databuffer_));//databuffer
        copy_end(databuffer_, " ", sizeof(databuffer_));//space
        copy_end(databuffer_, function, sizeof(databuffer_));//function
        copy_end(databuffer_, " ", sizeof(databuffer_));//space
        char line_str[20]={'\0'};
        snprintf(line_str,sizeof(line_str), ":%d", line);
        copy_end(databuffer_, line_str, sizeof(databuffer_));//line
        copy_end(databuffer_, file, sizeof(databuffer_));//file
        copy_end(databuffer_, "OperationError: ", sizeof(databuffer_));
    }
};


/**
 * operation exception template, able of copying
 * throwing instance of this template shall be considered part of good path
 * intended meaning is abort operation or part of operation because of anticipated reasons
 * like invalid input arguments, that shall be enumerated in parsable section of exception data viz ctor
 */

template <
    int DATASIZE ///size of internal buffer for detail of failure
    , class EXCEPTION_BASE ///additional exception types / interfaces
    , class FAIL_PARAM_ARRAY ///array of parameters names which may fail
    , class FAIL_REASON_ARRAY///array of reasons why may parameter fail
    >
class OperationException
    : virtual public OperationExceptionBase
    , virtual public EXCEPTION_BASE
    , public FAIL_PARAM_ARRAY
    , public FAIL_REASON_ARRAY
    , private CopyEndImpl
{
    /**
     * data shall be stored from end to begin
     * data shall look like this:
     * optional text with some details followed by fail reasons, params and args of the operation
     * any details || reason1:param1: val1 | reason2:param2: val2 ... | reason#:param#: val# |
     * separating character '|' in data (reason, param or value) shell be replaced by description like <pipe>
     */
    char databuffer_[DATASIZE+1];///+1 for ending by \0

public:
    typedef OperationException<DATASIZE, EXCEPTION_BASE, FAIL_PARAM_ARRAY, FAIL_REASON_ARRAY> OperationExceptionType;
    typedef FixedString<DATASIZE> FixedStringType;
    typedef boost::function<void (FixedStringType str)> FixedStringFunc;
    typedef OperationError<DATASIZE,EXCEPTION_BASE> OperationErrorType;

    /**
     * check str against expected reasons and parameters
     * throw if invalid
     */
    void check_key(FixedStringType str)
    {
        bool key_is_valid = false;
        //printf("\ncheck_param: %s",str.data);
        ConstArr expected_reasons = this->get_fail_reason();
        ConstArr expected_params =this->get_fail_param();

        FixedStringType key;
        for(int i = 0; i < expected_reasons.size ; ++i)
        {
            for(int j = 0; j < expected_params.size; ++j)
            {
                key = FixedStringType();//init
                copy_end(key.data,":",sizeof(key.data));
                copy_end(key.data,expected_params.arr[j],sizeof(key.data));
                copy_end(key.data,":",sizeof(key.data));
                copy_end(key.data,expected_reasons.arr[i],sizeof(key.data));

                if(strncmp(key.data, str.data,strlen(key.data)) == 0)
                {//ok is valid key
                    key_is_valid = true;
                    //printf("\ncheck_param valid key: %s",key.data);
                    break;
                }
            }//for expected params
            if(key_is_valid) break;
        }//for expected reasons
        if(!key_is_valid)//if parsable exception content is not valid then throw error
        {
            copy_end(databuffer_, " ", sizeof(databuffer_));//space
            copy_end(databuffer_, key.data, sizeof(databuffer_));//err
            copy_end(databuffer_, "check_key failed, invalid key: ", sizeof(databuffer_));//err
            throw OperationErrorType(databuffer_);
        }
    }

    /**
     * for exception params separated by '| ' from end to begin marked by '|| 'call func callback
     * callback may throw
     */
    void for_params(FixedStringFunc func)
    {
        int len_of_data = strlen(databuffer_);
        int last_separator_index = 0;
        for(int i = len_of_data - 1; i > -1; --i)//search data backward
        {
            //printf("\nfor i: %d\n", i);
            if(databuffer_[i] == '|')//if separator found
            {
                if(last_separator_index  == 0)
                {
                    last_separator_index = i;
                }
                else
                {
                    FixedStringType str;
                    char* data_ptr = databuffer_+i+2;
                    int data_len = databuffer_ + last_separator_index - 1 - data_ptr;
                    memmove(str.data,data_ptr,data_len);//ok if str have DATASIZE same as databuffer_ or bigger
                    //printf("\n found %s at: %d\n", str.data, i);
                    if (func)
                    {
                        func(str);
                    }
                    else
                    {
                        copy_end(databuffer_, "for_params failed, invalid func ", sizeof(databuffer_));//err
                        throw std::runtime_error(databuffer_);
                    }
                    //printf("\nlook_for last_separator_index: %d\n", last_separator_index);
                    last_separator_index = i;
                }
                //check border ||
                if ((i > 0) && (databuffer_[i-1] == '|'))
                {
                    //printf("\nborder found | at: %d\n", i);
                    break;
                }
            }//if |
        }//for i

    }//check_params

    /**
     * dump data
     */
    const char* what() const throw()
    {
        return databuffer_;
    }

    /**
     * ctor
     * file, line and function describe origin in source and shall have values from __FILE__, __LINE__ and __ASSERT_FUNCTION macros
     * data shall contain any details of failure ended by parsable exception params like:
     * some failure details || reason1:param1: value1 | reason2:param2: value2 | reason3:param3: value3 |
     * where valid reasons and params are enumerated in template parameters FAIL_REASON_ARRAY and FAIL_PARAM_ARRAY
     * reasons params and values shall not contain character '|' used for separation
     * if there shall be no parsable params, data shall end by '||'
     *
     * ctor of OperationException may fail and failure shall throw OperationError instance
     */

    OperationException(const char* file
            , const int line
            , const char* function
            , const char* data)
    : databuffer_()
    {
        //fill databuffer
        copy_end(databuffer_, data, sizeof(databuffer_));//databuffer
        copy_end(databuffer_, " ", sizeof(databuffer_));//space
        copy_end(databuffer_, function, sizeof(databuffer_));//function
        copy_end(databuffer_, " ", sizeof(databuffer_));//space
        char line_str[20]={'\0'};
        snprintf(line_str,sizeof(line_str), ":%d", line);
        copy_end(databuffer_, line_str, sizeof(databuffer_));//line
        copy_end(databuffer_, file, sizeof(databuffer_));//file

        //run check for reason-param data in buffer
        FixedStringFunc check_data = std::bind1st(std::mem_fun(&OperationExceptionType::check_key), this);
        for_params(check_data);
    }

    /**
     * call supplied function with exception params optionally filtered by by substring search in reason and param
     */

    void callback_exception_params(boost::function<void (FixedStringType reason, FixedStringType param, FixedStringType value)> func_void3fixedstr
            , const char key_substring[] = "")
    {
        SearchCallback<OperationExceptionType> (key_substring,func_void3fixedstr,*this).run();//exec
    }

};

#endif // OPEXCEPTION_H_
