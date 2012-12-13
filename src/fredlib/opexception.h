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


///operation exception base class
struct OperationExceptionBase
: virtual public std::exception  //common base
{
    virtual const char* what() const throw() = 0;
    virtual ~OperationExceptionBase() throw() {};
};

///operation exception template, able of copying
template <int DATASIZE=512> class OperationException
: virtual public OperationExceptionBase
{
    /**
     * data shall be stored from end to begin
     * data shall look like this:
     * optional text with some details followed by failed params of the operation \
     *  | operation_name: 'name' | param1: 'name' | val1: 'data' | reason1: 'text' ... | param#: 'name' | val#: 'data' | reason#: 'text' |
     */
    char databuffer_[DATASIZE+1];///+1 for ending by \0

    /**
     * copy end of the asciiz string first from "from" to "to" with regard to "size_of_to"
     */
    void copy_end(char * to, const char * from , int size_of_to ) throw()
    {
        int len_of_to = strlen(to);
        int len_of_from = strlen(from) + 1;//plus space
        int space_left_in_to = size_of_to - len_of_to;

        if (space_left_in_to >= len_of_from)
        {//all data fits in
            memmove(to+len_of_from, to, len_of_to);//make space
            memmove(to, from, len_of_from);//copy data
            to[len_of_from - 1] = ' ';//separate with space
            to[len_of_from+len_of_to] = '\0';//end by zero
        }
        else
        {//store only end of the data
            memmove(to + space_left_in_to, to, len_of_to);//make space
            memmove(to, from+ (len_of_from - space_left_in_to), space_left_in_to);//copy data
            to[space_left_in_to - 1] = ' ';//separate with space
            to[space_left_in_to+len_of_to] = '\0';//end by zero
        }
    }

    /**
     * look for value of key separated by '|'
     * output: start of value and value length
     */
    void look_for(const char* key, char *& val_ptr, int val_len)
    {
        int len_of_key = strlen(key);
        int len_of_data = strlen(databuffer_);
        int last_separator_index = len_of_data;
        for(int i = len_of_data - 1; i > -1; --i)//search data backward
        {
            if(databuffer_[i] == '|')//if separator found
            {
                int chars_to_search = len_of_data - (i + 1);
                if (chars_to_search >= len_of_key)
                {
                    if(strncmp(databuffer_+i+1, key,len_of_key) == 0)
                    {//found key at i+1
                        val_ptr = databuffer_+i+1+len_of_key+1;
                        val_len = val_ptr - last_separator_index - 1;
                        return;
                    }//if key found
                }//if search for key
                last_separator_index = i;
            }//if |

        }//for i
    }//look_for

public:

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
            , const char* data) throw()
    : databuffer_()
    {
        copy_end(databuffer_, data, sizeof(databuffer_));//databuffer
        copy_end(databuffer_, function, sizeof(databuffer_));//function
        char line_str[20]={'\0'};
        snprintf(line_str,sizeof(line_str), ": %d", line);
        copy_end(databuffer_, line_str, sizeof(databuffer_));//line
        copy_end(databuffer_, file, sizeof(databuffer_));//file
    }
};

///default exception data size
#define OPEX(DATA) OperationException(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))
///custom exception data size
#define OPEXS(SIZE,DATA) OperationException<(SIZE)>(__FILE__, __LINE__, __ASSERT_FUNCTION, (DATA))

#endif // OPEXCEPTION_H_
