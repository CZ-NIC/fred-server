/*
 * Copyright (C) 2010-2020  CZ.NIC, z. s. p. o.
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
 *  @file config_handler_decl.h
 *  common option handlers declaration
 */


#ifndef CONFIG_HANDLER_DECL_HH_572439E976FC479AB59B2826F8919ACB
#define CONFIG_HANDLER_DECL_HH_572439E976FC479AB59B2826F8919ACB
#include <cstdio>
#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <typeinfo>

#include <boost/utility.hpp>
#include <boost/format.hpp>

#include "src/util/cfg/faked_args.hh"
#include "src/util/cfg/handle_args.hh"

typedef std::map<std::string, HandleArgsPtr > HandlerPtrMap;

/**
 * \class CfgArgs
 * \brief handlers container and processor
 */
class CfgArgs : boost::noncopyable
{
    HandlerPtrVector hpv_; //defines processing order in handle() implementation
    HandlerPtrMap hpm_; //used for looking for handler by type
    static CfgArgs* instance_ptr;
public:
    template <class T> HandleArgsPtr get_handler_by_type()
    {
        HandlerPtrMap::const_iterator it;
        it = hpm_.find( typeid(T).name() );
        if(it != hpm_.end())//if found
            return it->second;
        //not found
        return HandleArgsPtr(static_cast<HandleArgs*>(0));
    }

    template <class T> T* get_handler_ptr_by_type()
    {
        HandlerPtrMap::const_iterator it;
        it = hpm_.find( typeid(T).name() );
        if(it != hpm_.end())//if found
            return dynamic_cast<T*>(it->second.get());
        //not found
        char errmsg[256]={'\0'};
        snprintf(errmsg, 255, "error: handler %s not found", typeid(T).name());
        throw std::runtime_error(errmsg);
    }
    friend std::unique_ptr<CfgArgs>::deleter_type;
protected:
    ~CfgArgs(){}
private:
    CfgArgs(const HandlerPtrVector& hpv)
        : hpv_(hpv) //vector init
    {
        //map init
        for(HandlerPtrVector::const_iterator i = hpv.begin()
                ; i != hpv.end(); ++i )
        {
            const HandleArgs *ha_ptr = i->get();
            hpm_[typeid(*ha_ptr).name()] = *i;
        }
    }

public:
    template <class HELP> static CfgArgs* init(const HandlerPtrVector& hpv);
    static CfgArgs* instance();

    FakedArgs fa;
    FakedArgs handle(int argc, char* argv[])
    {
        fa.init(argc, argv);

        for (HandlerPtrVector::const_iterator i = hpv_.begin(); i != hpv_.end(); ++i)
        {
            FakedArgs fa_out;
            (*i)->handle(fa.get_argc(), fa.get_argv(), fa_out);
            fa = fa_out;//last output to next input
        }
        return fa;
    }
};


//setter
template <class HELP> CfgArgs* CfgArgs::init(const HandlerPtrVector& hpv)
{
    std::unique_ptr<CfgArgs>
    tmp_instance(new CfgArgs(hpv));

    //gather options_descriptions for help print if present
    HandleArgsPtr ha =
            tmp_instance->get_handler_by_type<HELP>();
    HELP* hga = 0;//nonowning temp child
    if(ha.get() != 0)
        hga = dynamic_cast<HELP*>(ha.get());
    if(hga != 0)
    {
        for(HandlerPtrVector::const_iterator i = hpv.begin()
                ; i != hpv.end(); ++i )
            hga->po_description.push_back((*i)->get_options_description());
    }
    instance_ptr = tmp_instance.release();
    return instance_ptr;
}

typedef std::map<std::string, HandleGrpArgsPtr > HandlerGrpPtrMap;

/**
 * \class CfgArgGroups
 * \brief groupable handlers container and processor
 */
class CfgArgGroups : boost::noncopyable
{
    HandlerPtrGrid hpg_;
    HandlerGrpPtrMap hpm_;
    static CfgArgGroups* instance_ptr;
public:
    template <class T> HandleGrpArgsPtr get_handler_by_type()
    {
        HandlerGrpPtrMap::const_iterator it;
        it = hpm_.find( typeid(T).name() );
        if(it != hpm_.end())//if found
            return it->second;
        //not found
        return HandleGrpArgsPtr(static_cast<HandleGrpArgs*>(0));
    }

    template <class T> T* get_handler_ptr_by_type()
    {
        HandlerGrpPtrMap::const_iterator it;
        it = hpm_.find( typeid(T).name() );
        if(it != hpm_.end())//if found
            return dynamic_cast<T*>(it->second.get());
        //not found
        char errmsg[256]={'\0'};
        snprintf(errmsg, 255, "error: handler %s not found", typeid(T).name());
        throw std::runtime_error(errmsg);
    }
    friend std::unique_ptr<CfgArgGroups>::deleter_type;
protected:
    ~CfgArgGroups(){}
private:
    CfgArgGroups(const HandlerPtrGrid& hpg)
        : hpg_(hpg) //grid init
    {
        //map init
        for(HandlerPtrGrid::const_iterator i = hpg_.begin()
                ; i != hpg_.end(); ++i )
            for(HandlerGrpVector::const_iterator j = i->begin()
                    ; j != i->end(); ++j )
            {
                //overwrites instance of the same type used more than once
                const HandleGrpArgs *hga_ptr = j->get();
                hpm_[typeid(*hga_ptr).name()] = *j;
            }
    }

public:
    template <class HELP> static  CfgArgGroups * init(const HandlerPtrGrid& hpg);
    static CfgArgGroups * instance();

    FakedArgs fa;
    FakedArgs handle( int argc, char* argv[])
    {
        //initial fa
        fa.init(argc, argv);

        std::size_t group_index = 0;//start index

        for(HandlerPtrGrid::const_iterator i = hpg_.begin()
                ; i != hpg_.end(); ++i )
        {
            FakedArgs fa_out;
            group_index = i->at(group_index)//option group selection
                    ->handle( fa.get_argc(), fa.get_argv(), fa_out, group_index);
            fa=fa_out;//last output to next input
        }//for HandlerPtrGrid
        return fa;
    }//handle
};//class CfgArgGroups

//setter
template <class HELP> CfgArgGroups* CfgArgGroups::init(const HandlerPtrGrid& hpg)
{
    std::unique_ptr<CfgArgGroups>
    tmp_instance(new CfgArgGroups(hpg));

    //gather options_descriptions for help print if present
    HandleGrpArgsPtr ha =
            tmp_instance->get_handler_by_type<HELP>();
    HELP* hga = 0;//nonowning temp child
    if(ha.get() != 0)
        hga = dynamic_cast<HELP*>(ha.get());
    if(hga != 0)
    {
        for(HandlerPtrGrid::const_iterator i = hpg.begin()
                ; i != hpg.end(); ++i )
            for(HandlerGrpVector::const_iterator j = i->begin()
                    ; j != i->end(); ++j )
            {
                try
                {
                    std::shared_ptr<
                        boost::program_options::options_description> options
                        = (*j)->get_options_description();//check if there are some options
                    hga->get_po_description().push_back(options);
                }//try
                catch(const NO_OPTIONS&)
                {}//ignore when no options decription suplied
            }
    }
    instance_ptr = tmp_instance.release();
    return instance_ptr;
}

#endif
