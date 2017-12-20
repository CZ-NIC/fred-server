/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  @file
 *  registrar info data
 */

#ifndef INFO_REGISTRAR_DATA_H_
#define INFO_REGISTRAR_DATA_H_

#include <string>

#include "src/util/db/nullable.hh"
#include "src/util/printable.hh"

namespace LibFred
{
    /**
     * Common data of registrar.
     * Current state of the registrar.
     */
    struct InfoRegistrarData : public Util::Printable
    {
        std::string handle;/**< registrar handle */
        Nullable<std::string> name;/**< name of the registrar */
        Nullable<std::string> organization;/**< full trade name of organization */
        Nullable<std::string> street1;/**< part of address */
        Nullable<std::string> street2;/**< part of address */
        Nullable<std::string> street3;/**< part of address*/
        Nullable<std::string> city;/**< part of address - city */
        Nullable<std::string> stateorprovince;/**< part of address - region */
        Nullable<std::string> postalcode;/**< part of address - postal code */
        Nullable<std::string> country;/**< two character country code or country name */
        Nullable<std::string> telephone;/**<  telephone number */
        Nullable<std::string> fax;/**< fax number */
        Nullable<std::string> email;/**< e-mail address */
        Nullable<std::string> url;/**< web page of the registrar */
        Nullable<bool> system;/**< system registrar flag */
        Nullable<std::string> ico;/**< company registration number */
        Nullable<std::string> dic;/**< taxpayer identification number */
        Nullable<std::string> variable_symbol;/**< registrar payments coupling tag, have to match with payment variable symbol to couple payment with registrar*/
        Nullable<std::string> payment_memo_regex;/**< registrar payments coupling alternative to variable symbol, if payment_memo_regex is set, payment_memo have to match case insesitive with payment_memo_regex to couple payment with registrar*/
        bool                  vat_payer;/**< VAT payer flag */
        unsigned long long id; /**< registrar db id */

    public:
        /**
        * Constructor of the registrar data structure.
        */
        InfoRegistrarData();
        /**
        * Equality of the registrar data structure operator.
        * @param rhs is right hand side of the registrar data comparison
        * @return true if equal, false if not
        */
        bool operator==(const InfoRegistrarData& rhs) const;

        /**
        * Inequality of the registrar data structure operator.
        * @param rhs is right hand side of the registrar data comparison
        * @return true if not equal, false if equal
        */
        bool operator!=(const InfoRegistrarData& rhs) const;

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;
    };

} // namespace LibFred

#endif//INFO_REGISTRAR_DATA_H_
