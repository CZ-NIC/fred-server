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

struct set_ssn
{
    set_ssn(const Nullable< std::string > &_ssn, const char *_ssn_type, Fred::InfoContactData &_out)
    {
        this->operator()(_ssn, _ssn_type, _out);
    }
    const set_ssn& operator()(
        const Nullable< std::string > &_ssn,
        const char *_ssn_type,
        Fred::InfoContactData &_out)const
    {
        if (_out.ssntype.isnull() && !_ssn.isnull()) {
            _out.ssntype = _ssn_type;
            _out.ssn     = _ssn.get_value();
        }
        return *this;
    }
};

}//Corba::Conversion::{anonymous}

from_into< Registry::MojeID::Address, Fred::Contact::PlaceAddress >::dst_value_ref
from_into< Registry::MojeID::Address, Fred::Contact::PlaceAddress >::operator()(src_value src, dst_value_ref dst)const
{
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    Nullable< std::string > stateorprovince;

    from(              src.street1)    .into(dst.street1);
    from(empty_as_null(src.street2))   .into(street2);
    from(empty_as_null(src.street3))   .into(street3);
    from(              src.city)       .into(dst.city);
    from(empty_as_null(src.state))     .into(stateorprovince);
    from(              src.postal_code).into(dst.postalcode);
    from(              src.country)    .into(dst.country);

    if (!street2.isnull()) {
        dst.street2 = street2.get_value();
    }
    if (!street3.isnull()) {
        dst.street3 = street3.get_value();
    }
    if (!stateorprovince.isnull()) {
        dst.stateorprovince = stateorprovince.get_value();
    }
    return dst;
}

from_into< Registry::MojeID::Address, Fred::ContactAddress >::dst_value_ref
from_into< Registry::MojeID::Address, Fred::ContactAddress >::operator()(src_value src, dst_value_ref dst)const
{
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    Nullable< std::string > stateorprovince;

    from(              src.street1)    .into(dst.street1);
    from(empty_as_null(src.street2))   .into(street2);
    from(empty_as_null(src.street3))   .into(street3);
    from(              src.city)       .into(dst.city);
    from(empty_as_null(src.state))     .into(stateorprovince);
    from(              src.postal_code).into(dst.postalcode);
    from(              src.country)    .into(dst.country);

    if (!street2.isnull()) {
        dst.street2 = street2.get_value();
    }
    if (!street3.isnull()) {
        dst.street3 = street3.get_value();
    }
    if (!stateorprovince.isnull()) {
        dst.stateorprovince = stateorprovince.get_value();
    }
    return dst;
}

from_into< Registry::MojeID::ShippingAddress, Fred::ContactAddress >::dst_value_ref
from_into< Registry::MojeID::ShippingAddress, Fred::ContactAddress >::operator()(src_value src, dst_value_ref dst)const
{
    Nullable< std::string > company_name;
    Nullable< std::string > street2;
    Nullable< std::string > street3;
    Nullable< std::string > stateorprovince;

    from(empty_as_null(src.company_name)).into(company_name);
    from(              src.street1)      .into(dst.street1);
    from(empty_as_null(src.street2))     .into(street2);
    from(empty_as_null(src.street3))     .into(street3);
    from(              src.city)         .into(dst.city);
    from(empty_as_null(src.state))       .into(stateorprovince);
    from(              src.postal_code)  .into(dst.postalcode);
    from(              src.country)      .into(dst.country);

    if (!company_name.isnull()) {
        dst.company_name = company_name.get_value();
    }
    if (!street2.isnull()) {
        dst.street2 = street2.get_value();
    }
    if (!street3.isnull()) {
        dst.street3 = street3.get_value();
    }
    if (!stateorprovince.isnull()) {
        dst.stateorprovince = stateorprovince.get_value();
    }
    return dst;
}

from_into< Registry::MojeID::CreateContact, Fred::InfoContactData >::dst_value_ref
from_into< Registry::MojeID::CreateContact, Fred::InfoContactData >::operator()(src_value src, dst_value_ref dst)const
{
    std::string                      first_name;
    std::string                      last_name;
    Nullable< std::string >          birth_date;
    Nullable< std::string >          id_card_num;
    Nullable< std::string >          passport_num;
    Nullable< std::string >          ssn_id_num;
    Nullable< std::string >          vat_id_num;
    Fred::Contact::PlaceAddress      permanent;
    Nullable< Fred::ContactAddress > mailing;
    Nullable< Fred::ContactAddress > billing;
    Nullable< Fred::ContactAddress > shipping;
    Nullable< Fred::ContactAddress > shipping2;
    Nullable< Fred::ContactAddress > shipping3;
    std::string                      email;
    std::string                      telephone;

    from(              src.username)     .into(dst.handle);
    from(              src.first_name)   .into(first_name);
    from(              src.last_name)    .into(last_name);
    from(empty_as_null(src.organization)).into(dst.organization);
    from(empty_as_null(src.vat_reg_num)) .into(dst.vat);
    from(empty_as_null(src.birth_date))  .into(birth_date);
    from(empty_as_null(src.id_card_num)) .into(id_card_num);
    from(empty_as_null(src.passport_num)).into(passport_num);
    from(empty_as_null(src.ssn_id_num))  .into(ssn_id_num);
    from(empty_as_null(src.vat_id_num))  .into(vat_id_num);
    from(              src.permanent)    .into(permanent);
    from(              src.mailing)      .into(mailing);
    from(              src.billing)      .into(billing);
    from(              src.shipping)     .into(shipping);
    from(              src.shipping2)    .into(shipping2);
    from(              src.shipping3)    .into(shipping3);
    from(              src.email)        .into(email);
    from(empty_as_null(src.notify_email)).into(dst.notifyemail);
    from(              src.telephone)    .into(telephone);
    from(empty_as_null(src.fax))         .into(dst.fax);

    dst.name = first_name + " " + last_name;
    const bool contact_is_organization = !dst.organization.isnull();
    if (contact_is_organization) {
        set_ssn(vat_id_num, "ICO",      dst)
               (birth_date, "BIRTHDAY", dst);
    }
    else {
        set_ssn(birth_date, "BIRTHDAY", dst)
               (vat_id_num, "ICO",      dst);
    }
    set_ssn(id_card_num,  "OP",   dst)
           (passport_num, "PASS", dst)
           (ssn_id_num,   "MPSV", dst);
    dst.place = permanent;
    if (!mailing.isnull()) {
        dst.addresses[Fred::ContactAddressType::MAILING] = mailing.get_value();
    }
    if (!billing.isnull()) {
        dst.addresses[Fred::ContactAddressType::BILLING] = billing.get_value();
    }
    if (!shipping.isnull()) {
        dst.addresses[Fred::ContactAddressType::SHIPPING] = shipping.get_value();
    }
    if (!shipping2.isnull()) {
        dst.addresses[Fred::ContactAddressType::SHIPPING_2] = shipping2.get_value();
    }
    if (!shipping3.isnull()) {
        dst.addresses[Fred::ContactAddressType::SHIPPING_3] = shipping3.get_value();
    }
    dst.email     = email;
    dst.telephone = telephone;
    return dst;
}

}//Corba::Conversion
}//Corba
