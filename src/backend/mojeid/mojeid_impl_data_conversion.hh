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
#ifndef MOJEID_IMPL_DATA_CONVERSION_HH_3FFB13E7D2574A12916DF3E34D830D12
#define MOJEID_IMPL_DATA_CONVERSION_HH_3FFB13E7D2574A12916DF3E34D830D12

#include "src/backend/mojeid/mojeid_impl_data.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"

namespace Fred {
namespace Backend {
namespace MojeIdImplData {

void from_into(const Address& src, LibFred::Contact::PlaceAddress& dst);
void from_into(const ShippingAddress& src, LibFred::ContactAddress& dst);

void from_into(const LibFred::Contact::PlaceAddress& src, Address& dst);
void from_into(const LibFred::ContactAddress& src, ShippingAddress& dst);

void from_into(const CreateContact& src, LibFred::InfoContactData& dst);
void from_into(const UpdateContact& src, LibFred::InfoContactData& dst);
void from_into(const ValidatedContactData& src, LibFred::InfoContactData& dst);
void from_into(const UpdateTransferContact& src, LibFred::InfoContactData& dst);

void from_into(const LibFred::InfoContactData& src, InfoContact& dst);

} // namespace Fred::Backend::MojeIdImplData
} // namespace Fred::Backend
} // namespace Fred

#endif
