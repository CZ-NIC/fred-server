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
/**
 *  @file
 *  test fredlib utils
 */

#ifndef UTIL_HH_EF3AC5061B72408083458556AF1CEE43
#define UTIL_HH_EF3AC5061B72408083458556AF1CEE43

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

inline bool check_std_exception(const std::exception &e)
{
    return e.what()[0] != '\0';
}

namespace Test
{

struct autocommitting_context : virtual Test::instantiate_db_template {
    ::LibFred::OperationContextCreator ctx;

    virtual ~autocommitting_context() {
        ctx.commit_transaction();
    }
};

struct has_registrar : Test::autocommitting_context {
    ::LibFred::InfoRegistrarData registrar;

    has_registrar() {
        const std::string reg_handle = "REGISTRAR1";
        ::LibFred::CreateRegistrar(reg_handle).exec(ctx);
        registrar = ::LibFred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_contact : has_registrar {
    ::LibFred::InfoContactData contact;

    has_contact() {
        const std::string contact_handle = "CONTACT1";

        std::map<::LibFred::ContactAddressType, ::LibFred::ContactAddress> address_list;
        address_list[::LibFred::ContactAddressType::MAILING] = ::LibFred::ContactAddress(
            Optional<std::string>(),
            ::LibFred::Contact::PlaceAddress(
                    "ulice 1", "ulice 2", "ulice 3",
                    "mesto", "kraj", "12345", "CZ"
            )
        );

        Test::generate_test_data( ::LibFred::CreateContact(contact_handle, registrar.handle) )
            .set_addresses(address_list)
            .exec(ctx);

        contact = ::LibFred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
    }
};

struct has_contact_and_a_different_registrar : has_contact {
    ::LibFred::InfoRegistrarData the_different_registrar;

    has_contact_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        ::LibFred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = ::LibFred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_domain : has_contact {
    ::LibFred::InfoDomainData domain;

    ::LibFred::InfoContactData admin_contact1;
    ::LibFred::InfoContactData admin_contact2;

    has_domain() {
        {
            const std::string admin_contact1_handle = "ADM-CONTACT1";
            ::LibFred::CreateContact(admin_contact1_handle, registrar.handle).exec(ctx);
            admin_contact1 = ::LibFred::InfoContactByHandle(admin_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string admin_contact2_handle = "ADM-CONTACT2";
            ::LibFred::CreateContact(admin_contact2_handle, registrar.handle).exec(ctx);
            admin_contact2 = ::LibFred::InfoContactByHandle(admin_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string fqdn = "domena.cz";
            const std::vector<std::string> admin_contacts = boost::assign::list_of(admin_contact1.handle)(admin_contact2.handle);
            Test::generate_test_data( ::LibFred::CreateDomain(fqdn, registrar.handle, contact.handle) )
                .set_admin_contacts(admin_contacts).exec(ctx);
            domain = ::LibFred::InfoDomainByFqdn(fqdn).exec(ctx).info_domain_data;
        }
    }
};

struct has_keyset : has_contact {
    ::LibFred::InfoKeysetData keyset;

    ::LibFred::InfoContactData tech_contact1;
    ::LibFred::InfoContactData tech_contact2;

    has_keyset() {
        {
            const std::string tech_contact1_handle = "TECH-CONTACT1";
            ::LibFred::CreateContact(tech_contact1_handle, registrar.handle).exec(ctx);
            tech_contact1 = ::LibFred::InfoContactByHandle(tech_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string tech_contact2_handle = "TECH-CONTACT2";
            ::LibFred::CreateContact(tech_contact2_handle, registrar.handle).exec(ctx);
            tech_contact2 = ::LibFred::InfoContactByHandle(tech_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string handle = "KEYSET375";
            const std::vector<std::string> tech_contacts = boost::assign::list_of(tech_contact1.handle)(tech_contact2.handle);
            const std::vector<::LibFred::DnsKey> dns_keys = boost::assign::list_of
                (::LibFred::DnsKey(257, 3, 5, "carymarypodkocary456"))
                (::LibFred::DnsKey(255, 3, 5, "abrakadabra852"));

            Test::generate_test_data( ::LibFred::CreateKeyset(handle, registrar.handle) )
                .set_dns_keys(dns_keys)
                .set_tech_contacts(tech_contacts).exec(ctx);
            keyset = ::LibFred::InfoKeysetByHandle(handle).exec(ctx).info_keyset_data;
        }
    }
};

struct has_nsset : has_contact {
    ::LibFred::InfoNssetData nsset;

    ::LibFred::InfoContactData tech_contact1;
    ::LibFred::InfoContactData tech_contact2;

    has_nsset() {
        {
            const std::string tech_contact1_handle = "TECH-CONTACT1";
            ::LibFred::CreateContact(tech_contact1_handle, registrar.handle).exec(ctx);
            tech_contact1 = ::LibFred::InfoContactByHandle(tech_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string tech_contact2_handle = "TECH-CONTACT2";
            ::LibFred::CreateContact(tech_contact2_handle, registrar.handle).exec(ctx);
            tech_contact2 = ::LibFred::InfoContactByHandle(tech_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string handle = "NSSET9875123";
            const std::vector<std::string> tech_contacts = boost::assign::list_of(tech_contact1.handle)(tech_contact2.handle);

            Test::generate_test_data( ::LibFred::CreateNsset(handle, registrar.handle) )
                .set_tech_contacts(tech_contacts)
                .set_tech_check_level(3)
                .exec(ctx);

            nsset = ::LibFred::InfoNssetByHandle(handle).exec(ctx).info_nsset_data;
        }
    }
};

struct has_enum_domain : has_contact {
    ::LibFred::InfoDomainData domain;

    ::LibFred::InfoContactData admin_contact1;
    ::LibFred::InfoContactData admin_contact2;

    has_enum_domain() {
        {
            const std::string admin_contact1_handle = "ADM-CONTACT1";
            ::LibFred::CreateContact(admin_contact1_handle, registrar.handle).exec(ctx);
            admin_contact1 = ::LibFred::InfoContactByHandle(admin_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string admin_contact2_handle = "ADM-CONTACT2";
            ::LibFred::CreateContact(admin_contact2_handle, registrar.handle).exec(ctx);
            admin_contact2 = ::LibFred::InfoContactByHandle(admin_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string fqdn = "1.1.1.1.1.1.1.1.1.0.2.4.e164.arpa";
            const std::vector<std::string> admin_contacts = boost::assign::list_of(admin_contact1.handle)(admin_contact2.handle);
            Test::generate_test_data( ::LibFred::CreateDomain(fqdn, registrar.handle, contact.handle) )
                .set_admin_contacts(admin_contacts)
                .set_enum_publish_flag(true)
                .set_enum_validation_expiration(
                    boost::gregorian::from_string(
                        static_cast<std::string>( ctx.get_conn().exec("SELECT (now() + '1 year'::interval)::date")[0][0] )
                    )
                )
                .exec(ctx);
            domain = ::LibFred::InfoDomainByFqdn(fqdn).exec(ctx).info_domain_data;
        }
    }
};

struct has_domain_and_a_different_registrar : has_domain {
    ::LibFred::InfoRegistrarData the_different_registrar;

    has_domain_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        ::LibFred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = ::LibFred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_enum_domain_and_a_different_registrar : has_enum_domain {
    ::LibFred::InfoRegistrarData the_different_registrar;

    has_enum_domain_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        ::LibFred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = ::LibFred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_keyset_and_a_different_registrar : has_keyset {
    ::LibFred::InfoRegistrarData the_different_registrar;

    has_keyset_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        ::LibFred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = ::LibFred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_nsset_and_a_different_registrar : has_nsset {
    ::LibFred::InfoRegistrarData the_different_registrar;

    has_nsset_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        ::LibFred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = ::LibFred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

}

#endif
