/*
 * Copyright (C) 2014-2019  CZ.NIC, z. s. p. o.
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
#include "libfred/object/object_state.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/get_object_state_descriptions.hh"
#include "libfred/object_state/get_object_states.hh"
#include "src/deprecated/libfred/object_state/object_state_name.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/check_contact.hh"
#include "libfred/registrable_object/contact/copy_contact.hh"
#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/delete_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/contact/merge_contact.hh"
#include "libfred/registrable_object/contact/update_contact.hh"
#include "util/random/char_set/char_set.hh"
#include "util/random/random.hh"
#include "util/util.hh"
#include "test/libfred/enum_to_db_handle_conversion.hh"
#include "test/setup/fixtures.hh"

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/test/unit_test.hpp>

#include <algorithm>
#include <string>

BOOST_FIXTURE_TEST_SUITE(TestObjectState, Test::instantiate_db_template)

const std::string server_name = "test-object-state";


struct test_contact_fixture_8470af40b863415588b78b1fb1782e7e : public Test::instantiate_db_template
{
    std::string registrar_handle;
    std::string xmark;
    std::string test_contact_handle;

    test_contact_fixture_8470af40b863415588b78b1fb1782e7e()
    :xmark(Random::Generator().get_seq(Random::CharSet::digits(), 6))
    , test_contact_handle(std::string("TEST-CONTACT-HANDLE")+xmark)
    {
        ::LibFred::OperationContextCreator ctx;
        registrar_handle = static_cast<std::string>(ctx.get_conn().exec(
                "SELECT handle FROM registrar WHERE system = TRUE ORDER BY id LIMIT 1")[0][0]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar

        ::LibFred::Contact::PlaceAddress place;
        place.street1 = std::string("STR1") + xmark;
        place.city = "Praha";
        place.postalcode = "11150";
        place.country = "CZ";
        ::LibFred::CreateContact(test_contact_handle,registrar_handle).set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_name(std::string("TEST-CONTACT NAME")+xmark)
            .set_disclosename(true)
            .set_place(place)
            .set_discloseaddress(true)
            .exec(ctx);

        ctx.commit_transaction();//commit fixture
    }
    ~test_contact_fixture_8470af40b863415588b78b1fb1782e7e()
    {}
};


BOOST_FIXTURE_TEST_CASE(get_object_states, test_contact_fixture_8470af40b863415588b78b1fb1782e7e )
{
    ::LibFred::OperationContextCreator ctx;
    ::LibFred::InfoContactOutput contact_info1 = ::LibFred::InfoContactByHandle(test_contact_handle).exec(ctx);
    std::vector<::LibFred::ObjectStateData> states;
    states = ::LibFred::GetObjectStates(contact_info1.info_contact_data.id).exec(ctx);
    BOOST_CHECK(states.empty());
    ::LibFred::CreateObjectStateRequestId(contact_info1.info_contact_data.id,
        Util::set_of<std::string>(::LibFred::ObjectState::MOJEID_CONTACT)).exec(ctx);
    ::LibFred::PerformObjectStateRequest().set_object_id(contact_info1.info_contact_data.id).exec(ctx);

    states = ::LibFred::GetObjectStates(contact_info1.info_contact_data.id).exec(ctx);
    BOOST_CHECK(states.at(0).state_name == ::LibFred::ObjectState::MOJEID_CONTACT);
}

/**
 * @namespace ObjectStateDescriptionWithComparison
 */

BOOST_AUTO_TEST_SUITE(ObjectStateDescriptionWithComparison)

struct state_desc_less
{
    bool operator()(const ::LibFred::ObjectStateDescription& lhs, const ::LibFred::ObjectStateDescription& rhs) const
    {
        std::string l = boost::lexical_cast<std::string>(lhs.id);
        l += lhs.handle;
        l += lhs.description;

        std::string r = boost::lexical_cast<std::string>(rhs.id);
        r += rhs.handle;
        r += rhs.description;

        return l < r;
    }
};

bool state_desc_equal(const ::LibFred::ObjectStateDescription& lhs, const ::LibFred::ObjectStateDescription& rhs)
{
    return lhs.id == rhs.id
    && lhs.handle == rhs.handle
    && lhs.description == rhs.description;
}

struct object_state_description_fixture : public Test::instantiate_db_template
{
    std::vector<::LibFred::ObjectStateDescription> state_desc_en_all_vect;
    std::vector<::LibFred::ObjectStateDescription> state_desc_en_contact_vect;
    std::vector<::LibFred::ObjectStateDescription> state_desc_en_external_vect;
    object_state_description_fixture()
    : state_desc_en_all_vect(Util::vector_of<::LibFred::ObjectStateDescription>
            (::LibFred::ObjectStateDescription(1,"serverDeleteProhibited", "Deletion forbidden"))
            (::LibFred::ObjectStateDescription(2,"serverRenewProhibited", "Registration renewal forbidden"))
            (::LibFred::ObjectStateDescription(3,"serverTransferProhibited", "Sponsoring registrar change forbidden"))
            (::LibFred::ObjectStateDescription(4,"serverUpdateProhibited", "Update forbidden"))
            (::LibFred::ObjectStateDescription(5,"serverOutzoneManual", "The domain is administratively kept out of zone"))
            (::LibFred::ObjectStateDescription(6,"serverInzoneManual", "The domain is administratively kept in zone"))
            (::LibFred::ObjectStateDescription(7,"serverBlocked", "Domain blocked"))
            (::LibFred::ObjectStateDescription(8,"expirationWarning", "The domain expires in 30 days"))
            (::LibFred::ObjectStateDescription(9,"expired", "Domain expired"))
            (::LibFred::ObjectStateDescription(10,"unguarded", "The domain is 30 days after expiration"))
            (::LibFred::ObjectStateDescription(11,"validationWarning1", "The domain validation expires in 30 days"))
            (::LibFred::ObjectStateDescription(12,"validationWarning2", "The domain validation expires in 15 days"))
            (::LibFred::ObjectStateDescription(13,"notValidated", "Domain not validated"))
            (::LibFred::ObjectStateDescription(14,"nssetMissing", "The domain doesn't have associated nsset"))
            (::LibFred::ObjectStateDescription(15,"outzone", "The domain isn't generated in the zone"))
            (::LibFred::ObjectStateDescription(16,"linked", "Has relation to other records in the registry"))
            (::LibFred::ObjectStateDescription(17,"deleteCandidate", "To be deleted"))
            (::LibFred::ObjectStateDescription(18,"serverRegistrantChangeProhibited", "Registrant change forbidden"))
            (::LibFred::ObjectStateDescription(19,"deleteWarning", "The domain will be deleted in 11 days"))
            (::LibFred::ObjectStateDescription(20,"outzoneUnguarded", "The domain is out of zone after 30 days in expiration state"))
            (::LibFred::ObjectStateDescription(21,"conditionallyIdentifiedContact", "Contact is conditionally identified"))
            (::LibFred::ObjectStateDescription(22,"identifiedContact", "Contact is identified"))
            (::LibFred::ObjectStateDescription(23,"validatedContact", "Contact is validated"))
            (::LibFred::ObjectStateDescription(24,"mojeidContact", "MojeID contact"))
            (::LibFred::ObjectStateDescription(25,"contactPassedManualVerification", "Contact has been verified by CZ.NIC customer support"))
            (::LibFred::ObjectStateDescription(26,"contactInManualVerification", "Contact is being verified by CZ.NIC customer support"))
            (::LibFred::ObjectStateDescription(27,"contactFailedManualVerification", "Contact has failed the verification by CZ.NIC customer support"))
            (::LibFred::ObjectStateDescription(28,"outzoneUnguardedWarning", "The domain is to be out of zone soon."))
        )
    , state_desc_en_contact_vect(Util::vector_of<::LibFred::ObjectStateDescription>
            (::LibFred::ObjectStateDescription(1,"serverDeleteProhibited", "Deletion forbidden"))
            (::LibFred::ObjectStateDescription(3,"serverTransferProhibited", "Sponsoring registrar change forbidden"))
            (::LibFred::ObjectStateDescription(4,"serverUpdateProhibited", "Update forbidden"))
            (::LibFred::ObjectStateDescription(7,"serverBlocked", "Domain blocked"))
            (::LibFred::ObjectStateDescription(16,"linked", "Has relation to other records in the registry"))
            (::LibFred::ObjectStateDescription(17,"deleteCandidate", "To be deleted"))
            (::LibFred::ObjectStateDescription(21,"conditionallyIdentifiedContact", "Contact is conditionally identified"))
            (::LibFred::ObjectStateDescription(22,"identifiedContact", "Contact is identified"))
            (::LibFred::ObjectStateDescription(23,"validatedContact", "Contact is validated"))
            (::LibFred::ObjectStateDescription(24,"mojeidContact", "MojeID contact"))
            (::LibFred::ObjectStateDescription(25,"contactPassedManualVerification", "Contact has been verified by CZ.NIC customer support"))
            (::LibFred::ObjectStateDescription(26,"contactInManualVerification", "Contact is being verified by CZ.NIC customer support"))
            (::LibFred::ObjectStateDescription(27,"contactFailedManualVerification", "Contact has failed the verification by CZ.NIC customer support"))
        )
    , state_desc_en_external_vect(Util::vector_of<::LibFred::ObjectStateDescription>
            (::LibFred::ObjectStateDescription(1,"serverDeleteProhibited", "Deletion forbidden"))
            (::LibFred::ObjectStateDescription(2,"serverRenewProhibited", "Registration renewal forbidden"))
            (::LibFred::ObjectStateDescription(3,"serverTransferProhibited", "Sponsoring registrar change forbidden"))
            (::LibFred::ObjectStateDescription(4,"serverUpdateProhibited", "Update forbidden"))
            (::LibFred::ObjectStateDescription(5,"serverOutzoneManual", "The domain is administratively kept out of zone"))
            (::LibFred::ObjectStateDescription(6,"serverInzoneManual", "The domain is administratively kept in zone"))
            (::LibFred::ObjectStateDescription(7,"serverBlocked", "Domain blocked"))
            (::LibFred::ObjectStateDescription(9,"expired", "Domain expired"))
            (::LibFred::ObjectStateDescription(13,"notValidated", "Domain not validated"))
            (::LibFred::ObjectStateDescription(15,"outzone", "The domain isn't generated in the zone"))
            (::LibFred::ObjectStateDescription(16,"linked", "Has relation to other records in the registry"))
            (::LibFred::ObjectStateDescription(17,"deleteCandidate", "To be deleted"))
            (::LibFred::ObjectStateDescription(18,"serverRegistrantChangeProhibited", "Registrant change forbidden"))
            (::LibFred::ObjectStateDescription(21,"conditionallyIdentifiedContact", "Contact is conditionally identified"))
            (::LibFred::ObjectStateDescription(22,"identifiedContact", "Contact is identified"))
            (::LibFred::ObjectStateDescription(23,"validatedContact", "Contact is validated"))
            (::LibFred::ObjectStateDescription(24,"mojeidContact", "MojeID contact"))
            (::LibFred::ObjectStateDescription(25,"contactPassedManualVerification", "Contact has been verified by CZ.NIC customer support"))
            (::LibFred::ObjectStateDescription(26,"contactInManualVerification", "Contact is being verified by CZ.NIC customer support"))
            (::LibFred::ObjectStateDescription(27,"contactFailedManualVerification", "Contact has failed the verification by CZ.NIC customer support"))
        )
    {
        ::LibFred::OperationContextCreator ctx;

        ctx.get_conn().exec(
            "TRUNCATE TABLE enum_object_states CASCADE; "
            "TRUNCATE TABLE enum_object_states_desc CASCADE; "
            "INSERT INTO enum_object_states VALUES (01,'serverDeleteProhibited','{1,2,3}','t','t', 32768); "
            "INSERT INTO enum_object_states VALUES (02,'serverRenewProhibited','{3}','t','t', 16384); "
            "INSERT INTO enum_object_states VALUES (03,'serverTransferProhibited','{1,2,3}','t','t', 4096); "
            "INSERT INTO enum_object_states VALUES (04,'serverUpdateProhibited','{1,2,3}','t','t', 2048); "
            "INSERT INTO enum_object_states VALUES (05,'serverOutzoneManual','{3}','t','t', 128); "
            "INSERT INTO enum_object_states VALUES (06,'serverInzoneManual','{3}','t','t', 256); "
            "INSERT INTO enum_object_states VALUES (07,'serverBlocked','{1,3}','t','t', 65536); "
            "INSERT INTO enum_object_states VALUES (08,'expirationWarning','{3}','f','f', NULL); "
            "INSERT INTO enum_object_states VALUES (09,'expired','{3}','f','t', 2); "
            "INSERT INTO enum_object_states VALUES (10,'unguarded','{3}','f','f', NULL); "
            "INSERT INTO enum_object_states VALUES (11,'validationWarning1','{3}','f','f', NULL); "
            "INSERT INTO enum_object_states VALUES (12,'validationWarning2','{3}','f','f', NULL); "
            "INSERT INTO enum_object_states VALUES (13,'notValidated','{3}','f','t', 512); "
            "INSERT INTO enum_object_states VALUES (14,'nssetMissing','{3}','f','f', NULL); "
            "INSERT INTO enum_object_states VALUES (15,'outzone','{3}','f','t', 8); "
            "INSERT INTO enum_object_states VALUES (16,'linked','{1,2}','f','t', 1024); "
            "INSERT INTO enum_object_states VALUES (17,'deleteCandidate','{1,2,3}','f','t', NULL); "
            "INSERT INTO enum_object_states VALUES (18,'serverRegistrantChangeProhibited','{3}','t','t', 8192); "
            "INSERT INTO enum_object_states VALUES (19,'deleteWarning','{3}','f','f', NULL); "
            "INSERT INTO enum_object_states VALUES (20,'outzoneUnguarded','{3}','f','f', NULL); "

            "INSERT INTO enum_object_states_desc VALUES (01,'CS','Není povoleno smazání'); "
            "INSERT INTO enum_object_states_desc VALUES (01,'EN','Deletion forbidden'); "
            "INSERT INTO enum_object_states_desc VALUES (02,'CS','Není povoleno prodloužení registrace objektu'); "
            "INSERT INTO enum_object_states_desc VALUES (02,'EN','Registration renewal forbidden'); "
            "INSERT INTO enum_object_states_desc VALUES (03,'CS','Není povolena změna určeného registrátora'); "
            "INSERT INTO enum_object_states_desc VALUES (03,'EN','Sponsoring registrar change forbidden'); "
            "INSERT INTO enum_object_states_desc VALUES (04,'CS','Není povolena změna údajů'); "
            "INSERT INTO enum_object_states_desc VALUES (04,'EN','Update forbidden'); "
            "INSERT INTO enum_object_states_desc VALUES (05,'CS','Doména je administrativně vyřazena ze zóny'); "
            "INSERT INTO enum_object_states_desc VALUES (05,'EN','The domain is administratively kept out of zone'); "
            "INSERT INTO enum_object_states_desc VALUES (06,'CS','Doména je administrativně zařazena do zóny'); "
            "INSERT INTO enum_object_states_desc VALUES (06,'EN','The domain is administratively kept in zone'); "
            "INSERT INTO enum_object_states_desc VALUES (07,'CS','Doména je blokována'); "
            "INSERT INTO enum_object_states_desc VALUES (07,'EN','Domain blocked'); "
            "INSERT INTO enum_object_states_desc VALUES (08,'CS','Doména expiruje do 30 dní'); "
            "INSERT INTO enum_object_states_desc VALUES (08,'EN','The domain expires in 30 days'); "
            "INSERT INTO enum_object_states_desc VALUES (09,'CS','Doména je po expiraci'); "
            "INSERT INTO enum_object_states_desc VALUES (09,'EN','Domain expired'); "
            "INSERT INTO enum_object_states_desc VALUES (10,'CS','Doména je 30 dnů po expiraci'); "
            "INSERT INTO enum_object_states_desc VALUES (10,'EN','The domain is 30 days after expiration'); "
            "INSERT INTO enum_object_states_desc VALUES (11,'CS','Validace domény skončí za 30 dní'); "
            "INSERT INTO enum_object_states_desc VALUES (11,'EN','The domain validation expires in 30 days'); "
            "INSERT INTO enum_object_states_desc VALUES (12,'CS','Validace domény skončí za 15 dní'); "
            "INSERT INTO enum_object_states_desc VALUES (12,'EN','The domain validation expires in 15 days'); "
            "INSERT INTO enum_object_states_desc VALUES (13,'CS','Doména není validována'); "
            "INSERT INTO enum_object_states_desc VALUES (13,'EN','Domain not validated'); "
            "INSERT INTO enum_object_states_desc VALUES (14,'CS','Doména nemá přiřazen nsset'); "
            "INSERT INTO enum_object_states_desc VALUES (14,'EN','The domain doesn''t have associated nsset'); "
            "INSERT INTO enum_object_states_desc VALUES (15,'CS','Doména není generována do zóny'); "
            "INSERT INTO enum_object_states_desc VALUES (15,'EN','The domain isn''t generated in the zone'); "
            "INSERT INTO enum_object_states_desc VALUES (16,'CS','Je navázán na další záznam v registru'); "
            "INSERT INTO enum_object_states_desc VALUES (16,'EN','Has relation to other records in the registry'); "
            "INSERT INTO enum_object_states_desc VALUES (17,'CS','Určeno ke zrušení'); "
            "INSERT INTO enum_object_states_desc VALUES (17,'EN','To be deleted'); "
            "INSERT INTO enum_object_states_desc VALUES (18,'CS','Není povolena změna držitele'); "
            "INSERT INTO enum_object_states_desc VALUES (18,'EN','Registrant change forbidden'); "
            "INSERT INTO enum_object_states_desc VALUES (19,'CS','Registrace domény bude zrušena za 11 dní'); "
            "INSERT INTO enum_object_states_desc VALUES (19,'EN','The domain will be deleted in 11 days'); "
            "INSERT INTO enum_object_states_desc VALUES (20,'CS','Doména vyřazena ze zóny po 30 dnech od expirace'); "
            "INSERT INTO enum_object_states_desc VALUES (20,'EN','The domain is out of zone after 30 days in expiration state'); "

            "INSERT INTO enum_object_states VALUES (21,'conditionallyIdentifiedContact','{1}','t','t', 32); "
            "INSERT INTO enum_object_states VALUES (22,'identifiedContact','{1}','t','t', 16); "
            "INSERT INTO enum_object_states VALUES (23,'validatedContact','{1}','t','t', 64); "
            "INSERT INTO enum_object_states VALUES (24,'mojeidContact','{1}','t','t', 4); "

            "INSERT INTO enum_object_states_desc VALUES (21, 'CS', 'Kontakt je částečně identifikován'); "
            "INSERT INTO enum_object_states_desc VALUES (21, 'EN', 'Contact is conditionally identified'); "
            "INSERT INTO enum_object_states_desc VALUES (22, 'CS', 'Kontakt je identifikován'); "
            "INSERT INTO enum_object_states_desc VALUES (22, 'EN', 'Contact is identified'); "
            "INSERT INTO enum_object_states_desc VALUES (23, 'CS', 'Kontakt je validován'); "
            "INSERT INTO enum_object_states_desc VALUES (23, 'EN', 'Contact is validated'); "
            "INSERT INTO enum_object_states_desc VALUES (24, 'CS', 'MojeID kontakt'); "
            "INSERT INTO enum_object_states_desc VALUES (24, 'EN', 'MojeID contact'); "

            "INSERT INTO enum_object_states VALUES (26,'contactInManualVerification','{1}','t','t', NULL); "
            "INSERT INTO enum_object_states VALUES (25,'contactPassedManualVerification','{1}','t','t', NULL); "
            "INSERT INTO enum_object_states VALUES (27,'contactFailedManualVerification','{1}','t','t', NULL); "

            "INSERT INTO enum_object_states_desc VALUES (26, 'CS', 'Kontakt je ověřován zákaznickou podporou CZ.NIC'); "
            "INSERT INTO enum_object_states_desc VALUES (25, 'CS', 'Kontakt byl ověřen zákaznickou podporou CZ.NIC'); "
            "INSERT INTO enum_object_states_desc VALUES (27, 'CS', 'Ověření kontaktu zákaznickou podporou bylo neúspěšné'); "

            "INSERT INTO enum_object_states_desc VALUES (26, 'EN', 'Contact is being verified by CZ.NIC customer support'); "
            "INSERT INTO enum_object_states_desc VALUES (25, 'EN', 'Contact has been verified by CZ.NIC customer support'); "
            "INSERT INTO enum_object_states_desc VALUES (27, 'EN', 'Contact has failed the verification by CZ.NIC customer support'); "

            "INSERT INTO enum_object_states VALUES (28,'outzoneUnguardedWarning','{3}','f','f', NULL); "

            "INSERT INTO enum_object_states_desc VALUES (28,'CS','Doména bude brzy vyřazena ze zóny.'); "
            "INSERT INTO enum_object_states_desc VALUES (28,'EN','The domain is to be out of zone soon.'); "
        );
        ctx.commit_transaction();
    }
};

void check_object_state_desc_data(std::vector<::LibFred::ObjectStateDescription> test_osd,
    std::vector<::LibFred::ObjectStateDescription> fixture_osd)
{
    BOOST_REQUIRE(test_osd.size() == fixture_osd.size());

    std::sort(test_osd.begin(), test_osd.end(), state_desc_less());
    std::sort(fixture_osd.begin(), fixture_osd.end(), state_desc_less());

    BOOST_CHECK(
        std::mismatch(test_osd.begin(), test_osd.end(), fixture_osd.begin(), state_desc_equal)
        == std::make_pair(test_osd.end(), fixture_osd.end()));
}


/**
 * test ::LibFred::Object_State conversion functions
 */
BOOST_AUTO_TEST_CASE(fred_object_state_conversions)
{
    ::LibFred::OperationContextCreator ctx;
    static const char *const sql = "SELECT name FROM enum_object_states";
    enum_to_db_handle_conversion_test< ::LibFred::Object_State, 28 >(ctx, sql);
}

BOOST_FIXTURE_TEST_CASE(get_object_state_descriptions, object_state_description_fixture)
{
    ::LibFred::OperationContextCreator ctx;
    check_object_state_desc_data(::LibFred::GetObjectStateDescriptions("EN").exec(ctx), state_desc_en_all_vect);
    check_object_state_desc_data(::LibFred::GetObjectStateDescriptions("EN").set_object_type("contact").exec(ctx), state_desc_en_contact_vect);
    check_object_state_desc_data(::LibFred::GetObjectStateDescriptions("EN").set_external().exec(ctx), state_desc_en_external_vect);
}

BOOST_AUTO_TEST_SUITE_END();//ObjectStateDescriptionWithComparison

BOOST_AUTO_TEST_SUITE_END();//TestObjectState
