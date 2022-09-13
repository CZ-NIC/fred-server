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

#include "test/poc/parallel-tests/fixtures/epp.hh"
#include "test/poc/parallel-tests/fixtures/zone.hh"

namespace Test {

SessionData::SessionData(unsigned long long registrar_id)
    : ::Epp::SessionData{registrar_id, ::Epp::SessionLang::en, "", boost::none}
{ }

Session::Session(unsigned long long registrar_id)
    : data{registrar_id}
{ }

constexpr unsigned long long unauthenticated_registrar_id = 0;

SessionWithUnauthenticatedRegistrar::SessionWithUnauthenticatedRegistrar()
    : Session{unauthenticated_registrar_id}
{ }

HasSessionWithUnauthenticatedRegistrar::HasSessionWithUnauthenticatedRegistrar(::LibFred::OperationContext&)
    : session_with_unauthenticated_registrar{}
{ }

DefaultCreateDomainInputData::DefaultCreateDomainInputData()
    : CreateDomainInputData{
                "", // fqdn
                "", // registrant
                "", // nsset
                "", // keyset
                ::Epp::Domain::DomainRegistrationTime{1, ::Epp::Domain::DomainRegistrationTime::Unit::year}, // _period
                {}, // Util::vector_of<std::string>("CONTACT1")("CONTACT2"),
                ::Epp::Domain::EnumValidationExtension{}}
{ }

DefaultCreateDomainConfigData::DefaultCreateDomainConfigData()
    : CreateDomainConfigData{false}
{ }

CreateDomainInputData::CreateDomainInputData(
        const std::string& fqdn,
        const std::string& registrant,
        const std::string& nsset,
        const std::string& keyset,
        const std::vector<std::string>& admin_contacts_add)
    : data{fqdn, registrant, nsset, keyset,
           ::Epp::Domain::DomainRegistrationTime{1, ::Epp::Domain::DomainRegistrationTime::Unit::year}, // _period
           admin_contacts_add,
           boost::none}
{ }

HasRegistrarWithSessionAndCreateDomainInputData::HasRegistrarWithSessionAndCreateDomainInputData(::LibFred::OperationContext& ctx)
    : registrar{ctx, Setter::registrar(LibFred::CreateRegistrar{"REG-A"})},
      cz_zone{ctx},
      cz_enum_zone{ctx},
      has_cz_zone_access{ctx, cz_zone, registrar, 1'000'000},
      has_cz_enum_zone_access{ctx, cz_enum_zone, registrar, 1'000'000},
      session{registrar.data.id},
      registrant{ctx, Setter::contact(LibFred::CreateContact{"CONTACT", registrar.data.handle})},
      nsset{ctx, Setter::nsset(LibFred::CreateNsset{"NSSET", registrar.data.handle})},
      keyset{ctx, Setter::keyset(LibFred::CreateKeyset{"KEYSET", registrar.data.handle})},
      contact1{ctx, Setter::contact(LibFred::CreateContact{"CONTACT1", registrar.data.handle}, 1)},
      contact2{ctx, Setter::contact(LibFred::CreateContact{"CONTACT2", registrar.data.handle}, 2)},
      create_domain_input_data{
            cz_zone.fqdn("newdomain"),
            registrant.data.handle,
            nsset.data.handle,
            keyset.data.handle,
            { contact1.data.handle, contact2.data.handle }}
{ }

TheSameDomain::TheSameDomain(::LibFred::OperationContext& ctx, const ::Epp::Domain::CreateDomainInputData& data, const Registrar& sponsoring_registrar)
    : Domain{ctx, ::LibFred::CreateDomain{
            data.fqdn,
            sponsoring_registrar.data.handle,
            data.registrant,
            {}, // authinfopw
            Nullable<std::string>{data.nsset},
            Nullable<std::string>{data.keyset},
            data.admin_contacts,
            Test::get_current_local_date(ctx) + boost::gregorian::months{data.period.get_length_of_domain_registration_in_months()},
            Optional<boost::gregorian::date>{},
            Optional<bool>{},
            Optional<unsigned long long>{}}}
{ }

HasRegistrarNotInZone::HasRegistrarNotInZone(::LibFred::OperationContext& ctx)
    : registrar_not_in_zone{ctx, Setter::registrar(LibFred::CreateRegistrar{"REG-NOZONE"}, 1)}
{ }

HasSystemRegistrar::HasSystemRegistrar(::LibFred::OperationContext& ctx)
    : system_registrar{ctx, Setter::system_registrar(LibFred::CreateRegistrar{"REG-SYSTEM"})}
{ }

HasBlacklistedCzFqdn::HasBlacklistedCzFqdn(::LibFred::OperationContext& ctx)
    : blacklisted{CzZone::fqdn("blacklistedfqdn")}
{
    ctx.get_conn().exec_params(
            "INSERT INTO domain_blacklist (regexp, valid_from, reason) "
            "VALUES (REGEXP_REPLACE($1::TEXT, '[.]', '[.]'), NOW(), 'test purposes')",
            Database::query_param_list(blacklisted.fqdn));
}

} // namespace Test

using namespace Test;

boost::gregorian::date Test::get_current_local_date(::LibFred::OperationContext& ctx)
{
    const auto current_utc_time = boost::posix_time::time_from_string(
            static_cast<std::string>(ctx.get_conn().exec("SELECT CURRENT_TIMESTAMP AT TIME ZONE 'UTC'")[0][0]));
    //warning: timestamp conversion using local system timezone
    const auto current_local_time = boost::date_time::c_local_adjustor<ptime>::utc_to_local(current_utc_time);
    return current_local_time.date();
}
