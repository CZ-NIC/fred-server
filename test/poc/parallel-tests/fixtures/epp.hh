/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#ifndef EPP_HH_8AF6D0A1353260C52C0A83D6E74140D7
#define EPP_HH_8AF6D0A1353260C52C0A83D6E74140D7

#include "test/backend/epp/util.hh"

#include "test/poc/parallel-tests/fixtures/contact.hh"
#include "test/poc/parallel-tests/fixtures/domain.hh"
#include "test/poc/parallel-tests/fixtures/has_fresh_database.hh"
#include "test/poc/parallel-tests/fixtures/keyset.hh"
#include "test/poc/parallel-tests/fixtures/nsset.hh"
#include "test/poc/parallel-tests/fixtures/operation_context.hh"
#include "test/poc/parallel-tests/fixtures/registrar.hh"
#include "test/poc/parallel-tests/fixtures/zone_access.hh"

#include "src/backend/epp/epp_response_failure.hh"
#include "src/backend/epp/session_data.hh"

#include "src/backend/epp/domain/create_domain_config_data.hh"
#include "src/backend/epp/domain/create_domain_input_data.hh"

namespace Test {

struct HasOperationContext
{
    OperationContext ctx;
};

template <typename ...SubFixtures>
struct EppFixture : HasFreshDatabase, HasOperationContext, SubFixtures...
{
    EppFixture()
        : HasFreshDatabase{},
          HasOperationContext{},
          SubFixtures{ctx}...
    { }
};

struct SessionData : ::Epp::SessionData
{
    explicit SessionData(unsigned long long registrar_id);
};

struct Session
{
    explicit Session(unsigned long long registrar_id);
    SessionData data;
};

struct SessionWithUnauthenticatedRegistrar : Session
{
    SessionWithUnauthenticatedRegistrar();
};

struct HasSessionWithUnauthenticatedRegistrar
{
    explicit HasSessionWithUnauthenticatedRegistrar(::LibFred::OperationContext&);
    SessionWithUnauthenticatedRegistrar session_with_unauthenticated_registrar;
};

struct DefaultCreateDomainInputData : ::Epp::Domain::CreateDomainInputData
{
    DefaultCreateDomainInputData();
};

struct DefaultCreateDomainConfigData : ::Epp::Domain::CreateDomainConfigData
{
    DefaultCreateDomainConfigData();
};

struct CreateDomainInputData
{
    explicit CreateDomainInputData(
            const std::string& fqdn,
            const std::string& registrant,
            const std::string& nsset,
            const std::string& keyset,
            const std::vector<std::string>& admin_contacts_add);
    ::Epp::Domain::CreateDomainInputData data;
};

struct HasRegistrarWithSessionAndCreateDomainInputData
{
    explicit HasRegistrarWithSessionAndCreateDomainInputData(::LibFred::OperationContext&);
    Registrar registrar;
    CzZone cz_zone;
    CzEnumZone cz_enum_zone;
    ZoneAccess has_cz_zone_access;
    ZoneAccess has_cz_enum_zone_access;
    Session session;
    Contact registrant;
    Nsset nsset;
    Keyset keyset;
    Contact contact1;
    Contact contact2;
    CreateDomainInputData create_domain_input_data;
};

struct TheSameDomain : Domain
{
    explicit TheSameDomain(::LibFred::OperationContext& ctx, const ::Epp::Domain::CreateDomainInputData& data, const Registrar& sponsoring_registrar);
};

struct HasRegistrarNotInZone
{
    explicit HasRegistrarNotInZone(::LibFred::OperationContext& ctx);
    Registrar registrar_not_in_zone;
};

struct HasSystemRegistrar
{
    explicit HasSystemRegistrar(::LibFred::OperationContext& ctx);
    Registrar system_registrar;
};

struct Fqdn
{
    std::string fqdn;
};

struct HasBlacklistedCzFqdn
{
    explicit HasBlacklistedCzFqdn(::LibFred::OperationContext& ctx);
    Fqdn blacklisted;
};

boost::gregorian::date get_current_local_date(::LibFred::OperationContext& ctx);

} // namespace Test

#endif//EPP_HH_8AF6D0A1353260C52C0A83D6E74140D7
