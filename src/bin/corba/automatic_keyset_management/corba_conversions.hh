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

namespace CorbaConversions {

Registry::AutomaticKeysetManagement::DomainSeq_var wrap_Domains(const Fred::Backend::AutomaticKeysetManagement::Domains& domains);

Registry::AutomaticKeysetManagement::NameserverDomainsSeq_var wrap_NameserversDomains(
        const Fred::Backend::AutomaticKeysetManagement::NameserversDomains& nameservers_domains);

Registry::AutomaticKeysetManagement::EmailAddressSeq_var wrap_EmailAddresses(const Fred::Backend::AutomaticKeysetManagement::EmailAddresses& email_addresses);

Fred::Backend::AutomaticKeysetManagement::Nsset unwrap_Nsset(const Registry::AutomaticKeysetManagement::Nsset& nsset);

Fred::Backend::AutomaticKeysetManagement::DnsKey unwrap_DnsKey(const Registry::AutomaticKeysetManagement::DnsKey& dns_key);

Fred::Backend::AutomaticKeysetManagement::DnsKeys unwrap_DnsKeys(const Registry::AutomaticKeysetManagement::DnsKeySeq& dns_key_seq);

Fred::Backend::AutomaticKeysetManagement::Keyset unwrap_Keyset(const Registry::AutomaticKeysetManagement::Keyset& keyset);

} // namespace CorbaConversions

#endif
