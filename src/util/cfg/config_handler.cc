/*
 * Copyright (C) 2020  CZ.NIC, z. s. p. o.
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

#include "src/util/cfg/config_handler_decl.hh"

CfgArgs* CfgArgs::instance_ptr = nullptr;

CfgArgGroups* CfgArgGroups::instance_ptr = nullptr;

CfgArgs* CfgArgs::instance()
{
    CfgArgs* ret = instance_ptr;
    if (ret == 0) throw std::runtime_error("error: CfgArgs instance not set");
    return ret;
}

CfgArgGroups* CfgArgGroups::instance()
{
    CfgArgGroups* ret = instance_ptr;
    if (ret == 0) throw std::runtime_error("error: CfgArgGroups instance not set");
    return ret;
}
