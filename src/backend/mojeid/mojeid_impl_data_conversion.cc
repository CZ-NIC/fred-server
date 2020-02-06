/*
 * Copyright (C) 2016-2020  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "src/backend/mojeid/mojeid_impl_data_conversion.hh"
#include "src/backend/mojeid/mojeid_impl_data.hh"
#include "src/deprecated/libfred/registrable_object/contact/ssntype.hh"
#include "src/util/types/birthdate.hh"
#include <boost/algorithm/string/trim.hpp>

namespace Fred {
namespace Backend {
namespace MojeIdImplData {

namespace {

template <class SRC_ADDR_TYPE>
void common_conversion_into_fred(const SRC_ADDR_TYPE& src, LibFred::Contact::PlaceAddress& dst)
{
    dst.street1 = src.street1;
    if (!src.street2.isnull())
    {
        dst.street2 = src.street2.get_value();
    }
    if (!src.street3.isnull())
    {
        dst.street3 = src.street3.get_value();
    }
    dst.city = src.city;
    if (!src.state.isnull())
    {
        dst.stateorprovince = src.state.get_value();
    }
    dst.postalcode = src.postal_code;
    dst.country = src.country;
}

template <class DST_ADDR_TYPE>
void common_conversion_from_fred(const LibFred::Contact::PlaceAddress& src, DST_ADDR_TYPE& dst)
{
    dst.street1 = src.street1;
    if (src.street2.isset())
    {
        dst.street2 = src.street2.get_value();
    }
    if (src.street3.isset())
    {
        dst.street3 = src.street3.get_value();
    }
    dst.city = src.city;
    if (src.stateorprovince.isset())
    {
        dst.state = src.stateorprovince.get_value();
    }
    dst.postal_code = src.postalcode;
    dst.country = src.country;
}

template <class SRC_TYPE, class DST_TYPE>
void from_into_nullable(const SRC_TYPE& src, Nullable<DST_TYPE>& dst)
{
    DST_TYPE value;
    from_into(src, value);
    dst = value;
}

// SRC_INFO_TYPE ~ UpdateTransferContact without telephone
template <class SRC_INFO_TYPE>
void minimal_common_conversion_into_fred(const SRC_INFO_TYPE& src, LibFred::InfoContactData& dst)
{
    dst.organization = src.organization;
    dst.vat = src.vat_reg_num;
    const bool contact_is_organization = !dst.organization.isnull() && !dst.organization.get_value().empty();
    if (contact_is_organization)
    {
        if (!src.vat_id_num.isnull())
        {
            dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::ico);
            dst.ssn = src.vat_id_num.get_value();
        }
        else if (!src.birth_date.isnull())
        {
            dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::birthday);
            dst.ssn = src.birth_date.get_value().value;
        }
    }
    else
    {
        if (!src.birth_date.isnull())
        {
            dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::birthday);
            dst.ssn = src.birth_date.get_value().value;
        }
        else if (!src.vat_id_num.isnull())
        {
            dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::ico);
            dst.ssn = src.vat_id_num.get_value();
        }
    }
    from_into_nullable(src.permanent, dst.place);
    if (!src.mailing.isnull())
    {
        LibFred::ContactAddress mailing;
        from_into(src.mailing.get_value(), mailing);
        dst.addresses[LibFred::ContactAddressType::MAILING] = mailing;
    }
    dst.email = src.email;
    dst.notifyemail = src.notify_email;
    dst.fax = src.fax;
}

// DST_INFO_TYPE ~ UpdateTransferContact without telephone
template <class DST_INFO_TYPE>
void minimal_common_conversion_from_fred(const LibFred::InfoContactData& src, DST_INFO_TYPE& dst)
{
    dst.organization = src.organization;
    dst.vat_reg_num = src.vat;
    if (!src.ssn.isnull() && !src.ssntype.isnull())
    {
        switch (Conversion::Enums::from_db_handle<LibFred::SSNType>(src.ssntype.get_value()))
        {
            case LibFred::SSNType::birthday:
            {
                dst.birth_date =
                        Fred::Backend::MojeIdImplData::Birthdate(
                                boost::gregorian::to_iso_extended_string(
                                        birthdate_from_string_to_date(src.ssn.get_value())));
                break;
            }
            case LibFred::SSNType::ico:
                dst.vat_id_num = src.ssn.get_value();
                break;
            default:
                break;
        }
    }
    if (!src.place.isnull())
    {
        from_into(src.place.get_value(), dst.permanent);
    }

    LibFred::ContactAddressList::const_iterator addr_ptr = src.addresses.find(LibFred::ContactAddressType::MAILING);
    if (addr_ptr != src.addresses.end())
    {
        from_into_nullable(addr_ptr->second, dst.mailing);
    }

    if (!src.email.isnull())
    {
        dst.email = src.email.get_value();
    }
    dst.notify_email = src.notifyemail;
    dst.fax = src.fax;
}

// SRC_INFO_TYPE ~ CreateContact without username and telephone
template <class SRC_INFO_TYPE>
void common_conversion_into_fred(const SRC_INFO_TYPE& src, LibFred::InfoContactData& dst)
{
    minimal_common_conversion_into_fred(src, dst);
    dst.name = src.name;

    if (dst.ssntype.isnull() || dst.ssn.isnull())
    {
        if (!src.id_card_num.isnull())
        {
            dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::op);
            dst.ssn = src.id_card_num.get_value();
        }
        else if (!src.passport_num.isnull())
        {
            dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::pass);
            dst.ssn = src.passport_num.get_value();
        }
        else if (!src.ssn_id_num.isnull())
        {
            dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::mpsv);
            dst.ssn = src.ssn_id_num.get_value();
        }
    }

    if (!src.billing.isnull())
    {
        LibFred::ContactAddress billing;
        from_into(src.billing.get_value(), billing);
        dst.addresses[LibFred::ContactAddressType::BILLING] = billing;
    }
    if (!src.shipping.isnull())
    {
        LibFred::ContactAddress shipping;
        from_into(src.shipping.get_value(), shipping);
        dst.addresses[LibFred::ContactAddressType::SHIPPING] = shipping;
    }
    if (!src.shipping2.isnull())
    {
        LibFred::ContactAddress shipping;
        from_into(src.shipping2.get_value(), shipping);
        dst.addresses[LibFred::ContactAddressType::SHIPPING_2] = shipping;
    }
    if (!src.shipping3.isnull())
    {
        LibFred::ContactAddress shipping;
        from_into(src.shipping3.get_value(), shipping);
        dst.addresses[LibFred::ContactAddressType::SHIPPING_3] = shipping;
    }
}

// DST_INFO_TYPE ~ CreateContact without username and telephone
template <class DST_INFO_TYPE>
void common_conversion_from_fred(const LibFred::InfoContactData& src, DST_INFO_TYPE& dst)
{
    minimal_common_conversion_from_fred(src, dst);

    dst.name = boost::algorithm::trim_copy(src.name.get_value_or_default());

    if (!src.ssn.isnull() && !src.ssntype.isnull())
    {
        switch (Conversion::Enums::from_db_handle<LibFred::SSNType>(src.ssntype.get_value()))
        {
            case LibFred::SSNType::op:
                dst.id_card_num = src.ssn.get_value();
                break;
            case LibFred::SSNType::pass:
                dst.passport_num = src.ssn.get_value();
                break;
            case LibFred::SSNType::mpsv:
                dst.ssn_id_num = src.ssn.get_value();
                break;
            default:
                break;
        }
    }

    LibFred::ContactAddressList::const_iterator addr_ptr = src.addresses.find(LibFred::ContactAddressType::BILLING);
    if (addr_ptr != src.addresses.end())
    {
        from_into_nullable(addr_ptr->second, dst.billing);
    }
    addr_ptr = src.addresses.find(LibFred::ContactAddressType::SHIPPING);
    if (addr_ptr != src.addresses.end())
    {
        from_into_nullable(addr_ptr->second, dst.shipping);
    }
    addr_ptr = src.addresses.find(LibFred::ContactAddressType::SHIPPING_2);
    if (addr_ptr != src.addresses.end())
    {
        from_into_nullable(addr_ptr->second, dst.shipping2);
    }
    addr_ptr = src.addresses.find(LibFred::ContactAddressType::SHIPPING_3);
    if (addr_ptr != src.addresses.end())
    {
        from_into_nullable(addr_ptr->second, dst.shipping3);
    }
}

} // namespace Fred::Backend::MojeIdImplData::{anonymous}

void from_into(const Address& src, LibFred::Contact::PlaceAddress& dst)
{
    common_conversion_into_fred(src, dst);
}

void from_into(const ShippingAddress& src, LibFred::ContactAddress& dst)
{
    common_conversion_into_fred(src, dst);
    if (!src.company_name.isnull())
    {
        dst.company_name = src.company_name.get_value();
    }
}

void from_into(const LibFred::Contact::PlaceAddress& src, Address& dst)
{
    common_conversion_from_fred(src, dst);
}

void from_into(const LibFred::ContactAddress& src, ShippingAddress& dst)
{
    common_conversion_from_fred(src, dst);
    if (src.company_name.isset())
    {
        dst.company_name = src.company_name.get_value();
    }
}

void from_into(const CreateContact& src, LibFred::InfoContactData& dst)
{
    common_conversion_into_fred(src, dst);
    dst.handle = src.username;
    dst.telephone = src.telephone;
}

void from_into(const UpdateContact& src, LibFred::InfoContactData& dst)
{
    common_conversion_into_fred(src, dst);
    dst.telephone = src.telephone;
}

void from_into(const ValidatedContactData& src, LibFred::InfoContactData& dst)
{
    dst.name = src.name;
    dst.ssntype = Conversion::Enums::to_db_handle(LibFred::SSNType::birthday);
    dst.ssn = src.birth_date.value;
    from_into_nullable(src.permanent, dst.place);
}

void from_into(const UpdateTransferContact& src, LibFred::InfoContactData& dst)
{
    minimal_common_conversion_into_fred(src, dst);
    dst.name = src.name;
    dst.telephone = src.telephone;
}

void from_into(const LibFred::InfoContactData& src, InfoContact& dst)
{
    common_conversion_from_fred(src, dst);
    dst.id = src.id;
    dst.telephone = src.telephone;
}

} // namespace Fred::Backend::MojeIdImplData
} // namespace Fred::Backend
} // namespace Fred
