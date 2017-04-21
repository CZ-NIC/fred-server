/**
 *  @file
 *  test fredlib utils
 */

#ifndef TESTS_FREDLIB_UTIL_69498451224
#define TESTS_FREDLIB_UTIL_69498451224

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

inline bool check_std_exception(const std::exception &e)
{
    return e.what()[0] != '\0';
}

namespace Test
{

struct autocommitting_context : virtual Test::instantiate_db_template {
    Fred::OperationContextCreator ctx;

    virtual ~autocommitting_context() {
        ctx.commit_transaction();
    }
};

struct has_registrar : Test::autocommitting_context {
    Fred::InfoRegistrarData registrar;

    has_registrar() {
        const std::string reg_handle = "REGISTRAR1";
        Fred::CreateRegistrar(reg_handle).exec(ctx);
        registrar = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_contact : has_registrar {
    Fred::InfoContactData contact;

    has_contact() {
        const std::string contact_handle = "CONTACT1";

        std::map<Fred::ContactAddressType, Fred::ContactAddress> address_list;
        address_list[Fred::ContactAddressType::MAILING] = Fred::ContactAddress(
            Optional<std::string>(),
            Fred::Contact::PlaceAddress(
                    "ulice 1", "ulice 2", "ulice 3",
                    "mesto", "kraj", "12345", "CZ"
            )
        );

        Test::generate_test_data( Fred::CreateContact(contact_handle, registrar.handle) )
            .set_addresses(address_list)
            .exec(ctx);

        contact = Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
    }
};

struct has_contact_and_a_different_registrar : has_contact {
    Fred::InfoRegistrarData the_different_registrar;

    has_contact_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_domain : has_contact {
    Fred::InfoDomainData domain;

    Fred::InfoContactData admin_contact1;
    Fred::InfoContactData admin_contact2;

    has_domain() {
        {
            const std::string admin_contact1_handle = "ADM-CONTACT1";
            Fred::CreateContact(admin_contact1_handle, registrar.handle).exec(ctx);
            admin_contact1 = Fred::InfoContactByHandle(admin_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string admin_contact2_handle = "ADM-CONTACT2";
            Fred::CreateContact(admin_contact2_handle, registrar.handle).exec(ctx);
            admin_contact2 = Fred::InfoContactByHandle(admin_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string fqdn = "domena.cz";
            const std::vector<std::string> admin_contacts = boost::assign::list_of(admin_contact1.handle)(admin_contact2.handle);
            Test::generate_test_data( Fred::CreateDomain(fqdn, registrar.handle, contact.handle) )
                .set_admin_contacts(admin_contacts).exec(ctx);
            domain = Fred::InfoDomainByHandle(fqdn).exec(ctx).info_domain_data;
        }
    }
};

struct has_keyset : has_contact {
    Fred::InfoKeysetData keyset;

    Fred::InfoContactData tech_contact1;
    Fred::InfoContactData tech_contact2;

    has_keyset() {
        {
            const std::string tech_contact1_handle = "TECH-CONTACT1";
            Fred::CreateContact(tech_contact1_handle, registrar.handle).exec(ctx);
            tech_contact1 = Fred::InfoContactByHandle(tech_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string tech_contact2_handle = "TECH-CONTACT2";
            Fred::CreateContact(tech_contact2_handle, registrar.handle).exec(ctx);
            tech_contact2 = Fred::InfoContactByHandle(tech_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string handle = "KEYSET375";
            const std::vector<std::string> tech_contacts = boost::assign::list_of(tech_contact1.handle)(tech_contact2.handle);
            const std::vector<Fred::DnsKey> dns_keys = boost::assign::list_of
                (Fred::DnsKey(257, 3, 5, "carymarypodkocary456"))
                (Fred::DnsKey(255, 3, 5, "abrakadabra852"));

            Test::generate_test_data( Fred::CreateKeyset(handle, registrar.handle) )
                .set_dns_keys(dns_keys)
                .set_tech_contacts(tech_contacts).exec(ctx);
            keyset = Fred::InfoKeysetByHandle(handle).exec(ctx).info_keyset_data;
        }
    }
};

struct has_nsset : has_contact {
    Fred::InfoNssetData nsset;

    Fred::InfoContactData tech_contact1;
    Fred::InfoContactData tech_contact2;

    has_nsset() {
        {
            const std::string tech_contact1_handle = "TECH-CONTACT1";
            Fred::CreateContact(tech_contact1_handle, registrar.handle).exec(ctx);
            tech_contact1 = Fred::InfoContactByHandle(tech_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string tech_contact2_handle = "TECH-CONTACT2";
            Fred::CreateContact(tech_contact2_handle, registrar.handle).exec(ctx);
            tech_contact2 = Fred::InfoContactByHandle(tech_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string handle = "NSSET9875123";
            const std::vector<std::string> tech_contacts = boost::assign::list_of(tech_contact1.handle)(tech_contact2.handle);

            Test::generate_test_data( Fred::CreateNsset(handle, registrar.handle) )
                .set_tech_contacts(tech_contacts)
                .set_tech_check_level(3)
                .exec(ctx);

            nsset = Fred::InfoNssetByHandle(handle).exec(ctx).info_nsset_data;
        }
    }
};

struct has_enum_domain : has_contact {
    Fred::InfoDomainData domain;

    Fred::InfoContactData admin_contact1;
    Fred::InfoContactData admin_contact2;

    has_enum_domain() {
        {
            const std::string admin_contact1_handle = "ADM-CONTACT1";
            Fred::CreateContact(admin_contact1_handle, registrar.handle).exec(ctx);
            admin_contact1 = Fred::InfoContactByHandle(admin_contact1_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string admin_contact2_handle = "ADM-CONTACT2";
            Fred::CreateContact(admin_contact2_handle, registrar.handle).exec(ctx);
            admin_contact2 = Fred::InfoContactByHandle(admin_contact2_handle).exec(ctx).info_contact_data;
        }

        {
            const std::string fqdn = "1.1.1.1.1.1.1.1.1.0.2.4.e164.arpa";
            const std::vector<std::string> admin_contacts = boost::assign::list_of(admin_contact1.handle)(admin_contact2.handle);
            Test::generate_test_data( Fred::CreateDomain(fqdn, registrar.handle, contact.handle) )
                .set_admin_contacts(admin_contacts)
                .set_enum_publish_flag(true)
                .set_enum_validation_expiration(
                    boost::gregorian::from_string(
                        static_cast<std::string>( ctx.get_conn().exec("SELECT (now() + '1 year'::interval)::date")[0][0] )
                    )
                )
                .exec(ctx);
            domain = Fred::InfoDomainByHandle(fqdn).exec(ctx).info_domain_data;
        }
    }
};

struct has_domain_and_a_different_registrar : has_domain {
    Fred::InfoRegistrarData the_different_registrar;

    has_domain_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_enum_domain_and_a_different_registrar : has_enum_domain {
    Fred::InfoRegistrarData the_different_registrar;

    has_enum_domain_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_keyset_and_a_different_registrar : has_keyset {
    Fred::InfoRegistrarData the_different_registrar;

    has_keyset_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_nsset_and_a_different_registrar : has_nsset {
    Fred::InfoRegistrarData the_different_registrar;

    has_nsset_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

}

#endif // #include guard end
