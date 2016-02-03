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

/**
 *  @file
 *  declaration for Registry::MojeIDImplData namespace
 */

#include "src/mojeid/mojeid_impl_data_conversion.h"
#include "util/types/birthdate.h"
#include "src/fredlib/contact/ssntype.h"
#include <boost/algorithm/string/trim.hpp>

namespace Registry {

namespace MojeIDImplData {

namespace {

template < class SRC_ADDR_TYPE >
void common_conversion_into_fred(const SRC_ADDR_TYPE &src, Fred::Contact::PlaceAddress &dst)
{
    dst.street1 = src.street1;
    if (!src.street2.isnull()) {
        dst.street2 = src.street2.get_value();
    }
    if (!src.street3.isnull()) {
        dst.street3 = src.street3.get_value();
    }
    dst.city = src.city;
    if (!src.state.isnull()) {
        dst.stateorprovince = src.state.get_value();
    }
    dst.postalcode = src.postal_code;
    dst.country = src.country;
}

template < class DST_ADDR_TYPE >
void common_conversion_from_fred(const Fred::Contact::PlaceAddress &src, DST_ADDR_TYPE &dst)
{
    dst.street1 = src.street1;
    if (src.street2.isset()) {
        dst.street2 = src.street2.get_value();
    }
    if (src.street3.isset()) {
        dst.street3 = src.street3.get_value();
    }
    dst.city = src.city;
    if (src.stateorprovince.isset()) {
        dst.state = src.stateorprovince.get_value();
    }
    dst.postal_code = src.postalcode;
    dst.country = src.country;
}

template < class SRC_TYPE, class DST_TYPE >
void from_into_nullable(const SRC_TYPE &src, Nullable< DST_TYPE > &dst)
{
    DST_TYPE value;
    from_into(src, value);
    dst = value;
}

// SRC_INFO_TYPE ~ SetContact without telephone
template < class SRC_INFO_TYPE >
void minimal_common_conversion_into_fred(const SRC_INFO_TYPE &src, Fred::InfoContactData &dst)
{
    dst.organization = src.organization;
    dst.vat          = src.vat_reg_num;
    if (!src.birth_date.isnull()) {
        dst.ssntype = Conversion::Enums::into< std::string >(Fred::SSNType::BIRTHDAY);
        dst.ssn     = src.birth_date.get_value().value;
    }
    else if (!src.vat_id_num.isnull()) {
        dst.ssntype = Conversion::Enums::into< std::string >(Fred::SSNType::ICO);
        dst.ssn     = src.vat_id_num.get_value();
    }
    from_into_nullable(src.permanent, dst.place);
    if (!src.mailing.isnull()) {
        Fred::ContactAddress mailing;
        from_into(src.mailing.get_value(), mailing);
        dst.addresses[Fred::ContactAddressType::MAILING] = mailing;
    }
    dst.email       = src.email;
    dst.notifyemail = src.notify_email;
    dst.fax         = src.fax;
}

// DST_INFO_TYPE ~ SetContact without telephone
template < class DST_INFO_TYPE >
void minimal_common_conversion_from_fred(const Fred::InfoContactData &src, DST_INFO_TYPE &dst)
{
    dst.organization = src.organization;
    dst.vat_reg_num  = src.vat;
    if (!src.ssn.isnull() && !src.ssntype.isnull()) {
        switch (Fred::SSNType::from(src.ssntype.get_value())) {
            case Fred::SSNType::BIRTHDAY:
            {
                Registry::MojeIDImplData::Date birthdate;
                birthdate.value = boost::gregorian::to_iso_extended_string(
                                      birthdate_from_string_to_date(src.ssn.get_value()));
                dst.birth_date = birthdate;
                break;
            }
            case Fred::SSNType::ICO:
                dst.vat_id_num = src.ssn.get_value();
                break;
            default:
                break;
        }
    }
    if (!src.place.isnull()) {
        from_into(src.place.get_value(), dst.permanent);
    }

    Fred::ContactAddressList::const_iterator addr_ptr = src.addresses.find(Fred::ContactAddressType::MAILING);
    if (addr_ptr != src.addresses.end()) {
        from_into_nullable(addr_ptr->second, dst.mailing);
    }

    if (!src.email.isnull()) {
        dst.email = src.email.get_value();
    }
    dst.notify_email = src.notifyemail;
    dst.fax          = src.fax;
}

// SRC_INFO_TYPE ~ CreateContact without username and telephone
template < class SRC_INFO_TYPE >
void common_conversion_into_fred(const SRC_INFO_TYPE &src, Fred::InfoContactData &dst)
{
    minimal_common_conversion_into_fred(src, dst);
    dst.name = src.first_name + " " + src.last_name;

    if (dst.ssntype.isnull() || dst.ssn.isnull()) {
        if (!src.id_card_num.isnull()) {
            dst.ssntype = Conversion::Enums::into< std::string >(Fred::SSNType::OP);
            dst.ssn     = src.id_card_num.get_value();
        }
        else if (!src.passport_num.isnull()) {
            dst.ssntype = Conversion::Enums::into< std::string >(Fred::SSNType::PASS);
            dst.ssn     = src.passport_num.get_value();
        }
        else if (!src.ssn_id_num.isnull()) {
            dst.ssntype = Conversion::Enums::into< std::string >(Fred::SSNType::MPSV);
            dst.ssn     = src.ssn_id_num.get_value();
        }
    }

    if (!src.billing.isnull()) {
        Fred::ContactAddress billing;
        from_into(src.billing.get_value(), billing);
        dst.addresses[Fred::ContactAddressType::BILLING] = billing;
    }
    if (!src.shipping.isnull()) {
        Fred::ContactAddress shipping;
        from_into(src.shipping.get_value(), shipping);
        dst.addresses[Fred::ContactAddressType::SHIPPING] = shipping;
    }
    if (!src.shipping2.isnull()) {
        Fred::ContactAddress shipping;
        from_into(src.shipping2.get_value(), shipping);
        dst.addresses[Fred::ContactAddressType::SHIPPING_2] = shipping;
    }
    if (!src.shipping3.isnull()) {
        Fred::ContactAddress shipping;
        from_into(src.shipping3.get_value(), shipping);
        dst.addresses[Fred::ContactAddressType::SHIPPING_3] = shipping;
    }
}

// DST_INFO_TYPE ~ CreateContact without username and telephone
template < class DST_INFO_TYPE >
void common_conversion_from_fred(const Fred::InfoContactData &src, DST_INFO_TYPE &dst)
{
    minimal_common_conversion_from_fred(src, dst);

    const std::string name = boost::algorithm::trim_copy(src.name.get_value_or_default());
    const ::size_t last_name_start_pos = name.find_last_of(' ');
    if (last_name_start_pos == std::string::npos) {
        dst.first_name = name;
        dst.last_name.clear();
    }
    else {
        dst.first_name = boost::algorithm::trim_copy(name.substr(0, last_name_start_pos));
        dst.last_name = name.substr(last_name_start_pos + 1);
    }

    if (!src.ssn.isnull() && !src.ssntype.isnull()) {
        switch (Fred::SSNType::from(src.ssntype.get_value())) {
            case Fred::SSNType::OP:
                dst.id_card_num = src.ssn.get_value();
                break;
            case Fred::SSNType::PASS:
                dst.passport_num = src.ssn.get_value();
                break;
            case Fred::SSNType::MPSV:
                dst.ssn_id_num = src.ssn.get_value();
                break;
            default:
                break;
        }
    }

    Fred::ContactAddressList::const_iterator addr_ptr = src.addresses.find(Fred::ContactAddressType::BILLING);
    if (addr_ptr != src.addresses.end()) {
        from_into_nullable(addr_ptr->second, dst.billing);
    }
    addr_ptr = src.addresses.find(Fred::ContactAddressType::SHIPPING);
    if (addr_ptr != src.addresses.end()) {
        from_into_nullable(addr_ptr->second, dst.shipping);
    }
    addr_ptr = src.addresses.find(Fred::ContactAddressType::SHIPPING_2);
    if (addr_ptr != src.addresses.end()) {
        from_into_nullable(addr_ptr->second, dst.shipping2);
    }
    addr_ptr = src.addresses.find(Fred::ContactAddressType::SHIPPING_3);
    if (addr_ptr != src.addresses.end()) {
        from_into_nullable(addr_ptr->second, dst.shipping3);
    }
}

}//namespace Registry::MojeIDImplData::{anonymous}

void from_into(const Address &src, Fred::Contact::PlaceAddress &dst)
{
    common_conversion_into_fred(src, dst);
}

void from_into(const ShippingAddress &src, Fred::ContactAddress &dst)
{
    common_conversion_into_fred(src, dst);
    if (!src.company_name.isnull()) {
        dst.company_name = src.company_name.get_value();
    }
}

void from_into(const Fred::Contact::PlaceAddress &src, Address &dst)
{
    common_conversion_from_fred(src, dst);
}

void from_into(const Fred::ContactAddress &src, ShippingAddress &dst)
{
    common_conversion_from_fred(src, dst);
    if (src.company_name.isset()) {
        dst.company_name = src.company_name.get_value();
    }
}

void from_into(const CreateContact &src, Fred::InfoContactData &dst)
{
    common_conversion_into_fred(src, dst);
    dst.handle    = src.username;
    dst.telephone = src.telephone;
}

void from_into(const UpdateContact &src, Fred::InfoContactData &dst)
{
    common_conversion_into_fred(src, dst);
    dst.id        = src.id;
    dst.telephone = src.telephone;
}

void from_into(const SetContact &src, Fred::InfoContactData &dst)
{
    minimal_common_conversion_into_fred(src, dst);
    dst.telephone = src.telephone;
}

void from_into(const Fred::InfoContactData &src, CreateContact &dst)
{
    common_conversion_from_fred(src, dst);
    dst.username  = src.handle;
    if (!src.telephone.isnull()) {
        dst.telephone = src.telephone.get_value();
    }
}

void from_into(const Fred::InfoContactData &src, UpdateContact &dst)
{
    common_conversion_from_fred(src, dst);
    dst.id        = src.id;
    dst.telephone = src.telephone;
}

void from_into(const Fred::InfoContactData &src, SetContact &dst)
{
    minimal_common_conversion_from_fred(src, dst);
    if (!src.telephone.isnull()) {
        dst.telephone = src.telephone.get_value();
    }
}

}//namespace Registry::MojeIDImplData
}//namespace Registry
