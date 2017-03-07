#include "src/epp/contact/contact_change.h"

#include "src/epp/contact/util.h"

namespace Epp {
namespace Contact {

namespace {

template < class T >
struct ClassifyValue { };

template < >
struct ClassifyValue< boost::optional< Nullable< std::string > > >
{
    static ContactChange::Value::Meaning get_meaning_of_value(const boost::optional< Nullable< std::string > > &_value)
    {
        if (_value.is_initialized()) {
            if (_value->isnull()) {
                return ContactChange::Value::to_delete;
            }
            return ContactChange::Value::to_set;
        }
        return ContactChange::Value::not_to_touch;
    }
};

template < >
struct ClassifyValue< boost::optional< std::string > >
{
    static ContactChange::Value::Meaning get_meaning_of_value(const boost::optional< std::string > &_value)
    {
        if (_value.is_initialized()) {
            return ContactChange::Value::to_set;
        }
        return ContactChange::Value::not_to_touch;
    }
};

}//namespace Epp::{anonymous}

template < ContactChange::Value::Meaning MEANING, class T >
bool ContactChange::does_value_mean(const T &_value)
{
    return ClassifyValue< T >::get_meaning_of_value(_value) == MEANING;
}

template < class T >
T ContactChange::get_value(const boost::optional< Nullable< T > > &_value)
{
    return _value->get_value();
}

template < class T >
T ContactChange::get_value(const boost::optional< T > &_value)
{
    return *_value;
}


template
bool ContactChange::does_value_mean< ContactChange::Value::to_set,
                                     boost::optional< Nullable< std::string > > >
    (const boost::optional< Nullable< std::string > >&);

template
bool ContactChange::does_value_mean< ContactChange::Value::to_delete,
                                     boost::optional< Nullable< std::string > > >
    (const boost::optional< Nullable< std::string > >&);

template
bool ContactChange::does_value_mean< ContactChange::Value::not_to_touch,
                                     boost::optional< Nullable< std::string > > >
    (const boost::optional< Nullable< std::string > >&);

template
std::string ContactChange::get_value< std::string >(const boost::optional< Nullable< std::string > >&);


template
bool ContactChange::does_value_mean< ContactChange::Value::to_set,
                                     boost::optional< std::string > >
    (const boost::optional< std::string >&);

template
bool ContactChange::does_value_mean< ContactChange::Value::not_to_touch,
                                     boost::optional< std::string > >
    (const boost::optional< std::string >&);

template
std::string ContactChange::get_value< std::string >(const boost::optional< std::string >&);

ContactChange trim(const ContactChange& contact_change_)
{
    ContactChange contact_change;
    contact_change.name              = trim(contact_change_.name);
    contact_change.organization      = trim(contact_change_.organization);
    contact_change.streets           = trim(contact_change_.streets);
    contact_change.city              = trim(contact_change_.city);
    contact_change.state_or_province = trim(contact_change_.state_or_province);
    contact_change.postal_code       = trim(contact_change_.postal_code);
    contact_change.country_code      = trim(contact_change_.country_code);
    contact_change.telephone         = trim(contact_change_.telephone);
    contact_change.fax               = trim(contact_change_.fax);
    contact_change.email             = trim(contact_change_.email);
    contact_change.notify_email      = trim(contact_change_.notify_email);
    contact_change.vat               = trim(contact_change_.vat);
    contact_change.ident             = trim(contact_change_.ident);
    contact_change.ident_type        = contact_change_.ident_type;
    contact_change.authinfopw        = contact_change_.authinfopw;
    contact_change.disclose          = contact_change_.disclose;
    return contact_change;
};

} // namespace Epp::Contact
} // namespace Epp
