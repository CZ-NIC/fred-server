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
 *  common option handlers definition
 */


#ifndef CONFIG_HANDLER_H_
#define CONFIG_HANDLER_H_

#include "config_handler_decl.h"

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


//static instance init
std::auto_ptr<CfgArgs> CfgArgs::instance_ptr(0);

//static instance init
std::auto_ptr<CfgArgGroups> CfgArgGroups::instance_ptr(0);

//getter
CfgArgs* CfgArgs::instance()
{
    CfgArgs* ret = instance_ptr.get();
    if (ret == 0) throw std::runtime_error("error: CfgArgs instance not set");
    return ret;
}


//getter
CfgArgGroups* CfgArgGroups::instance()
{
    CfgArgGroups* ret = instance_ptr.get();
    if (ret == 0) throw std::runtime_error("error: CfgArgGroups instance not set");
    return ret;
}



#endif //CONFIG_HANDLER_H_
