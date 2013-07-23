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
 *  @domain_name_validation_params.h
 *  header of domain name validation administration implementation
 */

#ifndef DOMAIN_NAME_VALIDATION_PARAMS_H_
#define DOMAIN_NAME_VALIDATION_PARAMS_H_

#include "util/types/optional.h"

/**
 * \class DomainNameValidationCheckersInitArgs
 * \brief init of available domain name validation checkers
 */
struct DomainNameValidationCheckersInitArgs
{
    optional_string checker_name_prefix;
    std::vector<std::string> checker_names;
    bool serch_checkers_by_prefix;

    DomainNameValidationCheckersInitArgs()
    : serch_checkers_by_prefix(false)
    {}

    DomainNameValidationCheckersInitArgs(
            const optional_string& _checker_name_prefix
            , const std::vector<std::string> &_checker_names
            )
    : checker_name_prefix(_checker_name_prefix)
    , checker_names(_checker_names)
    {}//init ctor
};//struct DomainNameValidationCheckersInitArgs

#endif // DOMAIN_NAME_VALIDATION_PARAMS_H_
