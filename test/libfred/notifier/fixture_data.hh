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
 */

#ifndef FIXTURE_DATA_HH_C6D7DDED73CB410498DED2CDFCACBB5E
#define FIXTURE_DATA_HH_C6D7DDED73CB410498DED2CDFCACBB5E

#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "libfred/registrar/check_registrar.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"
#include "test/libfred/notifier/util.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/assign/list_of.hpp>

struct has_empty_contact : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoContactData contact;

    has_empty_contact()
    :   registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Reg Ist Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        contact(
            Test::exec( ::LibFred::CreateContact("REGISTRANT1", registrar.handle), ctx )
        )
    { }
};

struct has_full_contact : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoContactData contact;

    has_full_contact()
    :   registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Reg Ist Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        contact(
            Test::exec(
                ::LibFred::CreateContact("CONTACT1", registrar.handle)
                    .set_authinfo("AUTH656")
                    .set_name("John Doe")
                    .set_place(
                        ::LibFred::Contact::PlaceAddress(
                            "Milesovska 5",
                            "3. patro",
                            "1. dvere vlevo proti vytahu",
                            "Praha",
                            "Hl. m. Praha",
                            "130 00",
                            "CZ"
                        )
                    )
                    .set_addresses(
                        boost::assign::map_list_of
                            (::LibFred::ContactAddressType::SHIPPING, ::LibFred::ContactAddress("company name", "street 1", "street 2", "street 3", "city", "state or province", "PSTLCD1", "CZ"))
                     )
                    .set_organization("XYZ Inc.")
                    .set_telephone("+420123456789")
                    .set_fax("+420987654321")
                    .set_email("contact1@nic.cz")
                    .set_notifyemail("contact1.notify@.nic.cz")
                    .set_vat("CZ1234567890")
                    .set_ssntype("BIRTHDAY")
                    .set_ssn("31. 12. 1989")
                    .set_disclosename(true)
                    .set_discloseorganization(true)
                    .set_discloseaddress(true)
                    .set_disclosetelephone(true)
                    .set_disclosefax(true)
                    .set_discloseemail(true)
                    .set_disclosevat(true)
                    .set_discloseident(true)
                    .set_disclosenotifyemail(true)
                    .set_logd_request_id(1),
                ctx
            )
        )
    { }
};

struct has_domain : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoContactData domain_registrant;
    const ::LibFred::InfoDomainData dom;

    has_domain()
    :
        registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Reg Ist Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        domain_registrant(
            Test::exec(
                ::LibFred::CreateContact("REGISTRANT1", registrar.handle)
                    .set_email("registrant1@.nic.cz")
                    .set_notifyemail("registrant1notify@.nic.cz"),
                ctx
            )
        ),
        dom(
            Test::exec(
                ::LibFred::CreateDomain("mydomain123.cz", registrar.handle, domain_registrant.handle ),
                ctx
            )
        )
    { }
};

struct has_enum_domain : has_autocomitting_ctx {

    const bool dev_null_to_enable_setting_up_enum_zone;

    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoContactData domain_registrant;
    const ::LibFred::InfoDomainData dom;

    has_enum_domain()
    :
        dev_null_to_enable_setting_up_enum_zone(
            static_cast<bool>(
                /* TODO XXX dependent on init_cz script - existence of .cz zone */
                ctx.get_conn().exec( "UPDATE zone SET enum_zone = true WHERE fqdn = 'cz' RETURNING true " )[0][0]
            )
        ),
        registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Reg Ist Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        domain_registrant(
            Test::exec(
                ::LibFred::CreateContact("REGISTRANT1", registrar.handle)
                    .set_email("registrant1@.nic.cz")
                    .set_notifyemail("registrant1notify@.nic.cz"),
                ctx
            )
        ),
        dom(
            Test::exec(
                ::LibFred::CreateDomain("1.2.3.4.5.6.7.8.9.0.2.4.e164.arpa", registrar.handle, domain_registrant.handle )
                    .set_enum_validation_expiration( boost::posix_time::second_clock::local_time().date() )
                    .set_enum_publish_flag(false),
                ctx
            )
        )
    { }
};

struct has_empty_keyset : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoKeysetData keyset;

    has_empty_keyset()
    :
        registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Regis T Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        keyset(
            Test::exec(
                ::LibFred::CreateKeyset("MY_BIG_NSSET_1", registrar.handle),
                ctx
            )
        )
    { }
};

struct has_full_keyset : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoKeysetData keyset;

    has_full_keyset()
    :
        registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Regis T Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        keyset(
            Test::exec(
                ::LibFred::CreateKeyset("MY_BIG_NSSET_1", registrar.handle)
                    .set_authinfo("AUT_H_I_NFO")
                    .set_dns_keys(
                        boost::assign::list_of
                            ( ::LibFred::DnsKey(1, 1, 3, "da_key!!!") )
                            ( ::LibFred::DnsKey(2, 2, 3, "super_secret_key") )
                    )
                    .set_tech_contacts(
                        boost::assign::list_of
                            (
                                Test::exec(
                                    ::LibFred::CreateContact("CONTACT1", registrar.handle)
                                        .set_email("contact.1@nic.cz")
                                        .set_notifyemail("contact.1.notify@nic.cz"),
                                    ctx
                                ).handle
                            )
                            (
                                Test::exec(
                                    ::LibFred::CreateContact("CONTACT2", registrar.handle)
                                        .set_email("contact.2@nic.cz")
                                        .set_notifyemail("contact.2.notify@nic.cz"),
                                    ctx
                                ).handle
                            )
                    ),
                ctx
            )
        )
    { }
};

struct has_empty_nsset : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoNssetData nsset;

    has_empty_nsset()
    :
        registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Regis T Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        nsset(
            Test::exec(
                ::LibFred::CreateNsset("MY_BIG_NSSET_1", registrar.handle),
                ctx
            )
        )
    { }
};

struct has_full_nsset : has_autocomitting_ctx {
    const ::LibFred::InfoRegistrarData registrar;
    const ::LibFred::InfoNssetData nsset;

    has_full_nsset()
    :
        registrar(
            Test::exec(
                ::LibFred::CreateRegistrar("REGISTRAR1")
                    .set_name("Regis T Rar Jr.")
                    .set_url("registrar1.cz"),
                ctx
            )
        ),
        nsset(
            Test::exec(
                ::LibFred::CreateNsset("MY_BIG_NSSET_1", registrar.handle)
                    .set_authinfo("AUT_H_I_NFO")
                    .set_tech_check_level(3)
                    .set_dns_hosts(
                        boost::assign::list_of
                            (
                                ::LibFred::DnsHost(
                                    "host1.nic.cz",
                                    boost::assign::list_of
                                        (boost::asio::ip::address::from_string("192.168.0.1"))
                                        (boost::asio::ip::address::from_string("127.0.0.1"))
                                )
                            )
                            (
                                ::LibFred::DnsHost(
                                    "ns.wtf.net",
                                    boost::assign::list_of
                                        (boost::asio::ip::address::from_string("123.147.159.0"))
                                        (boost::asio::ip::address::from_string("4.5.6.7"))
                                )
                            )
                    )
                    .set_tech_contacts(
                        boost::assign::list_of
                            (
                                Test::exec(
                                    ::LibFred::CreateContact("CONTACT1", registrar.handle)
                                        .set_email("contact.1@nic.cz")
                                        .set_notifyemail("contact.1.notify@nic.cz"),
                                    ctx
                                ).handle
                            )
                            (
                                Test::exec(
                                    ::LibFred::CreateContact("CONTACT2", registrar.handle)
                                        .set_email("contact.2@nic.cz")
                                        .set_notifyemail("contact.2.notify@nic.cz"),
                                    ctx
                                ).handle
                            )
                    ),
                ctx
            )
        )
    { }
};

#endif
