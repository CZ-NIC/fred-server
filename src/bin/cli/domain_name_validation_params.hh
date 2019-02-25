/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
 *  @domain_name_validation_params.h
 *  header of domain name validation administration implementation
 */

#ifndef DOMAIN_NAME_VALIDATION_PARAMS_HH_FD360EFB54ED482E973592741BD1FC8A
#define DOMAIN_NAME_VALIDATION_PARAMS_HH_FD360EFB54ED482E973592741BD1FC8A

#include "src/util/types/optional.hh"

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

/**
 * \class ZoneDomainNameValidationCheckersArgs
 * \brief set domain name validation checkers per zone
 */
struct ZoneDomainNameValidationCheckersArgs
{
    optional_string zone_name;
    std::vector<std::string> checker_names;

    ZoneDomainNameValidationCheckersArgs()
    {}

    ZoneDomainNameValidationCheckersArgs(
            const optional_string& _zone_name
            , const std::vector<std::string> &_checker_names
            )
    : zone_name(_zone_name)
    , checker_names(_checker_names)
    {}//init ctor
};//struct ZoneDomainNameValidationCheckersArgs

#endif
