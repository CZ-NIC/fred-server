#include "src/corba/mojeid/corba_conversion2.h"
#include "util/types/birthdate.h"

namespace Corba {
namespace Conversion {

boost::gregorian::date& convert(const std::string &from, boost::gregorian::date &into)
{
    enum { GREGORIAN_DATE_LENGTH = 10 };
    const std::string str_date = from.substr(0, GREGORIAN_DATE_LENGTH);
    return into = boost::gregorian::from_simple_string(str_date);
}

Nullable< boost::gregorian::date > convert_as_birthdate(const Nullable< std::string > &_birth_date)
{
    if (!_birth_date.isnull()) {
        return Nullable< boost::gregorian::date >(birthdate_from_string_to_date(_birth_date.get_value()));
    }
    return Nullable< boost::gregorian::date >();
}

from_into< Registry::MojeID::Date, std::string >::dst_value_ref
from_into< Registry::MojeID::Date, std::string >::operator()(src_value src, dst_value_ref dst)const
{
    return from(src.value.in()).into(dst);
}

into_from< Registry::MojeID::Date, const char* >::dst_value_ref
into_from< Registry::MojeID::Date, const char* >::operator()(dst_value_ref dst, src_value src)const
{
    into(dst.value).from(src);
    return dst;
}

from_into< Registry::MojeID::Date, boost::gregorian::date >::dst_value_ref
from_into< Registry::MojeID::Date, boost::gregorian::date >::operator()(src_value src, dst_value_ref dst)const
{
    return dst = boost::gregorian::from_simple_string(from(src).into< std::string >());
}

into_from< Registry::MojeID::Date, boost::gregorian::date >::dst_value_ref
into_from< Registry::MojeID::Date, boost::gregorian::date >::operator()(dst_value_ref dst, src_value src)const
{
    return into(dst).from(boost::gregorian::to_iso_extended_string(src).c_str());
}

from_into< Registry::MojeID::DateTime, std::string >::dst_value_ref
from_into< Registry::MojeID::DateTime, std::string >::operator()(src_value src, dst_value_ref dst)const
{
    return from(src.value.in()).into(dst);
}

into_from< Registry::MojeID::DateTime, const char* >::dst_value_ref
into_from< Registry::MojeID::DateTime, const char* >::operator()(dst_value_ref dst, src_value src)const
{
    into(dst.value).from(src);
    return dst;
}

from_into< Registry::MojeID::DateTime, boost::posix_time::ptime >::dst_value_ref
from_into< Registry::MojeID::DateTime, boost::posix_time::ptime >::operator()(src_value src, dst_value_ref dst)const
{
    std::string iso_extended = from(src).into< std::string >();//2015-06-24T13:05:03.000123
    enum { T_POS = 10 };
    if ((T_POS <= iso_extended.length()) && (iso_extended[T_POS] == 'T')) {
        iso_extended[T_POS] = ' ';
    }
    return dst = boost::posix_time::time_from_string(iso_extended);
}

into_from< Registry::MojeID::DateTime, boost::posix_time::ptime >::dst_value_ref
into_from< Registry::MojeID::DateTime, boost::posix_time::ptime >::operator()(dst_value_ref dst, src_value src)const
{
    return into(dst).from(boost::posix_time::to_iso_extended_string(src).c_str());
}

into_from< Registry::MojeID::Date, boost::posix_time::ptime >::dst_value_ref
into_from< Registry::MojeID::Date, boost::posix_time::ptime >::operator()(dst_value_ref dst, src_value src)const
{
    return into(dst).from(src.date());
}

namespace {

struct set_ssn
{
    template < typename T >
    set_ssn(const Nullable< T > &_ssn, const char *_ssn_type, Fred::InfoContactData &_out)
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
    typedef boost::gregorian::date Date;
    const set_ssn& operator()(
        const Nullable< Date > &_ssn,
        const char *_ssn_type,
        Fred::InfoContactData &_out)const
    {
        if (_out.ssntype.isnull() && !_ssn.isnull()) {
            _out.ssntype = _ssn_type;
            _out.ssn     = boost::gregorian::to_iso_extended_string(_ssn.get_value());
        }
        return *this;
    }
};

template < typename T >
class set_optional_value
{
public:
    typedef T type;
    typedef Optional< type > O;
    typedef Nullable< type > N;
    ~set_optional_value()
    {
        if (!n_.isnull()) {
            o_ = n_.get_value();
        }
    }
    N& if_not_null() { return n_; }
private:
    set_optional_value(O &_o):o_(_o) { }
    set_optional_value(const set_optional_value &_src):o_(_src.o_), n_(_src.n_) { }
    template < typename TYPE >
    friend set_optional_value< TYPE > set_optional(Optional< TYPE > &_o);
    O &o_;
    N n_;
};

template < typename T >
set_optional_value< T > set_optional(Optional< T > &_o)
{
    return set_optional_value< T >(_o);
}

template < typename DICT >
class add_value_into_dictionary
{
public:
    ~add_value_into_dictionary()
    {
        if (!value_.isnull()) {
            dict_[key_] = value_.get_value();
        }
    }
    typedef typename DICT::mapped_type mapped_type;
    Nullable< mapped_type >& if_not_null() { return value_; }
private:
    typedef typename DICT::key_type key_type;
    add_value_into_dictionary(DICT &_dict, const key_type &_key):dict_(_dict), key_(_key) { }
    add_value_into_dictionary(const add_value_into_dictionary &_src):dict_(_src.dict_), key_(_src.key_) { }
    template < typename KEY, typename VALUE, typename CONVERTIBLE_TO_KEY >
    friend add_value_into_dictionary< std::map< KEY, VALUE > > add_value(std::map< KEY, VALUE > &_dict,
                                                                         const CONVERTIBLE_TO_KEY &_key);
    DICT &dict_;
    const key_type key_;
    Nullable< mapped_type > value_;
};

template < typename KEY, typename VALUE, typename CONVERTIBLE_TO_KEY >
add_value_into_dictionary< std::map< KEY, VALUE > > add_value(std::map< KEY, VALUE > &_dict,
                                                              const CONVERTIBLE_TO_KEY &_key)
{
    return add_value_into_dictionary< std::map< KEY, VALUE > >(_dict, _key);
}

template < typename T >
Nullable< T > optional_to_nullable(const Optional< T > &_o)
{
    return _o.isset() ? Nullable< T >(_o.get_value())
                      : Nullable< T >();
}

template < typename KEY, typename VALUE, typename CONVERTIBLE_TO_KEY >
Nullable< VALUE > missing_as_null(const std::map< KEY, VALUE > &_dict, const CONVERTIBLE_TO_KEY &_key)
{
    typename std::map< KEY, VALUE >::const_iterator item_ptr = _dict.find(_key);
    return item_ptr == _dict.end() ? Nullable< VALUE >()
                                   : Nullable< VALUE >(item_ptr->second);
}

}//Corba::Conversion::{anonymous}

from_into< Registry::MojeID::Address, Fred::Contact::PlaceAddress >::dst_value_ref
from_into< Registry::MojeID::Address, Fred::Contact::PlaceAddress >::operator()(src_value src, dst_value_ref dst)const
{
    from(src.street1.in())    .into(             dst.street1);
    from(src.street2.in())    .into(set_optional(dst.street2).if_not_null());
    from(src.street3.in())    .into(set_optional(dst.street3).if_not_null());
    from(src.city.in())       .into(             dst.city);
    from(src.state.in())      .into(set_optional(dst.stateorprovince).if_not_null());
    from(src.postal_code.in()).into(             dst.postalcode);
    from(src.country.in())    .into(             dst.country);
    return dst;
}

from_into< Registry::MojeID::Address, Fred::ContactAddress >::dst_value_ref
from_into< Registry::MojeID::Address, Fred::ContactAddress >::operator()(src_value src, dst_value_ref dst)const
{
    from(src.street1.in())    .into(             dst.street1);
    from(src.street2.in())    .into(set_optional(dst.street2).if_not_null());
    from(src.street3.in())    .into(set_optional(dst.street3).if_not_null());
    from(src.city.in())       .into(             dst.city);
    from(src.state.in())      .into(set_optional(dst.stateorprovince).if_not_null());
    from(src.postal_code.in()).into(             dst.postalcode);
    from(src.country.in())    .into(             dst.country);
    return dst;
}

from_into< Registry::MojeID::ShippingAddress, Fred::ContactAddress >::dst_value_ref
from_into< Registry::MojeID::ShippingAddress, Fred::ContactAddress >::operator()(src_value src, dst_value_ref dst)const
{
    from(src.company_name.in()).into(set_optional(dst.company_name).if_not_null());
    from(src.street1.in())     .into(             dst.street1);
    from(src.street2.in())     .into(set_optional(dst.street2).if_not_null());
    from(src.street3.in())     .into(set_optional(dst.street3).if_not_null());
    from(src.city.in())        .into(             dst.city);
    from(src.state.in())       .into(set_optional(dst.stateorprovince).if_not_null());
    from(src.postal_code.in()) .into(             dst.postalcode);
    from(src.country.in())     .into(             dst.country);
    return dst;
}

from_into< Registry::MojeID::CreateContact, Fred::InfoContactData >::dst_value_ref
from_into< Registry::MojeID::CreateContact, Fred::InfoContactData >::operator()(src_value src, dst_value_ref dst)const
{
    std::string                        first_name;
    std::string                        last_name;
    Nullable< boost::gregorian::date > birth_date;
    Nullable< std::string >            id_card_num;
    Nullable< std::string >            passport_num;
    Nullable< std::string >            ssn_id_num;
    Nullable< std::string >            vat_id_num;
    typedef Fred::ContactAddressType   AddrType;

    from(src.username.in())    .into(          dst.handle);
    from(src.first_name.in())  .into(          first_name);
    from(src.last_name.in())   .into(          last_name);
    from(src.organization.in()).into(          dst.organization);
    from(src.vat_reg_num.in()) .into(          dst.vat);
    from(src.birth_date.in())  .into(          birth_date);
    from(src.id_card_num.in()) .into(          id_card_num);
    from(src.passport_num.in()).into(          passport_num);
    from(src.ssn_id_num.in())  .into(          ssn_id_num);
    from(src.vat_id_num.in())  .into(          vat_id_num);
    from(src.permanent)   .into(          dst.place);
    from(src.mailing.in())     .into(add_value(dst.addresses, AddrType::MAILING).if_not_null());
    from(src.billing.in())     .into(add_value(dst.addresses, AddrType::BILLING).if_not_null());
    from(src.shipping.in())    .into(add_value(dst.addresses, AddrType::SHIPPING).if_not_null());
    from(src.shipping2.in())   .into(add_value(dst.addresses, AddrType::SHIPPING_2).if_not_null());
    from(src.shipping3.in())   .into(add_value(dst.addresses, AddrType::SHIPPING_3).if_not_null());
    from(src.email.in())       .into(          dst.email);
    from(src.notify_email.in()).into(          dst.notifyemail);
    from(src.telephone.in())   .into(          dst.telephone);
    from(src.fax.in())         .into(          dst.fax);

    dst.name = first_name + " " + last_name;
    const bool contact_is_organization = !dst.organization.isnull();
    (contact_is_organization
     ? set_ssn(vat_id_num,   "ICO",  dst) : set_ssn(birth_date, "BIRTHDAY", dst))
              (id_card_num,  "OP",   dst)
              (passport_num, "PASS", dst)
              (ssn_id_num,   "MPSV", dst);
    return dst;
}

from_into< Registry::MojeID::UpdateContact, Fred::InfoContactData >::dst_value_ref
from_into< Registry::MojeID::UpdateContact, Fred::InfoContactData >::operator()(src_value src, dst_value_ref dst)const
{
    std::string                        first_name;
    std::string                        last_name;
    Nullable< boost::gregorian::date > birth_date;
    Nullable< std::string >            id_card_num;
    Nullable< std::string >            passport_num;
    Nullable< std::string >            ssn_id_num;
    Nullable< std::string >            vat_id_num;
    typedef Fred::ContactAddressType   AddrType;

    dst.id = src.id;
    dst.handle.clear();

    from(src.first_name.in())  .into(          first_name);
    from(src.last_name.in())   .into(          last_name);
    from(src.organization.in()).into(          dst.organization);
    from(src.vat_reg_num.in()) .into(          dst.vat);
    from(src.birth_date.in())  .into(          birth_date);
    from(src.id_card_num.in()) .into(          id_card_num);
    from(src.passport_num.in()).into(          passport_num);
    from(src.ssn_id_num.in())  .into(          ssn_id_num);
    from(src.vat_id_num.in())  .into(          vat_id_num);
    from(src.permanent)   .into(          dst.place);
    from(src.mailing.in())     .into(add_value(dst.addresses, AddrType::MAILING).if_not_null());
    from(src.billing.in())     .into(add_value(dst.addresses, AddrType::BILLING).if_not_null());
    from(src.shipping.in())    .into(add_value(dst.addresses, AddrType::SHIPPING).if_not_null());
    from(src.shipping2.in())   .into(add_value(dst.addresses, AddrType::SHIPPING_2).if_not_null());
    from(src.shipping3.in())   .into(add_value(dst.addresses, AddrType::SHIPPING_3).if_not_null());
    from(src.email.in())       .into(          dst.email);
    from(src.notify_email.in()).into(          dst.notifyemail);
    from(src.telephone.in())   .into(          dst.telephone);
    from(src.fax.in())         .into(          dst.fax);

    dst.name = first_name + " " + last_name;
    const bool contact_is_organization = !dst.organization.isnull();
    (contact_is_organization
     ? set_ssn(vat_id_num,   "ICO",  dst) : set_ssn(birth_date, "BIRTHDAY", dst))
              (id_card_num,  "OP",   dst)
              (passport_num, "PASS", dst)
              (ssn_id_num,   "MPSV", dst);
    return dst;
}

from_into< Registry::MojeID::SetContact, Fred::InfoContactData >::dst_value_ref
from_into< Registry::MojeID::SetContact, Fred::InfoContactData >::operator()(src_value src, dst_value_ref dst)const
{
    Nullable< boost::gregorian::date > birth_date;
    Nullable< std::string >            vat_id_num;
    typedef Fred::ContactAddressType   AddrType;

    from(src.organization.in()).into(          dst.organization);
    from(src.vat_reg_num.in()) .into(          dst.vat);
    from(src.birth_date.in())  .into(          birth_date);
    from(src.vat_id_num.in())  .into(          vat_id_num);
    from(src.permanent)   .into(          dst.place);
    from(src.mailing.in())     .into(add_value(dst.addresses, AddrType::MAILING).if_not_null());
    from(src.email.in())       .into(          dst.email);
    from(src.notify_email.in()).into(          dst.notifyemail);
    from(src.telephone.in())   .into(          dst.telephone);
    from(src.fax.in())         .into(          dst.fax);

    const bool contact_is_organization = !dst.organization.isnull();
    (contact_is_organization
     ? set_ssn(vat_id_num,   "ICO",  dst) : set_ssn(birth_date, "BIRTHDAY", dst));
    return dst;
}

into_from< IDL_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >::dst_value_ref
into_from< IDL_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >::operator()(dst_value_ref dst, src_value src)const
{
    if (src.street1_absent) {
        into(dst.street1).from(Registry::MojeID::REQUIRED);
    }
    if (src.city_absent) {
        into(dst.city).from(Registry::MojeID::REQUIRED);
    }
    if (src.postalcode_absent) {
        into(dst.postal_code).from(Registry::MojeID::REQUIRED);
    }
    if (src.country_absent) {
        into(dst.country).from(Registry::MojeID::REQUIRED);
    }
    return dst;
}

into_from< IDL_MANDATORY_ADDRESS_VALIDATION_ERROR, IMPL_MANDATORY_CONTACT_ADDRESS_ERROR >::dst_value_ref
into_from< IDL_MANDATORY_ADDRESS_VALIDATION_ERROR, IMPL_MANDATORY_CONTACT_ADDRESS_ERROR >::operator()(dst_value_ref dst, src_value src)const
{
    if (src.absent) {
        into(dst.address_presence).from(Registry::MojeID::REQUIRED);
    }
    if (src.street1_absent) {
        into(dst.street1).from(Registry::MojeID::REQUIRED);
    }
    if (src.city_absent) {
        into(dst.city).from(Registry::MojeID::REQUIRED);
    }
    if (src.postalcode_absent) {
        into(dst.postal_code).from(Registry::MojeID::REQUIRED);
    }
    if (src.country_absent) {
        into(dst.country).from(Registry::MojeID::REQUIRED);
    }
    return dst;
}

into_from< IDL_SHIPPING_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >::dst_value_ref
into_from< IDL_SHIPPING_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >::operator()(dst_value_ref dst, src_value src)const
{
    if (src.street1_absent) {
        into(dst.street1).from(Registry::MojeID::REQUIRED);
    }
    if (src.city_absent) {
        into(dst.city).from(Registry::MojeID::REQUIRED);
    }
    if (src.postalcode_absent) {
        into(dst.postal_code).from(Registry::MojeID::REQUIRED);
    }
    if (src.country_absent) {
        into(dst.country).from(Registry::MojeID::REQUIRED);
    }
    return dst;
}

into_from< Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR, Registry::MojeID::MojeID2Impl::CheckCreateContactPrepare >::dst_value_ref
into_from< Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR, Registry::MojeID::MojeID2Impl::CheckCreateContactPrepare >::operator()(dst_value_ref dst, src_value src)const
{
    if (!src.Fred::MojeID::check_contact_username_availability::success()) {
        into(dst.username).from(Registry::MojeID::NOT_AVAILABLE);
    }
    else if (!src.Fred::MojeID::check_contact_username::success()) {
        if (src.Fred::MojeID::check_contact_username::absent) {
            into(dst.username).from(Registry::MojeID::REQUIRED);
        }
        else if (src.Fred::MojeID::check_contact_username::invalid) {
            into(dst.username).from(Registry::MojeID::INVALID);
        }
    }

    if (!src.Fred::check_contact_name::success()) {
        if (src.Fred::check_contact_name::first_name_absent) {
            into(dst.first_name).from(Registry::MojeID::REQUIRED);
        }
        if (src.Fred::check_contact_name::last_name_absent) {
            into(dst.last_name).from(Registry::MojeID::REQUIRED);
        }
    }

    if (!src.Fred::MojeID::check_contact_birthday_validity::success()) {
        into(dst.birth_date).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_email_presence::success()) {
        into(dst.email).from(Registry::MojeID::REQUIRED);
    }
    else if (!src.Fred::check_contact_email_validity::success()) {
        into(dst.email).from(Registry::MojeID::INVALID);
    }
    else if (!src.Fred::check_contact_email_availability::success()) {
        into(dst.email).from(Registry::MojeID::NOT_AVAILABLE);
    }

    if (!src.Fred::check_contact_notifyemail_validity::success()) {
        into(dst.notify_email).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_phone_presence::success()) {
        into(dst.phone).from(Registry::MojeID::REQUIRED);
    }
    else if (!src.Fred::check_contact_phone_validity::success()) {
        into(dst.phone).from(Registry::MojeID::INVALID);
    }
    else if (!src.Fred::check_contact_phone_availability::success()) {
        into(dst.phone).from(Registry::MojeID::NOT_AVAILABLE);
    }

    if (!src.Fred::check_contact_fax_validity::success()) {
        into(dst.fax).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_place_address::success()) {
        into(dst.permanent).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_place_address& >(src)));
    }
    if (!src.Fred::check_contact_addresses_mailing::success()) {
        into(dst.mailing).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                               static_cast< const Fred::check_contact_addresses_mailing& >(src)));
    }
    if (!src.Fred::check_contact_addresses_billing::success()) {
        into(dst.billing).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                               static_cast< const Fred::check_contact_addresses_billing& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping::success()) {
        into(dst.shipping).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                static_cast< const Fred::check_contact_addresses_shipping& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping2::success()) {
        into(dst.shipping2).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_addresses_shipping2& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping3::success()) {
        into(dst.shipping3).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_addresses_shipping3& >(src)));
    }
    return dst;
}


into_from< Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR, Registry::MojeID::MojeID2Impl::CheckMojeIDRegistration >::dst_value_ref
into_from< Registry::MojeID::Server::REGISTRATION_VALIDATION_ERROR, Registry::MojeID::MojeID2Impl::CheckMojeIDRegistration >::operator()(dst_value_ref dst, src_value src)const
{
    if (!src.Fred::MojeID::check_contact_username_availability::success()) {
        into(dst.username).from(Registry::MojeID::NOT_AVAILABLE);
    }

    if (!src.Fred::check_contact_name::success()) {
        if (src.Fred::check_contact_name::first_name_absent) {
            into(dst.first_name).from(Registry::MojeID::REQUIRED);
        }
        if (src.Fred::check_contact_name::last_name_absent) {
            into(dst.last_name).from(Registry::MojeID::REQUIRED);
        }
    }

    if (!src.Fred::MojeID::check_contact_birthday::success()) {
        into(dst.birth_date).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_email_presence::success()) {
        into(dst.email).from(Registry::MojeID::REQUIRED);
    }
    else if (!src.Fred::check_contact_email_validity::success()) {
        into(dst.email).from(Registry::MojeID::INVALID);
    }
    else if (!src.Fred::check_contact_email_availability::success()) {
            into(dst.email).from(Registry::MojeID::NOT_AVAILABLE);
    }

    if (!src.Fred::check_contact_notifyemail_validity::success()) {
        into(dst.notify_email).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_phone_presence::success()) {
        into(dst.phone).from(Registry::MojeID::REQUIRED);
    }
    else if (!src.Fred::check_contact_phone_validity::success()) {
        into(dst.phone).from(Registry::MojeID::INVALID);
    }
    else if (!src.Fred::check_contact_phone_availability::success()) {
        into(dst.phone).from(Registry::MojeID::NOT_AVAILABLE);
    }

    if (!src.Fred::check_contact_fax_validity::success()) {
        into(dst.fax).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_place_address::success()) {
        into(dst.permanent).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_place_address& >(src)));
    }
    if (!src.Fred::check_contact_addresses_mailing::success()) {
        into(dst.mailing).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                               static_cast< const Fred::check_contact_addresses_mailing& >(src)));
    }
    if (!src.Fred::check_contact_addresses_billing::success()) {
        into(dst.billing).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                               static_cast< const Fred::check_contact_addresses_billing& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping::success()) {
        into(dst.shipping).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                static_cast< const Fred::check_contact_addresses_shipping& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping2::success()) {
        into(dst.shipping2).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_addresses_shipping2& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping3::success()) {
        into(dst.shipping3).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_addresses_shipping3& >(src)));
    }

    return dst;
}


into_from< IDL_UPDATE_CONTACT_PREPARE_ERROR, IMPL_UPDATE_CONTACT_PREPARE_ERROR >::dst_value_ref
into_from< IDL_UPDATE_CONTACT_PREPARE_ERROR, IMPL_UPDATE_CONTACT_PREPARE_ERROR >::operator()(dst_value_ref dst, src_value src)const
{
    if (!src.Fred::check_contact_name::success()) {
        if (src.Fred::check_contact_name::first_name_absent) {
            into(dst.first_name).from(Registry::MojeID::REQUIRED);
        }
        if (src.Fred::check_contact_name::last_name_absent) {
            into(dst.last_name).from(Registry::MojeID::REQUIRED);
        }
    }

    if (!src.Fred::MojeID::check_contact_birthday::success()) {
        into(dst.birth_date).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_email_presence::success()) {
        into(dst.email).from(Registry::MojeID::REQUIRED);
    }
    else if (!src.Fred::check_contact_email_validity::success()) {
        into(dst.email).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_notifyemail_validity::success()) {
        into(dst.notify_email).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_phone_presence::success()) {
        into(dst.phone).from(Registry::MojeID::REQUIRED);
    }
    else if (!src.Fred::check_contact_phone_validity::success()) {
        into(dst.phone).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_fax_validity::success()) {
        into(dst.fax).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_place_address::success()) {
        into(dst.permanent).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_place_address& >(src)));
    }
    if (!src.Fred::check_contact_addresses_mailing::success()) {
        into(dst.mailing).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                               static_cast< const Fred::check_contact_addresses_mailing& >(src)));
    }
    if (!src.Fred::check_contact_addresses_billing::success()) {
        into(dst.billing).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                               static_cast< const Fred::check_contact_addresses_billing& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping::success()) {
        into(dst.shipping).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                static_cast< const Fred::check_contact_addresses_shipping& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping2::success()) {
        into(dst.shipping2).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_addresses_shipping2& >(src)));
    }
    if (!src.Fred::check_contact_addresses_shipping3::success()) {
        into(dst.shipping3).from(static_cast< const IMPL_CONTACT_ADDRESS_ERROR& >(
                                 static_cast< const Fred::check_contact_addresses_shipping3& >(src)));
    }

    return dst;
}


into_from< Registry::MojeID::Address, Fred::Contact::PlaceAddress >::dst_value_ref
into_from< Registry::MojeID::Address, Fred::Contact::PlaceAddress >::operator()(dst_value_ref dst, src_value src)const
{
    into(dst.street1)    .from(                     src.street1);
    into(dst.street2)    .from(optional_to_nullable(src.street2));
    into(dst.street3)    .from(optional_to_nullable(src.street3));
    into(dst.city)       .from(                     src.city);
    into(dst.state)      .from(optional_to_nullable(src.stateorprovince));
    into(dst.postal_code).from(                     src.postalcode);
    into(dst.country)    .from(                     src.country);
    return dst;
}

into_from< Registry::MojeID::Address, Fred::ContactAddress >::dst_value_ref
into_from< Registry::MojeID::Address, Fred::ContactAddress >::operator()(dst_value_ref dst, src_value src)const
{
    into(dst.street1)    .from(                     src.street1);
    into(dst.street2)    .from(optional_to_nullable(src.street2));
    into(dst.street3)    .from(optional_to_nullable(src.street3));
    into(dst.city)       .from(                     src.city);
    into(dst.state)      .from(optional_to_nullable(src.stateorprovince));
    into(dst.postal_code).from(                     src.postalcode);
    into(dst.country)    .from(                     src.country);
    return dst;
}

into_from< Registry::MojeID::ShippingAddress, Fred::ContactAddress >::dst_value_ref
into_from< Registry::MojeID::ShippingAddress, Fred::ContactAddress >::operator()(dst_value_ref dst, src_value src)const
{
    into(dst.company_name).from(optional_to_nullable(src.company_name));
    into(dst.street1)     .from(                     src.street1);
    into(dst.street2)     .from(optional_to_nullable(src.street2));
    into(dst.street3)     .from(optional_to_nullable(src.street3));
    into(dst.city)        .from(                     src.city);
    into(dst.state)       .from(optional_to_nullable(src.stateorprovince));
    into(dst.postal_code) .from(                     src.postalcode);
    into(dst.country)     .from(                     src.country);
    return dst;
}

into_from< Registry::MojeID::InfoContact, Fred::InfoContactData >::dst_value_ref
into_from< Registry::MojeID::InfoContact, Fred::InfoContactData >::operator()(dst_value_ref dst, src_value src)const
{
    if (src.place.isnull()) {
        throw std::runtime_error("Bad contact: place address must be set");
    }
    if (src.email.isnull()) {
        throw std::runtime_error("Bad contact: email must be set");
    }
    const std::string full_name = src.name.get_value_or_default();
    const std::string::size_type last_name_start = full_name.find_last_of(' ');
    const bool last_name_present = last_name_start != std::string::npos;
    const std::string first_name = full_name.substr(0, last_name_present ? last_name_start : 0);
    const std::string last_name = last_name_present ? full_name.substr(last_name_start + 1) : std::string();

    into(dst.id)          .from(src.id);
    into(dst.first_name)  .from(first_name);
    into(dst.last_name)   .from(last_name);
    into(dst.organization).from(src.organization);
    into(dst.vat_reg_num) .from(src.vat);
    into(dst.birth_date)  .from(Nullable< boost::gregorian::date >());
    into(dst.id_card_num) .from(reinterpret_cast< const char* >(NULL));
    into(dst.passport_num).from(reinterpret_cast< const char* >(NULL));
    into(dst.ssn_id_num)  .from(reinterpret_cast< const char* >(NULL));
    into(dst.vat_id_num)  .from(reinterpret_cast< const char* >(NULL));
    if (!src.ssntype.isnull()) {
        const std::string ssn_type = src.ssntype.get_value();
        if (ssn_type == "BIRTHDAY") {
            into(dst.birth_date).from(convert_as_birthdate(src.ssn));
        }
        else if (ssn_type == "ICO") {
            into(dst.vat_id_num)  .from(src.ssn);
        }
        else if (ssn_type == "OP") {
            into(dst.id_card_num) .from(src.ssn);
        }
        else if (ssn_type == "PASS") {
            into(dst.passport_num).from(src.ssn);
        }
        else if (ssn_type == "MPSV") {
            into(dst.ssn_id_num)  .from(src.ssn);
        }
    }
    into(dst.permanent)   .from(                src.place.get_value());
    into(dst.mailing)     .from(missing_as_null(src.addresses, Fred::ContactAddressType::MAILING));
    into(dst.billing)     .from(missing_as_null(src.addresses, Fred::ContactAddressType::BILLING));
    into(dst.shipping)    .from(missing_as_null(src.addresses, Fred::ContactAddressType::SHIPPING));
    into(dst.shipping2)   .from(missing_as_null(src.addresses, Fred::ContactAddressType::SHIPPING_2));
    into(dst.shipping3)   .from(missing_as_null(src.addresses, Fred::ContactAddressType::SHIPPING_3));
    into(dst.email)       .from(                src.email.get_value());
    into(dst.notify_email).from(                src.notifyemail);
    into(dst.telephone)   .from(                src.telephone);
    into(dst.fax)         .from(                src.fax);
    return dst;
}


into_from< Registry::MojeID::ContactStateInfo, Registry::MojeID::ContactStateData >::dst_value_ref
into_from< Registry::MojeID::ContactStateInfo, Registry::MojeID::ContactStateData >::operator()(
    dst_value_ref dst,
    src_value src)const
{
    into(dst.contact_id).from(src.get_contact_id());
    into(dst.mojeid_activation_datetime).from(src.get_validity(Fred::Object::State::MOJEID_CONTACT));
    if (!src.presents(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT)) {
        throw std::runtime_error("Bad contact: conditionallyIdentifiedContact state doesn't present");
    }
    into(dst.conditionally_identification_date).from(src.get_validity(Fred::Object::State::CONDITIONALLY_IDENTIFIED_CONTACT));
    if (!src.presents(Fred::Object::State::IDENTIFIED_CONTACT)) {
        into(dst.identification_date).from(Nullable< boost::posix_time::ptime >());
    }
    else {
        into(dst.identification_date).from(src.get_validity(Fred::Object::State::IDENTIFIED_CONTACT));
    }
    if (!src.presents(Fred::Object::State::VALIDATED_CONTACT)) {
        into(dst.validation_date).from(Nullable< boost::posix_time::ptime >());
    }
    else {
        into(dst.validation_date).from(src.get_validity(Fred::Object::State::VALIDATED_CONTACT));
    }
    if (!src.presents(Fred::Object::State::LINKED)) {
        into(dst.linked_date).from(Nullable< boost::posix_time::ptime >());
    }
    else {
        into(dst.linked_date).from(src.get_validity(Fred::Object::State::LINKED));
    }
    return dst;
}

into_from< IDL_MESSAGE_LIMIT_EXCEEDED, IMPL_MESSAGE_LIMIT_EXCEEDED >::dst_value_ref
into_from< IDL_MESSAGE_LIMIT_EXCEEDED, IMPL_MESSAGE_LIMIT_EXCEEDED >::operator()(
    dst_value_ref dst,
    src_value src)const
{
    into(dst.limit_expire_date).from(src.limit_expire_date);
    into(dst.limit_count).from(src.limit_count);
    into(dst.limit_days).from(src.limit_days);
    return dst;
}

into_from< IDL_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR, IMPL_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR >::dst_value_ref
into_from< IDL_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR, IMPL_CREATE_VALIDATION_REQUEST_VALIDATION_ERROR >::operator()(dst_value_ref dst, src_value src)const
{
    if (!src.Fred::check_contact_name::success()) {
        if (src.Fred::check_contact_name::first_name_absent) {
            into(dst.first_name).from(Registry::MojeID::REQUIRED);
        }
        if (src.Fred::check_contact_name::last_name_absent) {
            into(dst.last_name).from(Registry::MojeID::REQUIRED);
        }
    }

    if (!src.Fred::check_contact_place_address_mandatory::success()) {
        if (!src.Fred::check_contact_place_address_mandatory::success()) {
            into(dst.permanent).from(static_cast< const IMPL_MANDATORY_CONTACT_ADDRESS_ERROR& >(
                                     static_cast< const Fred::check_contact_place_address_mandatory& >(src)));
        }
    }

    if (!src.Fred::check_contact_email_presence::success()) {
        into(dst.email).from(Registry::MojeID::REQUIRED);
    }
    else if (!src.Fred::check_contact_email_validity::success()) {
        into(dst.email).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_phone_validity::success()) {
        into(dst.phone).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_notifyemail_validity::success()) {
        into(dst.notify_email).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::check_contact_fax_validity::success()) {
        into(dst.fax).from(Registry::MojeID::INVALID);
    }

    if (!src.Fred::MojeID::check_contact_ssn::success()) {
        into(dst.ssn).from(src.Fred::MojeID::check_contact_ssn::invalid ? Registry::MojeID::INVALID
                                                                        : Registry::MojeID::REQUIRED);
    }
    return dst;
}

}//Corba::Conversion
}//Corba
