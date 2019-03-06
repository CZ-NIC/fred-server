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
 *  @optional_from_program_options.h
 *  get optional types from boost program options
 */

#ifndef OPTIONAL_FROM_PROGRAM_OPTIONS_HH_5A1CCF0415324298906D4133BF417003
#define OPTIONAL_FROM_PROGRAM_OPTIONS_HH_5A1CCF0415324298906D4133BF417003

#include <boost/program_options.hpp>
#include "src/util/types/optional.hh"

//OPTIONAL_TYPE should be OptionalType instance or compatible

template <class OPTIONAL_TYPE>
void get_optional_from_vm(boost::program_options::variables_map& vm
        , const std::string& key, OPTIONAL_TYPE& opt)
{
    typedef typename OPTIONAL_TYPE::TypeOfValue TYPE;

    if(vm.count(key.c_str()))
        opt = OptionalType<TYPE>(vm[key.c_str()].as<TYPE>());
    else
        opt = OptionalType<TYPE>();
}

template <class OPTIONAL_TYPE>
OPTIONAL_TYPE get_optional_from_vm(boost::program_options::variables_map& vm, const std::string& key)
{
    OPTIONAL_TYPE opt;
    get_optional_from_vm(vm, key, opt);
    return opt;
}

#endif //OPTIONAL_FROM_PROGRAM_OPTIONS_H_

