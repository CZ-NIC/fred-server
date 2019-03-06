/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
 */

#include <boost/test/unit_test.hpp>
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/notifier/util.hh"
#include "test/libfred/notifier/fixture_data.hh"

#include "libfred/notifier/gather_email_data/gather_email_content.hh"


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailContent)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(Update)

template<typename T_has_contact>struct has_contact_empty_update : T_has_contact {

    const unsigned long long logd_request_id;
    const unsigned long long new_historyid;

    has_contact_empty_update()
    :   logd_request_id(12345),
        new_historyid(::LibFred::UpdateContactByHandle(T_has_contact::contact.handle, T_has_contact::registrar.handle).set_logd_request_id(logd_request_id).exec(T_has_contact::ctx))
    { }
};

BOOST_FIXTURE_TEST_CASE(test_minimal_update1, has_contact_empty_update<has_empty_contact>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "1";
    etalon["handle"] = contact.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::contact, Notification::updated),
                registrar.id,
                new_historyid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_minimal_update2, has_contact_empty_update<has_full_contact>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "1";
    etalon["handle"] = contact.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::contact, Notification::updated),
                registrar.id,
                new_historyid,
                input_svtrid
            )
        )
    );
}

template<typename T_has_contact> struct has_contact_big_update : T_has_contact {
    const ::LibFred::InfoContactData new_contact_data;

    has_contact_big_update()
    :   new_contact_data(
            ::LibFred::InfoContactHistoryByHistoryid(
                /*  TODO
                    , const Optional<Nullable<bool> >& domain_expiration_letter_flag
                    , const Optional<unsigned long long> logd_request_id
                 */

                ::LibFred::UpdateContactByHandle(T_has_contact::contact.handle, T_has_contact::contact.create_registrar_handle)
                    .set_authinfo(T_has_contact::contact.authinfopw + "42")
                    .set_name(T_has_contact::contact.name.get_value_or("John Smith") + " Jr.")
                    .set_place(
                        ::LibFred::Contact::PlaceAddress(
                            "NEW_uliceA 1",
                            "NEW_uliceA 2",
                            "NEW_uliceA 3",
                            "NEW_mestoA",
                            "NEW_krajA",
                            "NEW_PSCA",
                            "CZ"
                        )
                    )
                    .template /* "the single most obscure use of the template keyword" */ set_address<::LibFred::ContactAddressType::MAILING>(
                        ::LibFred::ContactAddress(
                            Optional<std::string>(),
                            "NEW_uliceB 1",
                            "NEW_uliceB 2",
                            "NEW_uliceB 3",
                            "NEW_mestoB",
                            "NEW_krajB",
                            "NEW_PSCB",
                            "CZ"
                        )
                    )
                    .template /* "the single most obscure use of the template keyword" */ set_address<::LibFred::ContactAddressType::SHIPPING>(
                        ::LibFred::ContactAddress(
                            Optional<std::string>(),
                            "NEW_uliceC 1",
                            "NEW_uliceC 2",
                            "NEW_uliceC 3",
                            "NEW_mestoC",
                            "NEW_krajC",
                            "NEW_PSCC",
                            "CZ"
                        )
                    )
                    .set_organization(T_has_contact::contact.organization.get_value_or("MegaCorp") + " Ltd.")
                    .set_telephone(T_has_contact::contact.telephone.get_value_or("+123 456 789 012") + "1")
                    .set_fax(T_has_contact::contact.fax.get_value_or("+123 456 789 012") + "1")
                    .set_email("a" + T_has_contact::contact.email.get_value_or("abc@def.cx"))
                    .set_notifyemail("a" + T_has_contact::contact.notifyemail.get_value_or("abc.notify@def.cx"))
                    .set_vat(T_has_contact::contact.vat.get_value_or("123456") + "1")
                    .set_personal_id(::LibFred::PersonalIdUnion::get_RC("19891231/1234"))
                    .set_disclosename(          !T_has_contact::contact.disclosename)
                    .set_discloseorganization(  !T_has_contact::contact.discloseorganization)
                    .set_discloseaddress(       !T_has_contact::contact.discloseaddress)
                    .set_disclosetelephone(     !T_has_contact::contact.disclosetelephone)
                    .set_disclosefax(           !T_has_contact::contact.disclosefax)
                    .set_discloseemail(         !T_has_contact::contact.discloseemail)
                    .set_disclosevat(           !T_has_contact::contact.disclosevat)
                    .set_discloseident(         !T_has_contact::contact.discloseident)
                    .set_disclosenotifyemail(   !T_has_contact::contact.disclosenotifyemail)
                    .exec(T_has_contact::ctx)
            ).exec(T_has_contact::ctx).info_contact_data
        )
    { }
};

BOOST_FIXTURE_TEST_CASE(test_big_update_from_empty_data, has_contact_big_update<has_empty_contact>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "1";
    etalon["handle"] = contact.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["changes.contact.address.permanent"]         = "1";
    etalon["changes.contact.address.permanent.old"]     = "";
    etalon["changes.contact.address.permanent.new"]     =
        new_contact_data.place.get_value().street1 + ", " +
        new_contact_data.place.get_value().street2.get_value() + ", " +
        new_contact_data.place.get_value().street3.get_value() + ", " +
        new_contact_data.place.get_value().stateorprovince.get_value() + ", " +
        new_contact_data.place.get_value().postalcode + ", " +
        new_contact_data.place.get_value().city + ", " +
        new_contact_data.place.get_value().country;

    etalon["changes.contact.address.mailing"]         = "1";
    etalon["changes.contact.address.mailing.old"]     = "";
    etalon["changes.contact.address.mailing.new"]     =
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).street1 + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).street2.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).street3.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).stateorprovince.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).postalcode + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).city + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).country;

    etalon["changes.contact.address.shipping"]         = "1";
    etalon["changes.contact.address.shipping.old"]     = "";
    etalon["changes.contact.address.shipping.new"]     =
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).street1 + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).street2.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).street3.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).stateorprovince.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).postalcode + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).city + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).country;

    etalon["changes.object.authinfo"]                   = "1";
    etalon["changes.object.authinfo.old"]               = contact.authinfopw;
    etalon["changes.object.authinfo.new"]               = new_contact_data.authinfopw;

    etalon["changes.contact.name"]                      = "1";
    etalon["changes.contact.name.old"]                  = "";
    etalon["changes.contact.name.new"]                  = new_contact_data.name.get_value();

    etalon["changes.contact.org"]                       = "1";
    etalon["changes.contact.org.old"]                   = "";
    etalon["changes.contact.org.new"]                   = new_contact_data.organization.get_value();

    etalon["changes.contact.telephone"]                 = "1";
    etalon["changes.contact.telephone.old"]             = "";
    etalon["changes.contact.telephone.new"]             = new_contact_data.telephone.get_value();

    etalon["changes.contact.fax"]                       = "1";
    etalon["changes.contact.fax.old"]                   = "";
    etalon["changes.contact.fax.new"]                   = new_contact_data.fax.get_value();

    etalon["changes.contact.email"]                     = "1";
    etalon["changes.contact.email.old"]                 = "";
    etalon["changes.contact.email.new"]                 = new_contact_data.email.get_value();

    etalon["changes.contact.notify_email"]              = "1";
    etalon["changes.contact.notify_email.old"]          = "";
    etalon["changes.contact.notify_email.new"]          = new_contact_data.notifyemail.get_value();

    etalon["changes.contact.ident_type"]                = "1";
    etalon["changes.contact.ident_type.old"]            = "";
    etalon["changes.contact.ident_type.new"]            = new_contact_data.ssntype.get_value();

    etalon["changes.contact.ident"]                     = "1";
    etalon["changes.contact.ident.old"]                 = "";
    etalon["changes.contact.ident.new"]                 = new_contact_data.ssn.get_value();

    etalon["changes.contact.vat"]                       = "1";
    etalon["changes.contact.vat.old"]                   = "";
    etalon["changes.contact.vat.new"]                   = new_contact_data.vat.get_value();

    etalon["changes.contact.disclose.name"]             = "1";
    etalon["changes.contact.disclose.name.old"]         = contact         .disclosename ? "1" : "0";
    etalon["changes.contact.disclose.name.new"]         = new_contact_data.disclosename ? "1" : "0";

    etalon["changes.contact.disclose.org"]              = "1";
    etalon["changes.contact.disclose.org.old"]          = contact         .discloseorganization ? "1" : "0";
    etalon["changes.contact.disclose.org.new"]          = new_contact_data.discloseorganization ? "1" : "0";

    etalon["changes.contact.disclose.email"]            = "1";
    etalon["changes.contact.disclose.email.old"]        = contact         .discloseemail ? "1" : "0";
    etalon["changes.contact.disclose.email.new"]        = new_contact_data.discloseemail ? "1" : "0";

    etalon["changes.contact.disclose.address"]          = "1";
    etalon["changes.contact.disclose.address.old"]      = contact         .discloseaddress ? "1" : "0";
    etalon["changes.contact.disclose.address.new"]      = new_contact_data.discloseaddress ? "1" : "0";

    etalon["changes.contact.disclose.notify_email"]     = "1";
    etalon["changes.contact.disclose.notify_email.old"] = contact         .disclosenotifyemail ? "1" : "0";
    etalon["changes.contact.disclose.notify_email.new"] = new_contact_data.disclosenotifyemail ? "1" : "0";

    etalon["changes.contact.disclose.ident"]            = "1";
    etalon["changes.contact.disclose.ident.old"]        = contact         .discloseident ? "1" : "0";
    etalon["changes.contact.disclose.ident.new"]        = new_contact_data.discloseident ? "1" : "0";

    etalon["changes.contact.disclose.vat"]              = "1";
    etalon["changes.contact.disclose.vat.old"]          = contact         .disclosevat ? "1" : "0";
    etalon["changes.contact.disclose.vat.new"]          = new_contact_data.disclosevat ? "1" : "0";

    etalon["changes.contact.disclose.telephone"]        = "1";
    etalon["changes.contact.disclose.telephone.old"]    = contact         .disclosetelephone ? "1" : "0";
    etalon["changes.contact.disclose.telephone.new"]    = new_contact_data.disclosetelephone ? "1" : "0";

    etalon["changes.contact.disclose.fax"]              = "1";
    etalon["changes.contact.disclose.fax.old"]          = contact         .disclosefax ? "1" : "0";
    etalon["changes.contact.disclose.fax.new"]          = new_contact_data.disclosefax ? "1" : "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::contact, Notification::updated),
                registrar.id,
                new_contact_data.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_big_update_from_full_data, has_contact_big_update<has_full_contact>)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "1";
    etalon["handle"] = contact.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";
    etalon["changes"] = "1";

    etalon["changes.contact.address.permanent"]         = "1";
    etalon["changes.contact.address.permanent.old"]     =
        contact.place.get_value().street1 + ", " +
        contact.place.get_value().street2.get_value() + ", " +
        contact.place.get_value().street3.get_value() + ", " +
        contact.place.get_value().stateorprovince.get_value() + ", " +
        contact.place.get_value().postalcode + ", " +
        contact.place.get_value().city + ", " +
        contact.place.get_value().country;
    etalon["changes.contact.address.permanent.new"]     =
        new_contact_data.place.get_value().street1 + ", " +
        new_contact_data.place.get_value().street2.get_value() + ", " +
        new_contact_data.place.get_value().street3.get_value() + ", " +
        new_contact_data.place.get_value().stateorprovince.get_value() + ", " +
        new_contact_data.place.get_value().postalcode + ", " +
        new_contact_data.place.get_value().city + ", " +
        new_contact_data.place.get_value().country;

    etalon["changes.contact.address.mailing"]         = "1";
    etalon["changes.contact.address.mailing.old"]     = "";
    etalon["changes.contact.address.mailing.new"]     =
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).street1 + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).street2.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).street3.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).stateorprovince.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).postalcode + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).city + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::MAILING).country;

    etalon["changes.contact.address.shipping"]         = "1";
    etalon["changes.contact.address.shipping.old"]     =
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).company_name.get_value() + ", " +
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).street1 + ", " +
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).street2.get_value() + ", " +
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).street3.get_value() + ", " +
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).stateorprovince.get_value() + ", " +
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).postalcode + ", " +
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).city + ", " +
        contact.addresses.at(::LibFred::ContactAddressType::SHIPPING).country;
    etalon["changes.contact.address.shipping.new"]     =
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).street1 + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).street2.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).street3.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).stateorprovince.get_value() + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).postalcode + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).city + ", " +
        new_contact_data.addresses.at(::LibFred::ContactAddressType::SHIPPING).country;

    etalon["changes.object.authinfo"]                   = "1";
    etalon["changes.object.authinfo.old"]               = contact         .authinfopw;
    etalon["changes.object.authinfo.new"]               = new_contact_data.authinfopw;

    etalon["changes.contact.name"]                      = "1";
    etalon["changes.contact.name.old"]                  = contact         .name.get_value();
    etalon["changes.contact.name.new"]                  = new_contact_data.name.get_value();

    etalon["changes.contact.org"]                       = "1";
    etalon["changes.contact.org.old"]                   = contact         .organization.get_value();
    etalon["changes.contact.org.new"]                   = new_contact_data.organization.get_value();

    etalon["changes.contact.telephone"]                 = "1";
    etalon["changes.contact.telephone.old"]             = contact         .telephone.get_value();
    etalon["changes.contact.telephone.new"]             = new_contact_data.telephone.get_value();

    etalon["changes.contact.fax"]                       = "1";
    etalon["changes.contact.fax.old"]                   = contact         .fax.get_value();
    etalon["changes.contact.fax.new"]                   = new_contact_data.fax.get_value();

    etalon["changes.contact.email"]                     = "1";
    etalon["changes.contact.email.old"]                 = contact         .email.get_value();
    etalon["changes.contact.email.new"]                 = new_contact_data.email.get_value();

    etalon["changes.contact.notify_email"]              = "1";
    etalon["changes.contact.notify_email.old"]          = contact         .notifyemail.get_value();
    etalon["changes.contact.notify_email.new"]          = new_contact_data.notifyemail.get_value();

    etalon["changes.contact.ident_type"]                = "1";
    etalon["changes.contact.ident_type.old"]            = contact         .ssntype.get_value();
    etalon["changes.contact.ident_type.new"]            = new_contact_data.ssntype.get_value();

    etalon["changes.contact.ident"]                     = "1";
    etalon["changes.contact.ident.old"]                 = contact         .ssn.get_value();
    etalon["changes.contact.ident.new"]                 = new_contact_data.ssn.get_value();

    etalon["changes.contact.vat"]                       = "1";
    etalon["changes.contact.vat.old"]                   = contact         .vat.get_value();
    etalon["changes.contact.vat.new"]                   = new_contact_data.vat.get_value();

    etalon["changes.contact.disclose.name"]             = "1";
    etalon["changes.contact.disclose.name.old"]         = contact         .disclosename ? "1" : "0";
    etalon["changes.contact.disclose.name.new"]         = new_contact_data.disclosename ? "1" : "0";

    etalon["changes.contact.disclose.org"]              = "1";
    etalon["changes.contact.disclose.org.old"]          = contact         .discloseorganization ? "1" : "0";
    etalon["changes.contact.disclose.org.new"]          = new_contact_data.discloseorganization ? "1" : "0";

    etalon["changes.contact.disclose.email"]            = "1";
    etalon["changes.contact.disclose.email.old"]        = contact         .discloseemail ? "1" : "0";
    etalon["changes.contact.disclose.email.new"]        = new_contact_data.discloseemail ? "1" : "0";

    etalon["changes.contact.disclose.address"]          = "1";
    etalon["changes.contact.disclose.address.old"]      = contact         .discloseaddress ? "1" : "0";
    etalon["changes.contact.disclose.address.new"]      = new_contact_data.discloseaddress ? "1" : "0";

    etalon["changes.contact.disclose.notify_email"]     = "1";
    etalon["changes.contact.disclose.notify_email.old"] = contact         .disclosenotifyemail ? "1" : "0";
    etalon["changes.contact.disclose.notify_email.new"] = new_contact_data.disclosenotifyemail ? "1" : "0";

    etalon["changes.contact.disclose.ident"]            = "1";
    etalon["changes.contact.disclose.ident.old"]        = contact         .discloseident ? "1" : "0";
    etalon["changes.contact.disclose.ident.new"]        = new_contact_data.discloseident ? "1" : "0";

    etalon["changes.contact.disclose.vat"]              = "1";
    etalon["changes.contact.disclose.vat.old"]          = contact         .disclosevat ? "1" : "0";
    etalon["changes.contact.disclose.vat.new"]          = new_contact_data.disclosevat ? "1" : "0";

    etalon["changes.contact.disclose.telephone"]        = "1";
    etalon["changes.contact.disclose.telephone.old"]    = contact         .disclosetelephone ? "1" : "0";
    etalon["changes.contact.disclose.telephone.new"]    = new_contact_data.disclosetelephone ? "1" : "0";

    etalon["changes.contact.disclose.fax"]              = "1";
    etalon["changes.contact.disclose.fax.old"]          = contact         .disclosefax ? "1" : "0";
    etalon["changes.contact.disclose.fax.new"]          = new_contact_data.disclosefax ? "1" : "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::contact, Notification::updated),
                registrar.id,
                new_contact_data.historyid,
                input_svtrid
            )
        )
    );
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
