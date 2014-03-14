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
 *  registrar info data diff
 */

#ifndef INFO_REGISTRAR_DIFF_H_
#define INFO_REGISTRAR_DIFF_H_

#include <string>

#include "info_registrar_data.h"

#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/printable.h"


namespace Fred
{
    /**
     * Diff of registrar data.
     * Data of the registrar difference with the same members as registrar data but in optional pairs. Optional pair member is set in case of difference in compared registrar data.
     */
    struct InfoRegistrarDiff : public Util::Printable
    {
        template <class T> struct DiffMemeber { typedef Optional<std::pair<T,T> > Type;};

        DiffMemeber<std::string>::Type handle;/**< registrar handle */
        DiffMemeber<Nullable<std::string> >::Type name;/**< name of the registrar */
        DiffMemeber<Nullable<std::string> >::Type organization;/**< full trade name of organization */
        DiffMemeber<Nullable<std::string> >::Type street1;/**< part of address */
        DiffMemeber<Nullable<std::string> >::Type street2;/**< part of address */
        DiffMemeber<Nullable<std::string> >::Type street3;/**< part of address*/
        DiffMemeber<Nullable<std::string> >::Type city;/**< part of address - city */
        DiffMemeber<Nullable<std::string> >::Type stateorprovince;/**< part of address - region */
        DiffMemeber<Nullable<std::string> >::Type postalcode;/**< part of address - postal code */
        DiffMemeber<Nullable<std::string> >::Type country;/**< two character country code or country name */
        DiffMemeber<Nullable<std::string> >::Type telephone;/**<  telephone number */
        DiffMemeber<Nullable<std::string> >::Type fax;/**< fax number */
        DiffMemeber<Nullable<std::string> >::Type email;/**< e-mail address */
        DiffMemeber<Nullable<std::string> >::Type url;/**< web page of the registrar */
        DiffMemeber<Nullable<bool> >::Type system;/**< system registrar flag */
        DiffMemeber<Nullable<std::string> >::Type ico;/**< company registration number */
        DiffMemeber<Nullable<std::string> >::Type dic;/**< taxpayer identification number */
        DiffMemeber<Nullable<std::string> >::Type variable_symbol;/**< registrar payments coupling tag, have to match with payment variable symbol to couple payment with registrar*/
        DiffMemeber<Nullable<std::string> >::Type payment_memo_regex;/**< registrar payments coupling alternative to variable symbol, if payment_memo_regex is set, payment_memo have to match case insesitive with payment_memo_regex to couple payment with registrar*/
        DiffMemeber<Nullable<bool> >::Type vat_payer;/**< VAT payer flag */
        DiffMemeber<unsigned long long >::Type id; /**< registrar db id */

        /**
        * Constructor of the registrar data diff structure.
        */
        InfoRegistrarDiff();

        /**
        * Dumps state of the instance into the string
        * @return string with description of the instance state
        */
        std::string to_string() const;

        /**
        * Check if some data is set into the instance
        * @return false if instance contains differing data and true if not
        */
        bool is_empty() const;
    };

    /**
     * Diff data of the registrar.
     * @param first
     * @param second
     * @return diff of given registrar
     */
    InfoRegistrarDiff diff_registrar_data(const InfoRegistrarData& first, const InfoRegistrarData& second);

}//namespace Fred

#endif//INFO_REGISTRAR_DIFF_H_
