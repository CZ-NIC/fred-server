/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  header of mojeid2 CORBA conversion functions and classes
 */

#ifndef CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965

#include "src/corba/MojeID2.hh"
#include "src/mojeid/mojeid2.h"
#include "src/corba/mojeid/corba_common_conversion2.h"
#include "src/fredlib/contact/info_contact_data.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Corba {
namespace Conversion {

boost::gregorian::date& convert(const std::string &from, boost::gregorian::date &into);

Nullable< boost::gregorian::date > convert_as_birthdate(const Nullable< std::string > &_birth_date);

template < >
struct from_into< Registry::MojeID::Date, std::string >
: from_into_base< Registry::MojeID::Date, std::string >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct into_from< Registry::MojeID::Date, const char* >
: into_from_base< Registry::MojeID::Date, const char* >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct from_into< Registry::MojeID::Date, boost::gregorian::date >
: from_into_base< Registry::MojeID::Date, boost::gregorian::date >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct into_from< Registry::MojeID::Date, boost::gregorian::date >
: into_from_base< Registry::MojeID::Date, boost::gregorian::date >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct from_into< Registry::MojeID::DateTime, std::string >
: from_into_base< Registry::MojeID::DateTime, std::string >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct into_from< Registry::MojeID::DateTime, const char* >
: into_from_base< Registry::MojeID::DateTime, const char* >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct from_into< Registry::MojeID::DateTime, boost::posix_time::ptime >
: from_into_base< Registry::MojeID::DateTime, boost::posix_time::ptime >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct into_from< Registry::MojeID::DateTime, boost::posix_time::ptime >
: into_from_base< Registry::MojeID::DateTime, boost::posix_time::ptime >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct into_from< Registry::MojeID::Date, boost::posix_time::ptime >
: into_from_base< Registry::MojeID::Date, boost::posix_time::ptime >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct from_into< Registry::MojeID::Address, Fred::Contact::PlaceAddress >
: from_into_base< Registry::MojeID::Address, Fred::Contact::PlaceAddress >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct from_into< Registry::MojeID::Address, Fred::ContactAddress >
: from_into_base< Registry::MojeID::Address, Fred::ContactAddress >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct from_into< Registry::MojeID::ShippingAddress, Fred::ContactAddress >
: from_into_base< Registry::MojeID::ShippingAddress, Fred::ContactAddress >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct from_into< Registry::MojeID::CreateContact, Fred::InfoContactData >
: from_into_base< Registry::MojeID::CreateContact, Fred::InfoContactData >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

template < >
struct from_into< Registry::MojeID::UpdateContact, Fred::InfoContactData >
: from_into_base< Registry::MojeID::UpdateContact, Fred::InfoContactData >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const;
};

typedef Registry::MojeID::AddressValidationError IDL_ADDRESS_VALIDATION_ERROR;
typedef Fred::GeneralCheck::contact_optional_address IMPL_CONTACT_ADDRESS_ERROR;

template < >
struct into_from< IDL_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >
: into_from_base< IDL_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

typedef Registry::MojeID::ShippingAddressValidationError IDL_SHIPPING_ADDRESS_VALIDATION_ERROR;
typedef Fred::GeneralCheck::contact_optional_address     IMPL_CONTACT_ADDRESS_ERROR;

template < >
struct into_from< IDL_SHIPPING_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >
: into_from_base< IDL_SHIPPING_ADDRESS_VALIDATION_ERROR, IMPL_CONTACT_ADDRESS_ERROR >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

typedef Registry::MojeID::Server::CREATE_CONTACT_PREPARE_VALIDATION_ERROR IDL_CREATE_CONTACT_PREPARE_ERROR;
typedef Registry::MojeID::MojeID2Impl::CreateContactPrepareError          IMPL_CREATE_CONTACT_PREPARE_ERROR;

template < >
struct into_from< IDL_CREATE_CONTACT_PREPARE_ERROR, IMPL_CREATE_CONTACT_PREPARE_ERROR >
: into_from_base< IDL_CREATE_CONTACT_PREPARE_ERROR, IMPL_CREATE_CONTACT_PREPARE_ERROR >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

typedef Registry::MojeID::Server::TRANSFER_CONTACT_PREPARE_VALIDATION_ERROR IDL_TRANSFER_CONTACT_PREPARE_ERROR;
typedef Registry::MojeID::MojeID2Impl::TransferContactPrepareError          IMPL_TRANSFER_CONTACT_PREPARE_ERROR;

template < >
struct into_from< IDL_TRANSFER_CONTACT_PREPARE_ERROR, IMPL_TRANSFER_CONTACT_PREPARE_ERROR >
: into_from_base< IDL_TRANSFER_CONTACT_PREPARE_ERROR, IMPL_TRANSFER_CONTACT_PREPARE_ERROR >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct into_from< Registry::MojeID::Address, Fred::Contact::PlaceAddress >
: into_from_base< Registry::MojeID::Address, Fred::Contact::PlaceAddress >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct into_from< Registry::MojeID::Address, Fred::ContactAddress >
: into_from_base< Registry::MojeID::Address, Fred::ContactAddress >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct into_from< Registry::MojeID::ShippingAddress, Fred::ContactAddress >
: into_from_base< Registry::MojeID::ShippingAddress, Fred::ContactAddress >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct into_from< Registry::MojeID::InfoContact, Fred::InfoContactData >
: into_from_base< Registry::MojeID::InfoContact, Fred::InfoContactData >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

template < >
struct into_from< Registry::MojeID::ContactStateInfo, Registry::MojeID::ContactStateData >
: into_from_base< Registry::MojeID::ContactStateInfo, Registry::MojeID::ContactStateData >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

typedef Registry::MojeID::Server::MESSAGE_LIMIT_EXCEEDED IDL_MESSAGE_LIMIT_EXCEEDED;
typedef Registry::MojeID::MojeID2Impl::MessageLimitExceeded IMPL_MESSAGE_LIMIT_EXCEEDED;

template < >
struct into_from< IDL_MESSAGE_LIMIT_EXCEEDED, IMPL_MESSAGE_LIMIT_EXCEEDED >
: into_from_base< IDL_MESSAGE_LIMIT_EXCEEDED, IMPL_MESSAGE_LIMIT_EXCEEDED >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const;
};

}//Corba::Conversion
}//Corba

#endif//CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965
