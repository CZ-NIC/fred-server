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
#include "libfred/registrable_object/contact/place_address.hh"


BOOST_AUTO_TEST_SUITE(TestNotifier)
BOOST_AUTO_TEST_SUITE(GatherEmailContent)
BOOST_AUTO_TEST_SUITE(Contact)
BOOST_AUTO_TEST_SUITE(Create)

BOOST_FIXTURE_TEST_CASE(test_empty_create, has_empty_contact)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"]      = "1";
    etalon["handle"]    = contact.handle;
    etalon["ticket"]    = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";

    etalon["fresh.object.authinfo"]     = contact.authinfopw;
    etalon["fresh.contact.name"]        = "";
    etalon["fresh.contact.org"]         = "";
    etalon["fresh.contact.address.billing"]    = "";
    etalon["fresh.contact.address.mailing"]    = "";
    etalon["fresh.contact.address.permanent"]  = "";
    etalon["fresh.contact.address.shipping"]   = "";
    etalon["fresh.contact.address.shipping_2"] = "";
    etalon["fresh.contact.address.shipping_3"] = "";
    etalon["fresh.contact.telephone"]    = "";
    etalon["fresh.contact.fax"]          = "";
    etalon["fresh.contact.email"]        = "";
    etalon["fresh.contact.notify_email"] = "";
    etalon["fresh.contact.ident_type"]   = "";
    etalon["fresh.contact.ident"]        = "";
    etalon["fresh.contact.vat"]          = "";
    etalon["fresh.contact.disclose.name"]         = "1";
    etalon["fresh.contact.disclose.org"]          = "1";
    etalon["fresh.contact.disclose.address"]      = "1";
    etalon["fresh.contact.disclose.email"]        = "0";
    etalon["fresh.contact.disclose.notify_email"] = "0";
    etalon["fresh.contact.disclose.ident"]        = "0";
    etalon["fresh.contact.disclose.vat"]          = "0";
    etalon["fresh.contact.disclose.telephone"]    = "0";
    etalon["fresh.contact.disclose.fax"]          = "0";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::contact, Notification::created),
                registrar.id,
                contact.crhistoryid,
                input_svtrid
            )
        )
    );
}

BOOST_FIXTURE_TEST_CASE(test_full_create, has_full_contact)
{
    const std::string input_svtrid = "abc-123";

    std::map<std::string, std::string> etalon;
    etalon["type"] = "1";
    etalon["handle"] = contact.handle;
    etalon["ticket"] = input_svtrid;
    etalon["registrar"] = registrar.name.get_value() + " (" + registrar.url.get_value() + ")";

    etalon["fresh.object.authinfo"]           = contact.authinfopw;
    etalon["fresh.contact.name"]              = contact.name.get_value();
    etalon["fresh.contact.org"]               = contact.organization.get_value();
    etalon["fresh.contact.address.billing"] = "";
    etalon["fresh.contact.address.mailing"] = "";
    const ::LibFred::Contact::PlaceAddress address = contact.place.get_value();
    etalon["fresh.contact.address.permanent"] =
        address.street1 + ", " +
        address.street2.get_value() + ", " +
        address.street3.get_value() + ", " +
        address.stateorprovince.get_value() + ", " +
        address.postalcode + ", " +
        address.city + ", " +
        address.country;
    const std::map<::LibFred::ContactAddressType, ::LibFred::ContactAddress>::const_iterator addr_it =
        contact.addresses.find(::LibFred::ContactAddressType::SHIPPING);
    BOOST_CHECK(addr_it != contact.addresses.end());

    etalon["fresh.contact.address.shipping"] =
        addr_it->second.company_name.get_value() + ", " +
        addr_it->second.street1 + ", " +
        addr_it->second.street2.get_value() + ", " +
        addr_it->second.street3.get_value() + ", " +
        addr_it->second.stateorprovince.get_value() + ", " +
        addr_it->second.postalcode + ", " +
        addr_it->second.city + ", " +
        addr_it->second.country;
    etalon["fresh.contact.address.shipping_2"] = "";
    etalon["fresh.contact.address.shipping_3"] = "";
    etalon["fresh.contact.telephone"]    = contact.telephone.get_value();
    etalon["fresh.contact.fax"]          = contact.fax.get_value();
    etalon["fresh.contact.email"]        = contact.email.get_value();
    etalon["fresh.contact.notify_email"] = contact.notifyemail.get_value();
    etalon["fresh.contact.ident_type"]   = contact.ssntype.get_value();
    etalon["fresh.contact.ident"]        = contact.ssn.get_value();
    etalon["fresh.contact.vat"]          = contact.vat.get_value();
    etalon["fresh.contact.disclose.name"]         = "1";
    etalon["fresh.contact.disclose.org"]          = "1";
    etalon["fresh.contact.disclose.email"]        = "1";
    etalon["fresh.contact.disclose.address"]      = "1";
    etalon["fresh.contact.disclose.notify_email"] = "1";
    etalon["fresh.contact.disclose.ident"]        = "1";
    etalon["fresh.contact.disclose.vat"]          = "1";
    etalon["fresh.contact.disclose.telephone"]    = "1";
    etalon["fresh.contact.disclose.fax"]          = "1";

    check_maps_are_equal(
        etalon,
        Notification::gather_email_content(
            ctx,
            Notification::notification_request(
                Notification::EventOnObject(::LibFred::contact, Notification::created),
                registrar.id,
                contact.crhistoryid,
                input_svtrid
            )
        )
    );
}
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
