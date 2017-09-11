/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/epp/contact/contact_change.h"

#include "src/epp/contact/util.h"

namespace Epp {
namespace Contact {

namespace {

template <class T>
struct ClassifyValue { };

template <class T>
struct ClassifyValue< boost::optional< Nullable<T> > >
{
    static ContactChange::Value::Meaning get_meaning_of_value(const boost::optional< Nullable<T> > &_value)
    {
        if (_value != boost::none)
        {
            if (_value->isnull())
            {
                return ContactChange::Value::to_delete;
            }
            return ContactChange::Value::to_set;
        }
        return ContactChange::Value::not_to_touch;
    }
};

template <class T>
struct ClassifyValue< boost::optional< boost::optional<T> > >
{
    static ContactChange::Value::Meaning get_meaning_of_value(const boost::optional< boost::optional<T> > &_value)
    {
        if (_value != boost::none)
        {
            if (*_value == boost::none)
            {
                return ContactChange::Value::to_delete;
            }
            return ContactChange::Value::to_set;
        }
        return ContactChange::Value::not_to_touch;
    }
};

template < >
struct ClassifyValue< boost::optional<std::string> >
{
    static ContactChange::Value::Meaning get_meaning_of_value(const boost::optional<std::string> &_value)
    {
        if (_value != boost::none)
        {
            return ContactChange::Value::to_set;
        }
        return ContactChange::Value::not_to_touch;
    }
};

}//namespace Epp::{anonymous}

template <ContactChange::Value::Meaning meaning, class T>
bool ContactChange::does_value_mean(const boost::optional< Nullable<T> >& _value)
{
    return ClassifyValue< boost::optional< Nullable<T> > >::get_meaning_of_value(_value) == meaning;
}

template <ContactChange::Value::Meaning meaning, class T>
bool ContactChange::does_value_mean(const boost::optional< boost::optional<T> >& _value)
{
    return ClassifyValue< boost::optional< boost::optional<T> > >::get_meaning_of_value(_value) == meaning;
}

template <ContactChange::Value::Meaning meaning, class T>
bool ContactChange::does_value_mean(const boost::optional<T>& _value)
{
    return ClassifyValue< boost::optional<T> >::get_meaning_of_value(_value) == meaning;
}

template <class T>
T ContactChange::get_value(const boost::optional< Nullable<T> >& _value)
{
    return _value->get_value();
}

template <class T>
T ContactChange::get_value(const boost::optional< boost::optional<T> >& _value)
{
    return **_value;
}

template <class T>
T ContactChange::get_value(const boost::optional<T>& _value)
{
    return *_value;
}


template
bool ContactChange::does_value_mean<ContactChange::Value::to_set, std::string>(
        const boost::optional< Nullable<std::string> >&);

template
bool ContactChange::does_value_mean<ContactChange::Value::to_delete, std::string>(
        const boost::optional< Nullable<std::string> >&);

template
bool ContactChange::does_value_mean<ContactChange::Value::not_to_touch, std::string>(
        const boost::optional< Nullable<std::string> >&);

template
std::string ContactChange::get_value<std::string>(const boost::optional< Nullable<std::string> >&);


template
bool ContactChange::does_value_mean<ContactChange::Value::to_set, ContactIdent>(
        const boost::optional< boost::optional<ContactIdent> >&);

template
bool ContactChange::does_value_mean<ContactChange::Value::to_delete, ContactIdent>(
        const boost::optional< boost::optional<ContactIdent> >&);

template
bool ContactChange::does_value_mean<ContactChange::Value::not_to_touch, ContactIdent>(
        const boost::optional< boost::optional<ContactIdent> >&);

template
ContactIdent ContactChange::get_value<ContactIdent>(const boost::optional< boost::optional<ContactIdent> >&);


template
bool ContactChange::does_value_mean<ContactChange::Value::to_set, ContactChange::Address>(
        const boost::optional< Nullable<ContactChange::Address> >&);

template
bool ContactChange::does_value_mean< ContactChange::Value::to_delete, ContactChange::Address>(
        const boost::optional< Nullable<ContactChange::Address> >&);

template
bool ContactChange::does_value_mean< ContactChange::Value::not_to_touch, ContactChange::Address>(
        const boost::optional< Nullable<ContactChange::Address> >&);

template
ContactChange::Address ContactChange::get_value<ContactChange::Address>(
        const boost::optional< Nullable<ContactChange::Address> >&);


template
bool ContactChange::does_value_mean< ContactChange::Value::to_set, std::string>(
        const boost::optional<std::string>&);

template
bool ContactChange::does_value_mean< ContactChange::Value::not_to_touch, std::string>(
        const boost::optional<std::string>&);

template
std::string ContactChange::get_value<std::string>(const boost::optional<std::string>&);

ContactChange trim(const ContactChange& src)
{
    ContactChange dst;
    dst.name = trim(src.name);
    dst.organization = trim(src.organization);
    dst.streets = trim(src.streets);
    dst.city = trim(src.city);
    dst.state_or_province = trim(src.state_or_province);
    dst.postal_code = trim(src.postal_code);
    dst.country_code = trim(src.country_code);
    dst.mailing_address = trim(src.mailing_address);
    dst.telephone = trim(src.telephone);
    dst.fax = trim(src.fax);
    dst.email = trim(src.email);
    dst.notify_email = trim(src.notify_email);
    dst.vat = trim(src.vat);
    dst.ident = trim(src.ident);
    dst.authinfopw = src.authinfopw;
    dst.disclose = src.disclose;
    return dst;
}

} // namespace Epp::Contact
} // namespace Epp
