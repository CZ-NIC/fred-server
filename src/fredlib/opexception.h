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
        :arr(list)
        , size(_size)
    {}
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

///operation exception error template, able of copying
template <
    int DATASIZE ///size of internal buffer for detail of failure
    >
class OperationExceptionError
: virtual public OperationExceptionBase
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

    ///ctor
    OperationExceptionError(const char* data) throw()
        : databuffer_()
    {
        //fill databuffer
        copy_end(databuffer_, data, sizeof(databuffer_));//databuffer
        copy_end(databuffer_, "OperationExceptionError: ", sizeof(databuffer_));//space
    }
};

///operation exception template, able of copying
template <
    int DATASIZE ///size of internal buffer for detail of failure
    , class EXCEPTION_BASE ///additional exception types / interfaces
    , class FAIL_PARAM_ARRAY ///array of parameters names which may fail
    , class FAIL_REASON_ARRAY///array of reasons why may parameter fail
    >
class OperationException
: virtual public OperationExceptionBase
, virtual public EXCEPTION_BASE
, private FAIL_PARAM_ARRAY
, private FAIL_REASON_ARRAY
, private CopyEndImpl
{
    /**
     * data shall be stored from end to begin
     * data shall look like this:
     * optional text with some details followed by fail reasons, params and values of the operation
     * || reason1:param1: val1 | reason2:param2: val2 ... | reason#:param#: val# |
     * separating character '|' in data (reason, param or value) shell be replaced by description like <pipe>
     */
    char databuffer_[DATASIZE+1];///+1 for ending by \0


public:
    typedef OperationException<DATASIZE, EXCEPTION_BASE, FAIL_PARAM_ARRAY, FAIL_REASON_ARRAY> OperationExceptionType;
    typedef FixedString<DATASIZE> FixedStringType;
    typedef boost::function<void (FixedStringType str)> FixedStringFunc;
    typedef OperationExceptionError<DATASIZE> OperationExceptionErrorType;

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
        if(!key_is_valid)
        {
            copy_end(databuffer_, " ", sizeof(databuffer_));//space
            copy_end(databuffer_, key.data, sizeof(databuffer_));//err
            copy_end(databuffer_, "check_key failed, invalid key: ", sizeof(databuffer_));//err
            throw OperationExceptionErrorType(databuffer_);
        }
    }


    /**
     * look for value of key separated by '|'
     * return: value
     */
    FixedStringType look_for(const char* key) throw()
    {
        FixedStringType ret;
        int len_of_key = strlen(key);
        int len_of_data = strlen(databuffer_);
        int last_separator_index = len_of_data;
        for(int i = len_of_data - 1; i > -1; --i)//search data backward
        {
            //printf("\nfor i: %d\n", i);
            if(databuffer_[i] == '|')//if separator found
            {
                //printf("\nlook_for found | at: %d\n", i);
                int chars_to_search = len_of_data - (i + 1);
                if (chars_to_search >= len_of_key)
                {
                    //printf("\nlook_for key: %s at: %d: %s\n", key,i, databuffer_+i+2);
                    if(strncmp(databuffer_+i+2, key,len_of_key) == 0)
                    {//found key at i+2
                        char* val_ptr = databuffer_+i+2+len_of_key+1;
                        int val_len = (databuffer_+last_separator_index) - val_ptr - 1;
                        //printf("\nlook_for found key: %s val_len: %d at: %d\n", key,val_len, i+1);
                        memmove(ret.data,val_ptr,val_len);//ok if ret have DATASIZE same as databuffer_ or bigger
                        return ret;
                    }//if key found
                }//if search for key
                //printf("\nlook_for last_separator_index: %d\n", last_separator_index);
                last_separator_index = i;
                //check border ||
                if ((i > 0) && (databuffer_[i-1] == '|'))
                {
                    //printf("\nborder found | at: %d\n", i);
                    return ret;//key not found
                }
            }//if |
        }//for i
        return ret;//key not found
    }//look_for

    int get_value_count() throw()
    {
        int value_count = 0;
        int len_of_data = strlen(databuffer_);
        for(int i = len_of_data - 1; i > -1; --i)//search data backward
        {
            //printf("\nfor i: %d\n", i);
            if(databuffer_[i] == '|')//if separator found
            {
                if(i < len_of_data -1)
                {
                    ++value_count;
                }
                //check border ||
                if ((i > 0) && (databuffer_[i-1] == '|'))
                {
                    //printf("\nborder found | at: %d\n", i);
                    break;
                }
            }//if |
        }//for i

        //printf("\nvalue_count: %d\n", value_count);
        return value_count - 1;
    }//get_value_count


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

    ///ctor
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
};

///default exception data size
//#define OPEX(DATA) OperationException<2048>(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
///custom exception data size
//#define OPEXS(SIZE,DATA) OperationException<(SIZE)>(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

#endif // OPEXCEPTION_H_
