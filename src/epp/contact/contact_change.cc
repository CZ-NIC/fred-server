#include "src/epp/contact/contact_change.h"

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

} // namespace Epp::Contact
} // namespace Epp
