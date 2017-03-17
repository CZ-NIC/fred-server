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

#ifndef FIXTURE_H_389DB6E81FF44407AFC3763706918744
#define FIXTURE_H_389DB6E81FF44407AFC3763706918744

#include "tests/interfaces/epp/fixture.h"
#include "tests/interfaces/epp/util.h"
#include "tests/setup/fixtures.h"

#include "src/epp/domain/check_domain_config_data.h"
#include "src/epp/domain/create_domain_config_data.h"
#include "src/epp/domain/create_domain_input_data.h"
#include "src/epp/domain/create_domain_localized.h"
#include "src/epp/domain/delete_domain_config_data.h"
#include "src/epp/domain/domain_enum_validation.h"
#include "src/epp/domain/domain_registration_time.h"
#include "src/epp/domain/info_domain_config_data.h"
#include "src/epp/domain/renew_domain_config_data.h"
#include "src/epp/domain/renew_domain_input_data.h"
#include "src/epp/domain/renew_domain_localized.h"
#include "src/epp/domain/transfer_domain_config_data.h"
#include "src/epp/domain/update_domain_config_data.h"
#include "src/epp/domain/update_domain_input_data.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/nsset_dns_host.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "util/db/nullable.h"
#include "util/optional_value.h"
#include "util/util.h"

#include <boost/asio/ip/address.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/exception/diagnostic_information.hpp>
#include <boost/optional.hpp>

#include <set>
#include <string>
#include <vector>

namespace Test {

std::vector<std::string> vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(const std::vector<Fred::ObjectIdHandlePair>& admin_contacts);

struct DefaultCheckDomainConfigData : Epp::Domain::CheckDomainConfigData
{
    DefaultCheckDomainConfigData()
        : CheckDomainConfigData(false)
    {
    }
};

struct DefaultInfoDomainConfigData : Epp::Domain::InfoDomainConfigData
{
    DefaultInfoDomainConfigData()
        : InfoDomainConfigData(false)
    {
    }
};

struct DefaultCreateDomainConfigData : Epp::Domain::CreateDomainConfigData
{
    DefaultCreateDomainConfigData()
        : CreateDomainConfigData(false)
    {
    }
};

struct DefaultUpdateDomainConfigData : Epp::Domain::UpdateDomainConfigData
{
    DefaultUpdateDomainConfigData()
        : UpdateDomainConfigData(
                  false, // rifd_epp_operations_charging,
                  true)  // rifd_epp_update_domain_keyset_clear
    {
    }
};

struct DefaultDeleteDomainConfigData : Epp::Domain::DeleteDomainConfigData
{
    DefaultDeleteDomainConfigData()
        : DeleteDomainConfigData(false)
    {
    }
};

struct DefaultTransferDomainConfigData : Epp::Domain::TransferDomainConfigData
{
    DefaultTransferDomainConfigData()
        : TransferDomainConfigData(false)
    {
    }
};

struct DefaultRenewDomainConfigData : Epp::Domain::RenewDomainConfigData
{
    DefaultRenewDomainConfigData()
        : RenewDomainConfigData(false)
    {
    }
};

struct Nsset {
    std::string handle;
    Nsset(Fred::OperationContext& _ctx, const std::string& _registrar_handle) {
        handle = "NSSET";
        Fred::CreateNsset(handle, _registrar_handle).exec(_ctx);
    }
};

struct Keyset {
    std::string handle;
    Keyset(Fred::OperationContext& _ctx, const std::string& _registrar_handle) {
        handle = "KEYSET";
        Fred::CreateKeyset(handle, _registrar_handle).exec(_ctx);
    }
};

struct DefaultCreateDomainInputData : Epp::Domain::CreateDomainInputData
{
    DefaultCreateDomainInputData()
        : CreateDomainInputData(
                  "", // _fqdn
                  "", // _registrant
                  "", // _nsset
                  "", // _keyset
                  boost::optional<std::string>("authinfopw"), //_authinfopw
                  Epp::Domain::DomainRegistrationTime(1, Epp::Domain::DomainRegistrationTime::Unit::year), // _period
                  std::vector<std::string>(), // Util::vector_of<std::string>("CONTACT1")("CONTACT2"),
                  std::vector<Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct CreateDomainInputData
{
    Epp::Domain::CreateDomainInputData data;

    CreateDomainInputData(
            const std::string& _fqdn,
            const std::string& _registrant,
            const std::string& _nsset,
            const std::string& _keyset,
            const std::vector<std::string>& _admin_contacts_add)
        : data(
                  _fqdn,
                  _registrant,
                  _nsset,
                  _keyset,
                  boost::optional<std::string>("authinfopw"), //_authinfopw
                  Epp::Domain::DomainRegistrationTime(1, Epp::Domain::DomainRegistrationTime::Unit::year), // _period
                  _admin_contacts_add,
                  std::vector<Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct DefaultRenewDomainInputData : Epp::Domain::RenewDomainInputData
{
    DefaultRenewDomainInputData()
        : RenewDomainInputData(
                  "", // _fqdn
                  "", // _current_exdate
                  Epp::Domain::DomainRegistrationTime(1, Epp::Domain::DomainRegistrationTime::Unit::year), // _period
                  std::vector<Epp::Domain::EnumValidationExtension>())
    {
    }
};

struct RenewDomainInputData
{
    Epp::Domain::RenewDomainInputData data;

    RenewDomainInputData(
            const std::string& _fqdn)
        : data(
                  _fqdn, // _fqdn
                  "", // _current_exdate
                  Epp::Domain::DomainRegistrationTime(1, Epp::Domain::DomainRegistrationTime::Unit::year), // _period
                  std::vector<Epp::Domain::EnumValidationExtension>())
    {
    }

    RenewDomainInputData(
            const std::string& _fqdn,
            const std::string& _current_exdate,
            const Epp::Domain::DomainRegistrationTime& _period,
            const std::vector<Epp::Domain::EnumValidationExtension>& _enum_validation_extension_list)
        : data(
                  _fqdn,
                  _current_exdate,
                  _period,
                  _enum_validation_extension_list)
    {
    }
};

struct DefaultUpdateDomainInputData : Epp::Domain::UpdateDomainInputData
{
    DefaultUpdateDomainInputData()
        : UpdateDomainInputData(
                  "domain.cz",
                  Optional<std::string>(), // registrant_chg
                  Optional<std::string>(), // authinfopw_chg
                  Optional<Nullable<std::string> >(), // nsset_chg
                  Optional<Nullable<std::string> >(), // keyset_chg
                  std::vector<std::string>(), // admin_contacts_add
                  std::vector<std::string>(), // admin_contacts_rem
                  std::vector<std::string>(), // tmpcontacts_rem
                  std::vector<Epp::Domain::EnumValidationExtension>()) // enum_validation_list
    {
    }
};

struct UpdateDomainInputData
{
    Epp::Domain::UpdateDomainInputData data;

    UpdateDomainInputData(
            const std::string& _fqdn)
        : data(
                  _fqdn,
                  Optional<std::string>("a"), // registrant_chg
                  Optional<std::string>("b"), // authinfopw_chg
                  Optional<Nullable<std::string> >("c"), // nsset_chg
                  Optional<Nullable<std::string> >("d"), // keyset_chg
                  std::vector<std::string>(), // admin_contacts_add
                  std::vector<std::string>(), // admin_contacts_rem
                  std::vector<std::string>(), // tmpcontacts_rem
                  std::vector<Epp::Domain::EnumValidationExtension>()) // enum_validation_list
    {
    }
};


namespace Fixture {

struct HasRegistrarWithSessionAndCreateDomainInputData
{
    Registrar registrar;
    Session session;
    Contact registrant;
    Nsset nsset;
    Keyset keyset;
    Contact contact1;
    Contact contact2;
    CreateDomainInputData create_domain_input_data;


    HasRegistrarWithSessionAndCreateDomainInputData(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          nsset(_ctx, registrar.data.handle),
          keyset(_ctx, registrar.data.handle),
          contact1(_ctx, registrar.data.handle, "CONTACT1"),
          contact2(_ctx, registrar.data.handle, "CONTACT2"),
          create_domain_input_data("newdomain.cz",
                  registrant.data.handle,
                  nsset.handle,
                  keyset.handle,
                  Util::vector_of<std::string>(contact1.data.handle)(contact2.data.handle))
    {
    }


};

struct HasRegistrarWithSessionAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact registrant;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndRenewDomainInputData(Fred::OperationContext& _ctx)
            : registrar(_ctx),
              session(_ctx, registrar.data.id),
              registrant(_ctx, registrar.data.handle),
              renew_domain_input_data("domainforrenewal.cz")
    {
    }
};

struct HasRegistrarWithSessionAndDomainAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact registrant;
    Domain domain;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndDomainAndRenewDomainInputData(Fred::OperationContext & _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          domain(_ctx, registrar.data.handle),
          renew_domain_input_data(
                  domain.data.fqdn,
                  boost::gregorian::to_iso_extended_string(domain.data.expiration_date),
                  Epp::Domain::DomainRegistrationTime(1, Epp::Domain::DomainRegistrationTime::Unit::year),
                  std::vector<Epp::Domain::EnumValidationExtension>())
    {
    }

};

struct HasRegistrarWithSessionAndCreateDomainInputDataAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact registrant;
    Nsset nsset;
    Keyset keyset;
    Contact contact1;
    Contact contact2;
    CreateDomainInputData create_domain_input_data;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndCreateDomainInputDataAndRenewDomainInputData(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          nsset(_ctx, registrar.data.handle),
          keyset(_ctx, registrar.data.handle),
          contact1(_ctx, registrar.data.handle, "CONTACT1"),
          contact2(_ctx, registrar.data.handle, "CONTACT2"),
          create_domain_input_data(
                  "nonexistentdomain.cz",
                  registrant.data.handle,
                  nsset.handle,
                  keyset.handle,
                  Util::vector_of<std::string>(contact1.data.handle)(contact2.data.handle)),
          renew_domain_input_data(
                  create_domain_input_data.data.fqdn,
                  "",
                  Epp::Domain::DomainRegistrationTime(1, Epp::Domain::DomainRegistrationTime::Unit::year),
                  std::vector<Epp::Domain::EnumValidationExtension>())
    {
    }

};

struct HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData
{
    Registrar registrar;
    Session session;
    Contact registrant;
    Nsset nsset;
    Keyset keyset;
    Contact contact1;
    Contact contact2;
    Domain domain;
    CreateDomainInputData create_domain_input_data;
    RenewDomainInputData renew_domain_input_data;

    HasRegistrarWithSessionAndDomainAndCreateDomainInputDataAndRenewDomainInputData(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          registrant(_ctx, registrar.data.handle),
          nsset(_ctx, registrar.data.handle),
          keyset(_ctx, registrar.data.handle),
          contact1(_ctx, registrar.data.handle, "CONTACT1"),
          contact2(_ctx, registrar.data.handle, "CONTACT2"),
          domain(_ctx, registrar.data.handle),
          create_domain_input_data(
                  domain.data.fqdn,
                  registrant.data.handle,
                  nsset.handle,
                  keyset.handle,
                  Util::vector_of<std::string>(contact1.data.handle)(contact2.data.handle)),
          renew_domain_input_data(
                  domain.data.fqdn,
                  boost::gregorian::to_iso_extended_string(domain.data.expiration_date),
                  Epp::Domain::DomainRegistrationTime(1, Epp::Domain::DomainRegistrationTime::Unit::year),
                  std::vector<Epp::Domain::EnumValidationExtension>())
    {
    }

};

struct HasDataForUpdateDomain
{
    Registrar registrar;
    Session session;
    FullDomain domain;

    const std::string new_registrant_handle_;
    const std::string new_authinfopw_;
    const std::string new_nsset_handle_;
    const std::string new_keyset_handle_;
    Optional<std::string> registrant_chg_;
    Optional<std::string> authinfopw_chg_;
    Optional<Nullable<std::string> > nsset_chg_;
    Optional<Nullable<std::string> > keyset_chg_;
    std::vector<std::string> admin_contacts_add_;
    std::vector<std::string> admin_contacts_rem_;
    std::vector<std::string> tmpcontacts_rem_;
    std::vector<Epp::Domain::EnumValidationExtension> enum_validation_list_;

    HasDataForUpdateDomain(Fred::OperationContext& _ctx)
        : registrar(_ctx),
          session(_ctx, registrar.data.id),
          domain(_ctx, registrar.data.handle),
          new_registrant_handle_("REGISTRANT2"),
          new_authinfopw_(" auth info "),
          new_nsset_handle_("NSSET2"),
          new_keyset_handle_("KEYSET2")
    {
        namespace ip = boost::asio::ip;

        Fred::CreateContact(new_registrant_handle_, registrar.data.handle).exec(_ctx);

        const std::string admin_handle2 = "CONTACT2";
        Fred::CreateContact(admin_handle2, registrar.data.handle).exec(_ctx);
        const std::string admin_handle3 = "CONTACT3";
        Fred::CreateContact(admin_handle3, registrar.data.handle).exec(_ctx);

        const std::string tech_handle = "TECH2";
        Fred::CreateContact(tech_handle, registrar.data.handle).exec(_ctx);

        Fred::CreateNsset(new_nsset_handle_, registrar.data.handle)
                .set_tech_contacts(Util::vector_of<std::string>(tech_handle))
                .set_dns_hosts(
                        Util::vector_of<Fred::DnsHost>
                        (Fred::DnsHost("a.ns.nic.cz",
                                Util::vector_of<ip::address>
                                        (ip::address::from_string("11.0.0.3"))
                                        (ip::address::from_string("11.1.1.3"))))
                        (Fred::DnsHost("c.ns.nic.cz",
                                Util::vector_of<ip::address>
                                     (ip::address::from_string("11.0.0.4"))
                                     (ip::address::from_string("11.1.1.4")))))
                .exec(_ctx);

        Fred::CreateKeyset(new_keyset_handle_, registrar.data.handle)
                .set_tech_contacts(boost::assign::list_of(tech_handle))
                .exec(_ctx);


        registrant_chg_ = Optional<std::string>(new_registrant_handle_);
        authinfopw_chg_ = Optional<std::string>(new_authinfopw_);

        nsset_chg_ = Optional<Nullable<std::string> >(new_nsset_handle_);
        keyset_chg_ = Optional<Nullable<std::string> >(new_keyset_handle_);

        admin_contacts_add_ = Util::vector_of<std::string>
                                  (admin_handle2)
                                  (admin_handle3);

        admin_contacts_rem_ = Test::vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(
                domain.data.admin_contacts);

        tmpcontacts_rem_ = Util::vector_of<std::string>
                               ("WHATEVER");

    }

};

} // namespace Test::Fixture

} // namespace Test

#endif
