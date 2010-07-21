/*  
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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
 *  @file config_handler.h
 *  common option handlers
 */


#ifndef CONFIG_HANDLER_H_
#define CONFIG_HANDLER_H_

#include <iostream>
#include <exception>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <typeinfo>

#include <boost/utility.hpp>
#include <boost/format.hpp>

#include "faked_args.h"
#include "handle_args.h"
//#include "handle_general_args.h"

typedef std::map<std::string, HandleArgsPtr > HandlerPtrMap;


//compose args processing
/* possible usage:
HandlerPtrVector ghpv =
boost::assign::list_of
(HandleArgsPtr(new HandleGeneralArgs))
(HandleArgsPtr(new HandleDatabaseArgs))
(HandleArgsPtr(new HandleThreadGroupArgs))
(HandleArgsPtr(new HandleCorbaNameServiceArgs));

in UTF main
 fa = CfgArgs::instance<HandleGeneralArgs>(ghpv)->handle(argc, argv);
*/

/**
 * \class CfgArgs
 * \brief handlers container and processor
 */
class CfgArgs : boost::noncopyable
{
    HandlerPtrVector hpv_;
    HandlerPtrMap hpm_;
    static std::auto_ptr<CfgArgs> instance_ptr;
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
        throw std::runtime_error("error: handler not found");
    }
    friend class std::auto_ptr<CfgArgs>;
protected:
    ~CfgArgs(){}
private:
    CfgArgs(const HandlerPtrVector& hpv)
        : hpv_(hpv) //vector init
    {
        //map init
        for(HandlerPtrVector::const_iterator i = hpv.begin()
                ; i != hpv.end(); ++i )
            hpm_[typeid( *((*i).get()) ).name()] = *i;
    }

public:
    template <class HELP> static  CfgArgs * instance(const HandlerPtrVector& hpv);
    static CfgArgs * instance();

    FakedArgs fa;
    FakedArgs handle( int argc, char* argv[])
    {
        //initial fa
        fa.prealocate_for_argc(argc);
        for (int i = 0; i < argc ; ++i)
            fa.add_argv(argv[i]);

        for(HandlerPtrVector::const_iterator i = hpv_.begin()
                ; i != hpv_.end(); ++i )
        {
            FakedArgs fa_out;
            (*i)->handle( fa.get_argc(), fa.get_argv(), fa_out);
            fa=fa_out;//last output to next input
        }//for HandlerPtrVector
        return fa;
    }//handle
};//class CfgArgs

//static instance init
std::auto_ptr<CfgArgs> CfgArgs::instance_ptr(0);



//setter
template <class HELP> CfgArgs* CfgArgs::instance(const HandlerPtrVector& hpv)
{
    std::auto_ptr<CfgArgs>
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
    instance_ptr = tmp_instance;
    return instance_ptr.get();
}

//getter
CfgArgs* CfgArgs::instance()
{
    CfgArgs* ret = instance_ptr.get();
    if (ret == 0) throw std::runtime_error("error: CfgArgs instance not set");
    return ret;
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
    static std::auto_ptr<CfgArgGroups> instance_ptr;
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
        throw std::runtime_error("error: handler not found");
    }
    friend class std::auto_ptr<CfgArgGroups>;
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
                hpm_[typeid( *((*j).get()) ).name()] = *j;
    }

public:
    template <class HELP> static  CfgArgGroups * instance(const HandlerPtrGrid& hpg);
    static CfgArgGroups * instance();

    FakedArgs fa;
    FakedArgs handle( int argc, char* argv[])
    {
        //initial fa
        fa.prealocate_for_argc(argc);
        for (int i = 0; i < argc ; ++i)
            fa.add_argv(argv[i]);

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

//static instance init
std::auto_ptr<CfgArgGroups> CfgArgGroups::instance_ptr(0);



//setter
template <class HELP> CfgArgGroups* CfgArgGroups::instance(const HandlerPtrGrid& hpg)
{
    std::auto_ptr<CfgArgGroups>
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
                hga->po_description.push_back((*j)->get_options_description());
    }
    instance_ptr = tmp_instance;
    return instance_ptr.get();
}

//getter
CfgArgGroups* CfgArgGroups::instance()
{
    CfgArgGroups* ret = instance_ptr.get();
    if (ret == 0) throw std::runtime_error("error: CfgArgGroups instance not set");
    return ret;
}



#endif //CONFIG_HANDLER_H_
