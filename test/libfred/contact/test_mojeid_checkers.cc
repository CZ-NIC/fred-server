/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
#include "src/backend/mojeid/checkers.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "src/deprecated/libfred/registrable_object/contact/ssntype.hh"
#include "test/setup/fixtures.hh"
#include "util/idn_utils.hh"
#include "test/libfred/enum_to_db_handle_conversion.hh"

#include <boost/test/unit_test.hpp>
#include <string>

const std::string server_name = "test-contact-checkers";

struct test_contact_checkers_fixture : public Test::instantiate_db_template
{
    enum
    {
        MAIN = 0,
        WITH_THE_SAME_EMAIL = 1,
        WITH_THE_SAME_PHONE = 2,
        NUMBER_OF_CONTACTS = 3
    };
    LibFred::InfoContactData contact[NUMBER_OF_CONTACTS];
    ::size_t contact_with_the_same_email_os_id;
    ::size_t contact_with_the_same_phone_os_id;

    test_contact_checkers_fixture()
    {
        const std::string test_contact_handle[NUMBER_OF_CONTACTS] =
        {
            "TEST-CONTACT-HANDLE-A",
            "TEST-CONTACT-HANDLE-B",
            "TEST-CONTACT-HANDLE-C"
        };
        const std::string test_contact_name[NUMBER_OF_CONTACTS] =
        {
            "TEST-CONTACT-A NAME",
            "TEST-CONTACT-B NAME",
            "TEST-CONTACT-C NAME"
        };
        const std::string test_contact_email[NUMBER_OF_CONTACTS] =
        {
            "contact-a@sezdan.cz",
            "contact-a@sezdan.cz",//the same as main
            "contact-c@sezdan.cz"
        };
        const std::string test_contact_phone[NUMBER_OF_CONTACTS] =
        {
            "+420.987654321",
            "+420.987654322",
            "+420.987654321" //the same as main
        };
        LibFred::OperationContextCreator ctx;
        const std::string registrar_handle = static_cast< std::string >(ctx.get_conn().exec(
            "SELECT handle FROM registrar WHERE system ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        LibFred::Contact::PlaceAddress place;
        place.street1    = "Uličnická 1";
        place.city       = "Praha 1";
        place.postalcode = "11150";
        place.country    = "CZ";
        for (int idx = 0; idx < NUMBER_OF_CONTACTS; ++idx) {
            LibFred::CreateContact(test_contact_handle[idx], registrar_handle)
                .set_name(test_contact_name[idx])
                .set_place(place)
                .set_email(test_contact_email[idx])
                .set_telephone(test_contact_phone[idx])
                .set_ssntype("BIRTHDAY")
                .set_ssn("1980-02-29")
                .exec(ctx);
            contact[idx] = LibFred::InfoContactByHandle(test_contact_handle[idx]).exec(ctx).info_contact_data;
        }
        Database::Result dbres = ctx.get_conn().exec_params(
            "INSERT INTO object_state (object_id,state_id,valid_from,valid_to,ohid_from,ohid_to) "
            "SELECT id,(SELECT id FROM enum_object_states WHERE name='conditionallyIdentifiedContact'),"
                   "NOW()-'2MONTHS'::INTERVAL,NULL,historyid,NULL "
            "FROM contact_history "
            "WHERE id=$1::BIGINT "
            "ORDER BY historyid DESC "
            "LIMIT 1 "
            "RETURNING id",
            Database::query_param_list(contact[WITH_THE_SAME_EMAIL].id));
        contact_with_the_same_email_os_id = static_cast< ::size_t >(dbres[0][0]);
        dbres = ctx.get_conn().exec_params(
            "INSERT INTO object_state (object_id,state_id,valid_from,valid_to,ohid_from,ohid_to) "
            "SELECT id,(SELECT id FROM enum_object_states WHERE name='conditionallyIdentifiedContact'),"
                   "NOW()-'2MONTHS'::INTERVAL,NULL,historyid,NULL "
            "FROM contact_history "
            "WHERE id=$1::BIGINT "
            "ORDER BY historyid DESC "
            "LIMIT 1 "
            "RETURNING id",
            Database::query_param_list(contact[WITH_THE_SAME_PHONE].id));
        contact_with_the_same_phone_os_id = static_cast< ::size_t >(dbres[0][0]);
        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_checkers_fixture()
    {}
    typedef boost::mpl::list<Fred::Backend::check_contact_name,
                             Fred::Backend::check_contact_mailing_address,
                             Fred::Backend::check_contact_email_presence,
                             Fred::Backend::check_contact_email_validity,
                             Fred::Backend::check_contact_phone_presence,
                             Fred::Backend::check_contact_phone_validity,
                             Fred::Backend::check_contact_fax_validity,
                             Fred::Backend::MojeId::check_contact_username,
                             Fred::Backend::MojeId::check_contact_birthday > list_of_checks_contact;
    typedef boost::mpl::list<Fred::Backend::check_contact_email_availability,
                             Fred::Backend::check_contact_phone_availability > list_of_checks_contact_ctx;
    typedef Fred::Backend::MojeId::Check< boost::mpl::list< list_of_checks_contact,
                                           list_of_checks_contact_ctx > > SumCheck;
    typedef SumCheck::ChangeWrapper<Fred::Backend::MojeId::check_wrapper_break_on_first_error >::type SumCheckWithException;
};


BOOST_FIXTURE_TEST_SUITE(TestContactCheckers, test_contact_checkers_fixture)

/**
 * testFred::Backend::SSNType conversion functions
 */
BOOST_AUTO_TEST_CASE(fred_ssntype_conversions)
{
    LibFred::OperationContextCreator ctx;
    static const char *const sql = "SELECT type FROM enum_ssntype";
    enum_to_db_handle_conversion_test<LibFred::SSNType, 6 >(ctx, sql);
}

/**
 * test call SumCheck
*/
BOOST_AUTO_TEST_CASE(check_all_without_exceptions)
{
    try {
        LibFred::OperationContextCreator ctx;
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success());
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success());
    }
    catch (const std::exception &e) {
        BOOST_FAIL("unexpected exception: " << e.what());
    }
}

/**
 * test call SumCheckWithException
*/
BOOST_AUTO_TEST_CASE(check_all_with_exceptions)
{
    try {
        LibFred::OperationContextCreator ctx;
        const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success());
    }
    catch (const Fred::Backend::check_contact_name &e) {
        BOOST_FAIL("Fred::check_contact_name failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_mailing_address &e) {
        BOOST_FAIL("Fred::check_contact_mailing_address failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_email_presence &e) {
        BOOST_FAIL("Fred::check_contact_email_presence failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_email_validity &e) {
        BOOST_FAIL("Fred::check_contact_email_validity failure: " + contact[MAIN].email.get_value_or_default());
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_phone_presence &e) {
        BOOST_FAIL("Fred::check_contact_phone_presence failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_phone_validity &e) {
        BOOST_FAIL("Fred::check_contact_phone_validity failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_fax_validity &e) {
        BOOST_FAIL("Fred::check_contact_fax_validity failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::MojeId::check_contact_username &e) {
        BOOST_FAIL("Fred::MojeId::check_contact_username failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::MojeId::check_contact_birthday &e) {
        BOOST_FAIL("Fred::MojeId::check_contact_birthday");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_email_availability &e) {
        BOOST_FAIL("Fred::check_contact_email_availability failure");
        BOOST_CHECK(!e.success());
    }
    catch (const Fred::Backend::check_contact_phone_availability &e) {
        BOOST_FAIL("Fred::check_contact_phone_availability failure");
        BOOST_CHECK(!e.success());
    }
}

BOOST_AUTO_TEST_CASE(check_contact_name)
{
    struct TestData
    {
        TestData(bool result):result(result) { }
        TestData(const char *name, bool result):name(name),result(result) { }
        const Nullable< std::string > name;
        const bool result;
    };
    static const TestData data[] =
    {
        TestData(false),                                      //[0] doesn't present
        TestData("", false),                                  //[1] empty
        TestData(" ", false),                                 //[2] space only
        TestData(" \r\n\v\t", false),                         //[3] white spaces only
        TestData(" \r\n\v\tFrantišek \r\n\v\t", false),       //[4] first name only
        TestData(" \r\n\v\tAnti\r\n\v\tŠek \r\n\v\t", false), //[5] first name only
        TestData(" \r\n\v\tFrantišek Dobrota \r\n\v\t", true),//[6] success
        TestData("František Dobrota", true),                  //[7] success
    };
    static const TestData *const data_end = data + (sizeof(data) / sizeof(*data));
    BOOST_ASSERT(( (sizeof(data) / sizeof(*data)) == 8 ));
    for (const TestData *data_ptr = data; data_ptr < data_end; ++data_ptr) {
        LibFred::OperationContextCreator ctx;
        contact[MAIN].name = data_ptr->name;
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success() == data_ptr->result);
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success() == data_ptr->result);
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success());
        const ::size_t idx = data_ptr - data;
        switch (idx)
        {
        case 0 ... 3:
            BOOST_CHECK(result.Fred::Backend::check_contact_name::first_name_absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_name::last_name_absent);
            break;
        case 4 ... 5:
            BOOST_CHECK(!result.Fred::Backend::check_contact_name::first_name_absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_name::last_name_absent);
            break;
        case 6 ... 7:
            BOOST_CHECK(!result.Fred::Backend::check_contact_name::first_name_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_name::last_name_absent);
            break;
        }

        try {
            const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
            BOOST_CHECK(data_ptr->result);
            BOOST_CHECK(result.success());
        }
        catch (const Fred::Backend::check_contact_mailing_address&) {
            BOOST_FAIL("Fred::check_contact_mailing_address failure");
        }
        catch (const Fred::Backend::check_contact_email_presence&) {
            BOOST_FAIL("Fred::check_contact_email_presence failure");
        }
        catch (const Fred::Backend::check_contact_email_validity&) {
            BOOST_FAIL("Fred::check_contact_email_validity failure");
        }
        catch (const Fred::Backend::check_contact_phone_presence&) {
            BOOST_FAIL("Fred::check_contact_phone_presence failure");
        }
        catch (const Fred::Backend::check_contact_phone_validity&) {
            BOOST_FAIL("Fred::check_contact_phone_validity failure");
        }
        catch (const Fred::Backend::check_contact_fax_validity&) {
            BOOST_FAIL("Fred::check_contact_fax_validity failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_username&) {
            BOOST_FAIL("Fred::MojeId::check_contact_username failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_birthday&) {
            BOOST_FAIL("Fred::MojeId::check_contact_birthday");
        }
        catch (const Fred::Backend::check_contact_email_availability&) {
            BOOST_FAIL("Fred::check_contact_email_availability failure");
        }
        catch (const Fred::Backend::check_contact_phone_availability&) {
            BOOST_FAIL("Fred::check_contact_phone_availability failure");
        }
        catch (const Fred::Backend::check_contact_name &e) {
            BOOST_CHECK(!data_ptr->result);
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 0 ... 3:
                BOOST_CHECK(e.first_name_absent);
                BOOST_CHECK(e.last_name_absent);
                break;
            case 4 ... 5:
                BOOST_CHECK(!e.first_name_absent);
                BOOST_CHECK(e.last_name_absent);
                break;
            default:
                BOOST_FAIL("data[" << idx << "] may be valid");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(check_contact_mailing_address)
{
    const LibFred::InfoContactData::Address valid_addr = contact[MAIN].get_address< LibFred::ContactAddressType::MAILING >();
    for (int idx = 0; idx < 13; ++idx) {
        LibFred::OperationContextCreator ctx;
        contact[MAIN].addresses[LibFred::ContactAddressType::MAILING] = valid_addr;
        LibFred::ContactAddress &addr = contact[MAIN].addresses[LibFred::ContactAddressType::MAILING];
        bool result_success = false;
        switch (idx)
        {
        case 0:
            addr.street1 = "";//street1_absent
            break;
        case 1:
            addr.street1 = " ";//street1_absent
            break;
        case 2:
            addr.street1 = " \r\n\t\v";//street1_absent
            break;
        case 3:
            addr.city = "";//city_absent
            break;
        case 4:
            addr.city = " ";//city_absent
            break;
        case 5:
            addr.city = " \r\n\t\v";//city_absent
            break;
        case 6:
            addr.postalcode = "";//postalcode_absent
            break;
        case 7:
            addr.postalcode = " ";//postalcode_absent
            break;
        case 8:
            addr.postalcode = " \r\n\t\v";//postalcode_absent
            break;
        case 9:
            addr.country = "";//country_absent
            break;
        case 10:
            addr.country = " ";//country_absent
            break;
        case 11:
            addr.country = " \r\n\t\v";//country_absent
            break;
        case 12:
            result_success = true;
            break;
        }
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success() == result_success);
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success() == result_success);
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success());
        switch (idx)
        {
        case 0 ... 2:
            BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::street1_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::city_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::postalcode_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::country_absent);
            break;
        case 3 ... 5:
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::street1_absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::city_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::postalcode_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::country_absent);
            break;
        case 6 ... 8:
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::street1_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::city_absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::postalcode_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::country_absent);
            break;
        case 9 ... 11:
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::street1_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::city_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::postalcode_absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::country_absent);
            break;
        case 12:
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::street1_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::city_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::postalcode_absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_mailing_address::country_absent);
            break;
        }

        try {
            const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
            BOOST_CHECK(result_success);
            BOOST_CHECK(result.success());
        }
        catch (const Fred::Backend::check_contact_name&) {
            BOOST_FAIL("Fred::check_contact_name failure");
        }
        catch (const Fred::Backend::check_contact_email_presence&) {
            BOOST_FAIL("Fred::check_contact_email_presence failure");
        }
        catch (const Fred::Backend::check_contact_email_validity&) {
            BOOST_FAIL("Fred::check_contact_email_validity failure");
        }
        catch (const Fred::Backend::check_contact_phone_presence&) {
            BOOST_FAIL("Fred::check_contact_phone_presence failure");
        }
        catch (const Fred::Backend::check_contact_phone_validity&) {
            BOOST_FAIL("Fred::check_contact_phone_validity failure");
        }
        catch (const Fred::Backend::check_contact_fax_validity&) {
            BOOST_FAIL("Fred::check_contact_fax_validity failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_username&) {
            BOOST_FAIL("Fred::MojeId::check_contact_username failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_birthday&) {
            BOOST_FAIL("Fred::MojeId::check_contact_birthday");
        }
        catch (const Fred::Backend::check_contact_email_availability&) {
            BOOST_FAIL("Fred::check_contact_email_availability failure");
        }
        catch (const Fred::Backend::check_contact_phone_availability&) {
            BOOST_FAIL("Fred::check_contact_phone_availability failure");
        }
        catch (const Fred::Backend::check_contact_mailing_address &e) {
            BOOST_CHECK(!result_success);
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 0 ... 2:
                BOOST_CHECK(e.street1_absent);
                BOOST_CHECK(!e.city_absent);
                BOOST_CHECK(!e.postalcode_absent);
                BOOST_CHECK(!e.country_absent);
                break;
            case 3 ... 5:
                BOOST_CHECK(!e.street1_absent);
                BOOST_CHECK(e.city_absent);
                BOOST_CHECK(!e.postalcode_absent);
                BOOST_CHECK(!e.country_absent);
                break;
            case 6 ... 8:
                BOOST_CHECK(!e.street1_absent);
                BOOST_CHECK(!e.city_absent);
                BOOST_CHECK(e.postalcode_absent);
                BOOST_CHECK(!e.country_absent);
                break;
            case 9 ... 11:
                BOOST_CHECK(!e.street1_absent);
                BOOST_CHECK(!e.city_absent);
                BOOST_CHECK(!e.postalcode_absent);
                BOOST_CHECK(e.country_absent);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(check_contact_email)
{
    struct TestData
    {
        TestData(bool p, bool v, bool a):present(p),valid(v),available(a) { }
        TestData(const std::string &e, bool p, bool v, bool a):email(e),present(p),valid(v),available(a) { }
        const Nullable< std::string > email;
        const bool present:1;
        const bool valid:1;
        const bool available:1;
        bool success()const { return present && valid && available; }
    };
    enum { MAX_MOJEID_EMAIL_LENGTH = 200 };
    static const std::string allowed_chars = "-!#$%&'*+/=?^_`{}|~0-9A-Z";
    const std::string valid_email1 = "bezny@email.cz";
    BOOST_CHECK(Util::get_utf8_char_len(valid_email1) <= MAX_MOJEID_EMAIL_LENGTH);
    const std::string valid_email2 = "s.ruznym.bordelem." + allowed_chars + "@email.cz";
    BOOST_CHECK(Util::get_utf8_char_len(valid_email2) <= MAX_MOJEID_EMAIL_LENGTH);
    const std::string valid_email3 = "s.ip.adresou@[255.0.10.100]";
    BOOST_CHECK(Util::get_utf8_char_len(valid_email3) <= MAX_MOJEID_EMAIL_LENGTH);
    const std::string valid_email4 = "s.dia.kritikou-v-@doméně.cz";
    BOOST_CHECK(Util::get_utf8_char_len(valid_email4) <= MAX_MOJEID_EMAIL_LENGTH);
    const std::string valid_email5 = std::string(MAX_MOJEID_EMAIL_LENGTH - 33, '_') + "tak-akorat-dlouhy@dia.kritický.cz";
    BOOST_CHECK(Util::get_utf8_char_len(valid_email5) == MAX_MOJEID_EMAIL_LENGTH);
    static const TestData data[] =
    {
        TestData("_" + valid_email5,                       true,  false, true),
        TestData(                                          false, true,  false),
        TestData("",                                       false, true,  false),
        TestData(" ",                                      false, true,  false),
        TestData(" \r\n\v\t",                              false, true,  false),
        TestData(" \r\n\v\t" + valid_email1 + " \r\n\v\t", true,  true,  true),
        TestData(valid_email1,                             true,  true,  true),
        TestData(valid_email2,                             true,  true,  true),
        TestData(valid_email3,                             true,  true,  true),
        TestData(valid_email4,                             true,  true,  true),
        TestData(valid_email5,                             true,  true,  true),
        TestData(contact[MAIN].email.get_value(),          true,  true,  false)
    };
    static const TestData *const data_end = data + (sizeof(data) / sizeof(*data));
    BOOST_ASSERT(( (sizeof(data) / sizeof(*data)) == 12 ));
    for (const TestData *data_ptr = data; data_ptr < data_end; ++data_ptr) {
        LibFred::OperationContextCreator ctx;
        const ::size_t idx = data_ptr - data;
        if (idx == 11) {
            ctx.get_conn().exec_params(
                "UPDATE object_state SET valid_from=NOW()-'3WEEKS'::INTERVAL WHERE id=$1::BIGINT",
                Database::query_param_list(contact_with_the_same_email_os_id));
        }
        contact[MAIN].email = data_ptr->email;
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success() == data_ptr->success());
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success() == data_ptr->present);
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success() == data_ptr->valid);
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success() == data_ptr->available);
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::absent == result.Fred::Backend::check_contact_email_availability::absent);
        switch (idx)
        {
        case 0:
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_presence::absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::invalid);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_availability::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_availability::used_recently);
            break;
        case 1 ... 4:
            BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_validity::invalid);
            BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_availability::used_recently);
            break;
        case 5 ... 10:
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_presence::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_validity::invalid);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_availability::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_availability::used_recently);
            break;
        case 11:
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_presence::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_validity::invalid);
            BOOST_CHECK(!result.Fred::Backend::check_contact_email_availability::absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::used_recently);
            break;
        }

        try {
            const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
            BOOST_CHECK(data_ptr->success());
            BOOST_CHECK(result.success());
        }
        catch (const Fred::Backend::check_contact_name&) {
            BOOST_FAIL("Fred::check_contact_name failure");
        }
        catch (const Fred::Backend::check_contact_mailing_address&) {
            BOOST_FAIL("Fred::check_contact_mailing_address failure");
        }
        catch (const Fred::Backend::check_contact_phone_presence&) {
            BOOST_FAIL("Fred::check_contact_phone_presence failure");
        }
        catch (const Fred::Backend::check_contact_phone_validity&) {
            BOOST_FAIL("Fred::check_contact_phone_validity failure");
        }
        catch (const Fred::Backend::check_contact_fax_validity&) {
            BOOST_FAIL("Fred::check_contact_fax_validity failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_username&) {
            BOOST_FAIL("Fred::MojeId::check_contact_username failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_birthday&) {
            BOOST_FAIL("Fred::MojeId::check_contact_birthday");
        }
        catch (const Fred::Backend::check_contact_phone_availability&) {
            BOOST_FAIL("Fred::check_contact_phone_availability failure");
        }
        catch (const Fred::Backend::check_contact_email_availability &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 11:
                BOOST_CHECK(!e.absent);
                BOOST_CHECK(e.used_recently);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
        catch (const Fred::Backend::check_contact_email_validity &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 0:
                BOOST_CHECK(e.invalid);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
        catch (const Fred::Backend::check_contact_email_presence &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 1 ... 4:
                BOOST_CHECK(e.absent);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(check_contact_phone)
{
    struct TestData
    {
        TestData(bool p, bool v, bool a):present(p),valid(v),available(a) { }
        TestData(const std::string &e, bool p, bool v, bool a):phone(e),present(p),valid(v),available(a) { }
        const Nullable< std::string > phone;
        const bool present:1;
        const bool valid:1;
        const bool available:1;
        bool success()const { return present && valid && available; }
    };
    static const TestData data[] =
    {
        TestData(false, true, false),
        TestData("", false, true, false),
        TestData(" ", false, true, false),
        TestData(" \r\n\v\t", false, true, false),
        TestData("+420", true, false, true),
        TestData("+420.", true, false, true),
        TestData("420.602123456", true, false, true),
        TestData("+420602123456", true, false, true),
        TestData("602123456", true, false, true),
        TestData("+123.012345678901234", true, false, true),
        TestData("+0.0", true, true, true),
        TestData("+420.602123456", true, true, true),
        TestData("+123.01234567890123", true, true, true),
        TestData(contact[MAIN].telephone.get_value(), true, true, false)
    };
    static const TestData *const data_end = data + (sizeof(data) / sizeof(*data));
    BOOST_ASSERT(( (sizeof(data) / sizeof(*data)) == 14 ));
    for (const TestData *data_ptr = data; data_ptr < data_end; ++data_ptr) {
        LibFred::OperationContextCreator ctx;
        const ::size_t idx = data_ptr - data;
        if (idx == 13) {
            ctx.get_conn().exec_params(
                "UPDATE object_state SET valid_from=NOW()-'3WEEKS'::INTERVAL WHERE id=$1::BIGINT",
                Database::query_param_list(contact_with_the_same_phone_os_id));
        }
        contact[MAIN].telephone = data_ptr->phone;
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success() == data_ptr->success());
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success() == data_ptr->present);
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success() == data_ptr->valid);
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success() == data_ptr->available);
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::absent == result.Fred::Backend::check_contact_phone_availability::absent);
        switch (idx)
        {
        case 0 ... 3:
            BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_validity::invalid);
            BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_availability::used_recently);
            break;
        case 4 ... 9:
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_presence::absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::invalid);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_availability::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_availability::used_recently);
            break;
        case 10 ... 12:
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_presence::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_validity::invalid);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_availability::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_availability::used_recently);
            break;
        case 13:
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_presence::absent);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_validity::invalid);
            BOOST_CHECK(!result.Fred::Backend::check_contact_phone_availability::absent);
            BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::used_recently);
            break;
        }

        try {
            const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
            BOOST_CHECK(data_ptr->success());
            BOOST_CHECK(result.success());
        }
        catch (const Fred::Backend::check_contact_name&) {
            BOOST_FAIL("Fred::check_contact_name failure");
        }
        catch (const Fred::Backend::check_contact_mailing_address&) {
            BOOST_FAIL("Fred::check_contact_mailing_address failure");
        }
        catch (const Fred::Backend::check_contact_email_presence&) {
            BOOST_FAIL("Fred::check_contact_email_presence failure");
        }
        catch (const Fred::Backend::check_contact_email_validity&) {
            BOOST_FAIL("Fred::check_contact_email_validity failure");
        }
        catch (const Fred::Backend::check_contact_fax_validity&) {
            BOOST_FAIL("Fred::check_contact_fax_validity failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_username&) {
            BOOST_FAIL("Fred::MojeId::check_contact_username failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_birthday&) {
            BOOST_FAIL("Fred::MojeId::check_contact_birthday");
        }
        catch (const Fred::Backend::check_contact_email_availability&) {
            BOOST_FAIL("Fred::check_contact_email_availability failure");
        }
        catch (const Fred::Backend::check_contact_phone_availability &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 13:
                BOOST_CHECK(!e.absent);
                BOOST_CHECK(e.used_recently);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
        catch (const Fred::Backend::check_contact_phone_validity &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 4 ... 9:
                BOOST_CHECK(e.invalid);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
        catch (const Fred::Backend::check_contact_phone_presence &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            switch (idx)
            {
            case 0 ... 3:
                BOOST_CHECK(e.absent);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(check_contact_fax)
{
    struct TestData
    {
        TestData(bool result):result(result) { }
        TestData(const std::string &fax, bool result):fax(fax),result(result) { }
        const Nullable< std::string > fax;
        const bool result;
    };
    static const TestData data[] =
    {
        TestData(true),
        TestData("", true),
        TestData(" ", true),
        TestData(" \r\n\v\t", true),
        TestData("+0.0", true),
        TestData("+420.602123456", true),
        TestData("+123.01234567890123", true),
        TestData("+420", false),
        TestData("+420.", false),
        TestData("420.602123456", false),
        TestData("+420602123456", false),
        TestData("602123456", false),
        TestData("+123.012345678901234", false)
    };
    static const TestData *const data_end = data + (sizeof(data) / sizeof(*data));
    BOOST_ASSERT(( (sizeof(data) / sizeof(*data)) == 13 ));
    for (const TestData *data_ptr = data; data_ptr < data_end; ++data_ptr) {
        LibFred::OperationContextCreator ctx;
        contact[MAIN].fax = data_ptr->fax;
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success() == data_ptr->result);
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success() == data_ptr->result);
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::invalid == !data_ptr->result);
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success());

        try {
            const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
            BOOST_CHECK(data_ptr->result);
            BOOST_CHECK(result.success());
        }
        catch (const Fred::Backend::check_contact_name&) {
            BOOST_FAIL("Fred::check_contact_name failure");
        }
        catch (const Fred::Backend::check_contact_mailing_address&) {
            BOOST_FAIL("Fred::check_contact_mailing_address failure");
        }
        catch (const Fred::Backend::check_contact_email_presence&) {
            BOOST_FAIL("Fred::check_contact_email_presence failure");
        }
        catch (const Fred::Backend::check_contact_email_validity&) {
            BOOST_FAIL("Fred::check_contact_email_validity failure");
        }
        catch (const Fred::Backend::check_contact_phone_presence&) {
            BOOST_FAIL("Fred::check_contact_phone_presence failure");
        }
        catch (const Fred::Backend::check_contact_phone_validity&) {
            BOOST_FAIL("Fred::check_contact_phone_validity failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_username&) {
            BOOST_FAIL("Fred::MojeId::check_contact_username failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_birthday&) {
            BOOST_FAIL("Fred::MojeId::check_contact_birthday");
        }
        catch (const Fred::Backend::check_contact_email_availability&) {
            BOOST_FAIL("Fred::check_contact_email_availability failure");
        }
        catch (const Fred::Backend::check_contact_phone_availability&) {
            BOOST_FAIL("Fred::check_contact_phone_availability failure");
        }
        catch (const Fred::Backend::check_contact_fax_validity &e) {
            BOOST_CHECK(!data_ptr->result);
            BOOST_CHECK(!e.success());
            const ::size_t idx = data_ptr - data;
            switch (idx)
            {
            case 7 ... 12:
                BOOST_CHECK(e.invalid);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(check_contact_username)
{
    struct TestData
    {
        TestData(const std::string &u, bool p, bool v):username(u),present(p),valid(v) { }
        const std::string username;
        const bool present:1;
        const bool valid:1;
        bool success()const { return present && valid; }
    };
    static const TestData data[] =
    {
        TestData("", false, true),
        TestData(" ", false, true),
        TestData(" \r\n\v\t", false, true),
        TestData(" \r\n\v\tFRANTISEK", true, false),
        TestData("-FRANTISEK", true, false),
        TestData("FRANTISEK-", true, false),
        TestData("-FRANTISEK-", true, false),
        TestData("FRANTI--SEK", true, false),
        TestData(std::string(Fred::Backend::GeneralCheck::MojeId::USERNAME_LENGTH_LIMIT + 1, 'A'), true, false),
        TestData("FRANTISEK", true, true),
        TestData("F-ANT-SEK", true, true),
        TestData("Frant1Sek", true, true),
        TestData("A", true, true),
        TestData("Z", true, true),
        TestData("a", true, true),
        TestData("z", true, true),
        TestData("0", true, true),
        TestData("9", true, true),
        TestData(std::string(Fred::Backend::GeneralCheck::MojeId::USERNAME_LENGTH_LIMIT, 'A'), true, true)
    };
    static const TestData *const data_end = data + (sizeof(data) / sizeof(*data));
    BOOST_ASSERT(( (sizeof(data) / sizeof(*data)) == 19 ));
    for (const TestData *data_ptr = data; data_ptr < data_end; ++data_ptr) {
        LibFred::OperationContextCreator ctx;
        contact[MAIN].handle = data_ptr->username;
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success() == data_ptr->success());
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success() == data_ptr->success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::absent == !data_ptr->present);
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::invalid == !data_ptr->valid);
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success());

        try {
            const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
            BOOST_CHECK(data_ptr->success());
            BOOST_CHECK(result.success());
        }
        catch (const Fred::Backend::check_contact_name&) {
            BOOST_FAIL("Fred::check_contact_name failure");
        }
        catch (const Fred::Backend::check_contact_mailing_address&) {
            BOOST_FAIL("Fred::check_contact_mailing_address failure");
        }
        catch (const Fred::Backend::check_contact_email_presence&) {
            BOOST_FAIL("Fred::check_contact_email_presence failure");
        }
        catch (const Fred::Backend::check_contact_email_validity&) {
            BOOST_FAIL("Fred::check_contact_email_validity failure");
        }
        catch (const Fred::Backend::check_contact_phone_presence&) {
            BOOST_FAIL("Fred::check_contact_phone_presence failure");
        }
        catch (const Fred::Backend::check_contact_phone_validity&) {
            BOOST_FAIL("Fred::check_contact_phone_validity failure");
        }
        catch (const Fred::Backend::check_contact_fax_validity&) {
            BOOST_FAIL("Fred::check_contact_fax_validity failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_birthday&) {
            BOOST_FAIL("Fred::MojeId::check_contact_birthday failure");
        }
        catch (const Fred::Backend::check_contact_email_availability&) {
            BOOST_FAIL("Fred::check_contact_email_availability failure");
        }
        catch (const Fred::Backend::check_contact_phone_availability&) {
            BOOST_FAIL("Fred::check_contact_phone_availability failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_username &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            const ::size_t idx = data_ptr - data;
            switch (idx)
            {
            case 0 ... 2:
                BOOST_CHECK(e.absent);
                BOOST_CHECK(!e.invalid);
                break;
            case 3 ... 8:
                BOOST_CHECK(!e.absent);
                BOOST_CHECK(e.invalid);
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
    }
}

BOOST_AUTO_TEST_CASE(check_contact_birthday)
{
    struct TestData
    {
        TestData(const char *t, const char *s, bool p, bool v)
        :   ssntype(t == NULL ? Nullable< std::string >() : Nullable< std::string >(t)),
            ssn(s == NULL ? Nullable< std::string >() : Nullable< std::string >(s)),
            present(p),
            valid(v) { }
        const Nullable< std::string > ssntype;
        const Nullable< std::string > ssn;
        const bool present:1;
        const bool valid:1;
        bool success()const { return present && valid; }
    };
    static const TestData data[] =
    {
        TestData(NULL,         NULL,         false, true),
        TestData("cosi kdesi", NULL,         false, true),
        TestData(NULL,         "bla bla",    false, true),
        TestData("cosi kdesi", "bla bla",    false, true),
        TestData("BIRTHDAY",   "1980-12-01", true,  true),
        TestData("BIRTHDAY",   "1980-12-31", true,  true),
        TestData("BIRTHDAY",   "1980-02-29", true,  true),
        TestData("BIRTHDAY",   "1981-02-28", true,  true),
        TestData("BIRTHDAY",   "1980-13-01", true,  false),
        TestData("BIRTHDAY",   "1980-02-30", true,  false),
        TestData("BIRTHDAY",   "1981-02-29", true,  false)
    };
    static const TestData *const data_end = data + (sizeof(data) / sizeof(*data));
    BOOST_ASSERT(( (sizeof(data) / sizeof(*data)) == 11 ));
    for (const TestData *data_ptr = data; data_ptr < data_end; ++data_ptr) {
        LibFred::OperationContextCreator ctx;
        contact[MAIN].ssntype = data_ptr->ssntype;
        contact[MAIN].ssn = data_ptr->ssn;
        const SumCheck result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
        BOOST_CHECK(result.success() == data_ptr->success());
        BOOST_CHECK(result.Fred::Backend::check_contact_name::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_mailing_address::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_email_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_presence::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_validity::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_fax_validity::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_username::success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::success() == data_ptr->success());
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::absent    == !data_ptr->present);
        BOOST_CHECK(result.Fred::Backend::MojeId::check_contact_birthday::invalid   == !data_ptr->valid);
        BOOST_CHECK(result.Fred::Backend::check_contact_email_availability::success());
        BOOST_CHECK(result.Fred::Backend::check_contact_phone_availability::success());

        try {
            const SumCheckWithException result(Fred::Backend::MojeId::make_args(contact[MAIN]), Fred::Backend::MojeId::make_args(contact[MAIN], ctx));
            BOOST_CHECK(data_ptr->success());
            BOOST_CHECK(result.success());
        }
        catch (const Fred::Backend::check_contact_name&) {
            BOOST_FAIL("Fred::check_contact_name failure");
        }
        catch (const Fred::Backend::check_contact_mailing_address&) {
            BOOST_FAIL("Fred::check_contact_mailing_address failure");
        }
        catch (const Fred::Backend::check_contact_email_presence&) {
            BOOST_FAIL("Fred::check_contact_email_presence failure");
        }
        catch (const Fred::Backend::check_contact_email_validity&) {
            BOOST_FAIL("Fred::check_contact_email_validity failure");
        }
        catch (const Fred::Backend::check_contact_phone_presence&) {
            BOOST_FAIL("Fred::check_contact_phone_presence failure");
        }
        catch (const Fred::Backend::check_contact_phone_validity&) {
            BOOST_FAIL("Fred::check_contact_phone_validity failure");
        }
        catch (const Fred::Backend::check_contact_fax_validity&) {
            BOOST_FAIL("Fred::check_contact_fax_validity failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_username&) {
            BOOST_FAIL("Fred::MojeId::check_contact_birthday failure");
        }
        catch (const Fred::Backend::check_contact_email_availability&) {
            BOOST_FAIL("Fred::check_contact_email_availability failure");
        }
        catch (const Fred::Backend::check_contact_phone_availability&) {
            BOOST_FAIL("Fred::check_contact_phone_availability failure");
        }
        catch (const Fred::Backend::MojeId::check_contact_birthday &e) {
            BOOST_CHECK(!data_ptr->success());
            BOOST_CHECK(!e.success());
            const ::size_t idx = data_ptr - data;
            switch (idx)
            {
            case 0 ... 3:
                BOOST_CHECK(e.absent);
                BOOST_CHECK(!e.invalid);
                BOOST_CHECK(data_ptr->ssntype.isnull() ||
                            (data_ptr->ssntype.get_value() != "BIRTHDAY"));
                break;
            case 8 ... 10:
                BOOST_CHECK(!e.absent);
                BOOST_CHECK(e.invalid);
                BOOST_CHECK(!data_ptr->ssntype.isnull());
                BOOST_CHECK(!data_ptr->ssntype.isnull() && (data_ptr->ssntype.get_value() == "BIRTHDAY"));
                BOOST_CHECK(!data_ptr->ssn.isnull());
                break;
            default:
                BOOST_FAIL("test " << idx << " may be valid");
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END();//TestContactCheckers
