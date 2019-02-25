/*
 * Copyright (C) 2017-2019  CZ.NIC, z. s. p. o.
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
#ifndef CORBA_CONVERSIONS_HH_46010069E4BD4FD5B4D951E9325C1AA0
#define CORBA_CONVERSIONS_HH_46010069E4BD4FD5B4D951E9325C1AA0

#include "src/backend/automatic_keyset_management/automatic_keyset_management.hh"
#include "src/bin/corba/AutomaticKeysetManagement.hh"

namespace LibFred {
namespace Corba {

Registry::AutomaticKeysetManagement::DomainSeq_var wrap_Domains(const LibFred::AutomaticKeysetManagement::Domains& domains);

Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var wrap_NameserversDomains(
        const LibFred::AutomaticKeysetManagement::NameserversDomains& nameservers_domains);

Registry::AutomaticKeysetManagement::EmailAddressSeq_var wrap_EmailAddresses(const LibFred::AutomaticKeysetManagement::EmailAddresses& email_addresses);

LibFred::AutomaticKeysetManagement::Nsset unwrap_Nsset(const Registry::AutomaticKeysetManagement::Nsset& nsset);

LibFred::AutomaticKeysetManagement::DnsKey unwrap_DnsKey(const Registry::AutomaticKeysetManagement::DnsKey& dns_key);

LibFred::AutomaticKeysetManagement::DnsKeys unwrap_DnsKeys(const Registry::AutomaticKeysetManagement::DnsKeySeq& dns_key_seq);

LibFred::AutomaticKeysetManagement::Keyset unwrap_Keyset(const Registry::AutomaticKeysetManagement::Keyset& keyset);

} // namespace LibFred::Corba
} // namespace Corba

#endif
