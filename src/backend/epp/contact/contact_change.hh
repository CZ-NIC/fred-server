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

#ifndef CONTACT_CHANGE_HH_2C8C709868134389AD38F302E513C812
#define CONTACT_CHANGE_HH_2C8C709868134389AD38F302E513C812

#include "src/backend/epp/contact/contact_disclose.hh"
#include "src/backend/epp/contact/contact_ident.hh"
#include "src/backend/epp/contact/util.hh"
#include "src/util/db/nullable.hh"

#include <boost/optional.hpp>

#include <string>
#include <vector>

namespace Epp {
namespace Contact {

struct ContactChange
{
    struct Address
    {
        boost::optional<std::string> street1;
        boost::optional<std::string> street2;
        boost::optional<std::string> street3;
        boost::optional<std::string> city;
        boost::optional<std::string> state_or_province;
        boost::optional<std::string> postal_code;
        boost::optional<std::string> country_code;
    };
    boost::optional< Nullable<std::string> > name;
    boost::optional< Nullable<std::string> > organization;
    boost::optional<std::vector<std::string>> streets;
    boost::optional< Nullable<std::string> > city;
    boost::optional< Nullable<std::string> > state_or_province;
    boost::optional< Nullable<std::string> > postal_code;
    boost::optional<std::string> country_code;
    boost::optional< Nullable<Address> > mailing_address;
    boost::optional< Nullable<std::string> > telephone;
    boost::optional< Nullable<std::string> > fax;
    boost::optional< Nullable<std::string> > email;
    boost::optional< Nullable<std::string> > notify_email;
    boost::optional< Nullable<std::string> > vat;
    boost::optional< boost::optional<ContactIdent> > ident;//Nullable<ContactIdent>() requires ContactIdent() which is not available
    boost::optional< Nullable<std::string> > authinfopw;
    boost::optional<ContactDisclose> disclose;
    struct Value
    {
        enum Meaning
        {
            to_set,
            to_delete,
            not_to_touch
        };
    };
    template <Value::Meaning meaning, class T>
    static bool does_value_mean(const boost::optional< Nullable<T> >& _value);
    template <Value::Meaning meaning, class T>
    static bool does_value_mean(const boost::optional< boost::optional<T> >& _value);
    template <Value::Meaning meaning, class T>
    static bool does_value_mean(const boost::optional<T>& _value);
    template <class T>
    static T get_value(const boost::optional< Nullable<T> >& _value);
    template <class T>
    static T get_value(const boost::optional< boost::optional<T> >& _value);
    template <class T>
    static T get_value(const boost::optional<T>& _value);
};

ContactChange trim(const ContactChange& src);

} // namespace Epp::Contact
} // namespace Epp

#endif
