/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @enumparam_params.h
 *  header of enumparam client implementation
 */

#ifndef ENUMPARAM_PARAMS_H_
#define ENUMPARAM_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class EnumParameterChangeArgs
 * \brief admin client enum_parameter_change
 */
struct EnumParameterChangeArgs
{
    std::string parameter_name;
    std::string parameter_value;

    EnumParameterChangeArgs()
    {}//ctor
    EnumParameterChangeArgs(
             const std::string& _parameter_name
             , const std::string& _parameter_value
            )
    : parameter_name(_parameter_name)
    , parameter_value(_parameter_value)
    {}//init ctor
};//struct EnumParameterChangeArgs

#endif // ENUMPARAM_PARAMS_H_
