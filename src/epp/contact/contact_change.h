/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef CONTACT_CHANGE_H_EEBE9D36AFCD48B8BFA4A97DD6F38EA1
#define CONTACT_CHANGE_H_EEBE9D36AFCD48B8BFA4A97DD6F38EA1

#include "src/epp/contact/contact_disclose.h"
#include "src/epp/contact/util.h"
#include "util/db/nullable.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

struct ContactChange
{
    boost::optional<Nullable<std::string> > name;
    boost::optional<Nullable<std::string> > organization;
    std::vector<boost::optional<Nullable<std::string> > > streets;
    boost::optional<Nullable<std::string> > city;
    boost::optional<Nullable<std::string> > state_or_province;
    boost::optional<Nullable<std::string> > postal_code;
    boost::optional<std::string> country_code;
    boost::optional<Nullable<std::string> > telephone;
    boost::optional<Nullable<std::string> > fax;
    boost::optional<Nullable<std::string> > email;
    boost::optional<Nullable<std::string> > notify_email;
    boost::optional<Nullable<std::string> > vat;
    boost::optional<Nullable<std::string> > ident;
    struct IdentType
    {
        enum Enum
        {
            op,
            pass,
            ico,
            mpsv,
            birthday

        };

    };

    Nullable<IdentType::Enum> ident_type;
    boost::optional<Nullable<std::string> > authinfopw;
    boost::optional<ContactDisclose> disclose;
    struct Value
    {
        enum Meaning
        {
            to_set,
            to_delete,
            not_to_touch,

        };

    };

    template <Value::Meaning MEANING, class T>
    static bool does_value_mean(const T& _value);


    template <class T>
    static T get_value(const boost::optional<Nullable<T> >& _value);


    template <class T>
    static T get_value(const boost::optional<T>& _value);


};

ContactChange trim(const ContactChange& contact_change_);


} // namespace Epp::Contact
} // namespace Epp

#endif
