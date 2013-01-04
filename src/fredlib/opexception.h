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

namespace Fred
{

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

///operation exception base class
struct OperationExceptionBase
: virtual public std::exception  //common base
{
    virtual ConstArr get_fail_reason() throw() = 0;
    virtual ConstArr get_fail_param() throw() = 0;
    virtual const char* what() const throw() = 0;
    virtual ~OperationExceptionBase() throw() {};
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
    }//push_front
};

/**
 * separation of reason, param and value
 * with optional filtering by _key_substring search in reason and param
 */

template <class EXCEPTION> struct SearchCallback
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
                expected_key.push_front(":");
                expected_key.push_front(expected_params.arr[j]);
                expected_key.push_front(":");
                expected_key.push_front(expected_reasons.arr[i]);

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
class OperationError_
    : virtual public EXCEPTION_BASE
{
    ///any data of failure
    FixedString<DATASIZE> fs;

public:
    /**
     * dump data
     */
    const char* what() const throw()
    {
        return fs.data;
    }

    /**
     * ctor
     * intended for situations where data already contains enough details of failure
     * like failure of OperationException instance
     */
    OperationError_(const char* data) throw()
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
    OperationError_(const char* file
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
    : virtual public EXCEPTION_BASE
    , public FAIL_PARAM_ARRAY
    , public FAIL_REASON_ARRAY
{
public:
    typedef OperationException<DATASIZE, EXCEPTION_BASE, FAIL_PARAM_ARRAY, FAIL_REASON_ARRAY> OperationExceptionType;
    typedef FixedString<DATASIZE> FixedStringType;
    typedef boost::function<void (FixedStringType str)> FixedStringFunc;
    typedef OperationError_<DATASIZE,EXCEPTION_BASE> OperationErrorType;
private:
    /**
     * data shall be stored from end to begin
     * data shall look like this:
     * optional text with some details followed by fail reasons, params and args of the operation
     * any details || reason1:param1: val1 | reason2:param2: val2 ... | reason#:param#: val# |
     * separating character '|' in data (reason, param or value) shell be replaced by description like [pipe]
     */
    FixedStringType fs;

public:
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
                key.push_front(":");
                key.push_front(expected_params.arr[j]);
                key.push_front(":");
                key.push_front(expected_reasons.arr[i]);

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
            fs.push_front(" ");//space
            fs.push_front(str.data);//err
            fs.push_front("check_key failed, invalid key: ");//err
            throw OperationErrorType(fs.data);
        }
    }

    /**
     * for exception params separated by '| ' from end to begin marked by '|| 'call func callback
     * callback may throw
     */
    void for_params(FixedStringFunc func)
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
                        func(str);
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

    /**
     * dump data
     */
    const char* what() const throw()
    {
        return fs.data;
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
     * ctor of OperationException may fail and failure shall throw OperationError_ instance
     */

    OperationException(const char* file
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
        const char* what() const throw()
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
        typedef boost::function<void (FixedStringType str)> FixedStringFunc;
        typedef OperationError<DATASIZE> OperationErrorType;

        /**
         * get valid exception params from child
         */

        ConstArr get_fail_param() throw()
        {
            return static_cast<EXCEPTION_CHILD*>(this)->get_fail_param_impl();
        }

        /**
         * get valid exception reasons from child
         */

        ConstArr get_fail_reason() throw()
        {
            return static_cast<EXCEPTION_CHILD*>(this)->get_fail_reason_impl();
        }

        /**
         * dump exception data
         */

        const char* what() const throw()
        {
            return fs.data;
        }

        /**
         * check str against expected reasons and parameters
         * throw if invalid
         */

        void check_key(FixedStringType str)
        {
            bool key_is_valid = false;
            //printf("\ncheck_param: %s",str.data);
            ConstArr expected_reasons = get_fail_reason();
            ConstArr expected_params = get_fail_param();

            FixedStringType key;
            for(int i = 0; i < expected_reasons.size ; ++i)
            {
                for(int j = 0; j < expected_params.size; ++j)
                {
                    key = FixedStringType();//init
                    key.push_front(":");
                    key.push_front(expected_params.arr[j]);
                    key.push_front(":");
                    key.push_front(expected_reasons.arr[i]);

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
                fs.push_front(" ");//space
                fs.push_front(str.data);//err
                fs.push_front("check_key failed, invalid key: ");//err
                throw OperationErrorType(fs.data);
            }
        }

        /**
         * call supplied function with exception params optionally filtered by by substring search in reason and param
         */

        void callback_exception_params(boost::function<void (FixedStringType reason, FixedStringType param, FixedStringType value)> func_void3fixedstr
                , const char key_substring[] = "")
        {
            SearchCallback<OperationExceptionImplType> (key_substring,func_void3fixedstr,*this).run();//exec
        }

        /**
         * for exception params separated by '| ' from end to begin marked by '|| 'call func callback
         * callback may throw
         */
        void for_params(FixedStringFunc func)
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
                            func(str);
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
            FixedStringFunc check_data = std::bind1st(std::mem_fun(&OperationExceptionImplType::check_key), this);
            for_params(check_data);
        }


        ~OperationExceptionImpl() throw() {};

    };


}//namespace Fred
#endif // OPEXCEPTION_H_
