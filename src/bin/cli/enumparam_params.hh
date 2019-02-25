/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
 *  @enumparam_params.h
 *  header of enumparam client implementation
 */

#ifndef ENUMPARAM_PARAMS_HH_6BD58E2DC17649F5B45443F9341035EA
#define ENUMPARAM_PARAMS_HH_6BD58E2DC17649F5B45443F9341035EA

#include "src/util/types/optional.hh"

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

#endif
