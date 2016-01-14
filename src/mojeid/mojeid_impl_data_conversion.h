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

#ifndef MOJEID_IMPL_DATA_CONVERSION_H_88A849ACE9E8EA6C6F74FC5AE03FF2EF//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define MOJEID_IMPL_DATA_CONVERSION_H_88A849ACE9E8EA6C6F74FC5AE03FF2EF

#include "src/mojeid/mojeid_impl_data.h"
#include "src/fredlib/contact/info_contact_data.h"

namespace Registry {

namespace MojeIDImplData {

void from_into(const Address         &src, Fred::Contact::PlaceAddress &dst);
void from_into(const ShippingAddress &src, Fred::ContactAddress        &dst);

void from_into(const Fred::Contact::PlaceAddress &src, Address         &dst);
void from_into(const Fred::ContactAddress        &src, ShippingAddress &dst);

void from_into(const CreateContact &src, Fred::InfoContactData &dst);
void from_into(const UpdateContact &src, Fred::InfoContactData &dst);
void from_into(const SetContact    &src, Fred::InfoContactData &dst);

void from_into(const Fred::InfoContactData &src, CreateContact &dst);
void from_into(const Fred::InfoContactData &src, UpdateContact &dst);
void from_into(const Fred::InfoContactData &src, SetContact    &dst);

}//namespace Registry::MojeIDImplData
}//namespace Registry

#endif//MOJEID_IMPL_DATA_CONVERSION_H_88A849ACE9E8EA6C6F74FC5AE03FF2EF
