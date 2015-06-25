#include "src/corba/mojeid/corba_conversion2.h"

namespace Corba {
namespace Conversion {

boost::gregorian::date& convert(const std::string &from, boost::gregorian::date &into)
{
    enum { GREGORIAN_DATE_LENGTH = 10 };
    const std::string str_date = from.substr(0, GREGORIAN_DATE_LENGTH);
    return into = boost::gregorian::from_simple_string(str_date);
}

from_into< Registry::MojeID::Date, boost::gregorian::date >::dst_value_ref
from_into< Registry::MojeID::Date, boost::gregorian::date >::operator()(src_value src, dst_value_ref dst)const
{
    return dst = boost::gregorian::from_simple_string(src);
}

into_from< Registry::MojeID::Date, boost::gregorian::date >::dst_value_ref
into_from< Registry::MojeID::Date, boost::gregorian::date >::operator()(dst_value_ref dst, src_value src)const
{
    return dst = CORBA::string_dup(boost::gregorian::to_iso_extended_string(src).c_str());
}

from_into< Registry::MojeID::DateTime, boost::posix_time::ptime >::dst_value_ref
from_into< Registry::MojeID::DateTime, boost::posix_time::ptime >::operator()(src_value src, dst_value_ref dst)const
{
    std::string iso_extended = src;//2015-06-24T13:05:03.000123
    enum { T_POS = 10 };
    if ((T_POS <= iso_extended.length()) && (iso_extended[T_POS] == 'T')) {
        iso_extended[T_POS] = ' ';
    }
    return dst = boost::posix_time::time_from_string(iso_extended);
}

into_from< Registry::MojeID::DateTime, boost::posix_time::ptime >::dst_value_ref
into_from< Registry::MojeID::DateTime, boost::posix_time::ptime >::operator()(dst_value_ref dst, src_value src)const
{
    return dst = CORBA::string_dup(boost::posix_time::to_iso_extended_string(src).c_str());
}

namespace {

// because frontend doesn't preserve NULL values :(
template < typename CORBA_NullableType >
const CORBA_NullableType& empty_as_null(const CORBA_NullableType &_value)
{
    if ((_value.in() == NULL) ||               // is NULL or
        (_value.in()->_value()[0] != '\0')) {  // is nonempty
        return _value;
    }
    static const CORBA_NullableType null_value;// is not NULL but is empty
    return null_value;
}

}//Corba::Conversion::{anonymous}

from_into< Registry::MojeID::Address, Fred::MojeID::Address >::dst_value_ref
from_into< Registry::MojeID::Address, Fred::MojeID::Address >::operator()(src_value src, dst_value_ref dst)const
{
    from(              src.street1)    .into(dst.street1);
    from(empty_as_null(src.street2))   .into(dst.street2);
    from(empty_as_null(src.street3))   .into(dst.street3);
    from(              src.city)       .into(dst.city);
    from(empty_as_null(src.state))     .into(dst.state);
    from(              src.postal_code).into(dst.postal_code);
    from(              src.country)    .into(dst.country);
    return dst;
}

from_into< Registry::MojeID::ShippingAddress, Fred::MojeID::ShippingAddress >::dst_value_ref
from_into< Registry::MojeID::ShippingAddress, Fred::MojeID::ShippingAddress >::operator()(src_value src, dst_value_ref dst)const
{
    from(empty_as_null(src.company_name)).into(dst.company_name);
    from(              src.street1)      .into(dst.street1);
    from(empty_as_null(src.street2))     .into(dst.street2);
    from(empty_as_null(src.street3))     .into(dst.street3);
    from(              src.city)         .into(dst.city);
    from(empty_as_null(src.state))       .into(dst.state);
    from(              src.postal_code)  .into(dst.postal_code);
    from(              src.country)      .into(dst.country);
    return dst;
}

from_into< Registry::MojeID::CreateContact, Fred::MojeID::CreateContact >::dst_value_ref
from_into< Registry::MojeID::CreateContact, Fred::MojeID::CreateContact >::operator()(src_value src, dst_value_ref dst)const
{
    from(              src.username)     .into(dst.username);
    from(              src.first_name)   .into(dst.first_name);
    from(              src.last_name)    .into(dst.last_name);
    from(empty_as_null(src.organization)).into(dst.organization);
    from(empty_as_null(src.vat_reg_num)) .into(dst.vat_reg_num);
    from(empty_as_null(src.birth_date))  .into(dst.birth_date);
    from(empty_as_null(src.id_card_num)) .into(dst.id_card_num);
    from(empty_as_null(src.passport_num)).into(dst.passport_num);
    from(empty_as_null(src.ssn_id_num))  .into(dst.ssn_id_num);
    from(empty_as_null(src.vat_id_num))  .into(dst.vat_id_num);
    from(              src.permanent)    .into(dst.permanent);
    from(              src.mailing)      .into(dst.mailing);
    from(              src.billing)      .into(dst.billing);
    from(              src.shipping)     .into(dst.shipping);
    from(              src.shipping2)    .into(dst.shipping2);
    from(              src.shipping3)    .into(dst.shipping3);
    from(              src.email)        .into(dst.email);
    from(empty_as_null(src.notify_email)).into(dst.notify_email);
    from(              src.telephone)    .into(dst.telephone);
    from(empty_as_null(src.fax))         .into(dst.fax);
    return dst;
}

}//Corba::Conversion
}//Corba
