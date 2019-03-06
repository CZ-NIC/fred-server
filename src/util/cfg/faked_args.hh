/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file faked_args.h
 *  manipulation with cmdline arguments
 */


#ifndef FAKED_ARGS_HH_734E8D0126D94E4791B090541FF354C2
#define FAKED_ARGS_HH_734E8D0126D94E4791B090541FF354C2

#include <iostream>
#include <exception>
#include <string>
#include <vector>

/**
 * \class FakedArgs
 * \brief for manipulation with cmdline arguments
 */

class FakedArgs //faked args
{
    typedef std::vector<char> char_vector_t;//type for argv buffer
    typedef std::vector<char_vector_t> argv_buffers_t;//buffers vector type
    typedef std::vector<char*> argv_t;//pointers vector type

    argv_buffers_t argv_buffers;//owning vector of buffers
    argv_t argv;//new argv - nonowning
public:
    void copy(const FakedArgs& fa)
    {
        argv_buffers=fa.argv_buffers;

        argv.reserve( fa.argv_buffers.size() );
        argv.clear();
        for(argv_buffers_t::iterator i = argv_buffers.begin()
                ; i!=argv_buffers.end();++i)
        {
            argv.push_back( &(i->at(0)) );
        }
    }

    FakedArgs(){}

    FakedArgs(const FakedArgs& fa)
    {
        copy(fa);
    }

    FakedArgs& operator=(const FakedArgs& fa)
    {
        if (this != &fa) copy(fa);
        return *this;
    }

    void clear()
    {
        argv_buffers.clear();
        argv.clear();
    }
    //optional memory prealocation for expected argc
    //actual argc is not affected
    void prealocate_for_argc(int expected_argc)
    {
        //vector prealocation
        argv_buffers.reserve(expected_argc);
        argv.reserve(expected_argc);
    }//prealocate_for_argc

    int get_argc() const //argc getter
    {
        return static_cast<int>(argv_buffers.size());
    }

    char** get_argv() //argv getter
    {
        return &(argv.at(0));
    }

    void add_argv(char* asciiz)//add zero terminated C-style string of chars
    {
        add_argv(std::string(asciiz));
    }

    void add_argv(std::string str)//add std::string
    {
        //std::cout << "add_argv str : " << str <<  std::endl;
        argv_buffers.push_back(FakedArgs::char_vector_t());//added buffer
        std::size_t strsize = str.length();
        //argv size
        std::size_t argv_size = argv_buffers.size();
        std::size_t argv_idx = argv_size - 1;
        //preallocation of buffer for first ending with 0
        argv_buffers.at(argv_idx).reserve(strsize+1);

        //actual string copy
        for(std::string::const_iterator si = str.begin()
                ; si != str.end();  ++si )
        {
            argv_buffers.at(argv_idx).push_back(*si);
        }//for si
        argv_buffers.at(argv_idx).push_back(0);//zero terminated string

        //refresh argv
        argv.clear();
        for(argv_buffers_t::iterator i = argv_buffers.begin()
                ; i!=argv_buffers.end();++i)
        {
            argv.push_back( &(i->at(0)) );
        }
        //std::cout << "add_argv str : " << str <<  std::endl;
    }

    FakedArgs copy_onlynospaces_args()
    {
        FakedArgs ret;

        for (int i = 0; i < get_argc(); ++i)
        {
            std::string arg = argv.at(i);
            if((arg.find(' ') == std::string::npos)
                && (arg.find('\t') == std::string::npos))
            {
                ret.add_argv(arg);
            }
        }

        return ret;
    }

    void init(int argc, char* argv[])
    {
        clear();
        //initial fa
        prealocate_for_argc(argc);
        for (int i = 0; i < argc ; ++i)
            add_argv(argv[i]);
    }

    FakedArgs(int argc, char* argv[])
    {
        init(argc, argv);
    }

    std::string print_into_string()
    {
        std::string ret;
        for (argv_t::iterator it = argv.begin(); it != argv.end(); ++it)
        {
            ret += *it + std::string(" ");
        }

        return ret;
    }

};//class FakedArgs


#endif
