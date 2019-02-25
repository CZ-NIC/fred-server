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
/**
 *  @file
 *  merge contact test fixture
 */

#ifndef TEST_MERGE_CONTACT_FIXTURE_HH_33293FCB533C454FAC1CEFD251A927A1
#define TEST_MERGE_CONTACT_FIXTURE_HH_33293FCB533C454FAC1CEFD251A927A1

#include <math.h>
#include <string>
#include <vector>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "util/util.hh"
#include "util/printable.hh"
#include "util/map_at.hh"
#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include "libfred/registrable_object/contact/create_contact.hh"
#include "libfred/registrable_object/contact/info_contact.hh"
#include "libfred/registrable_object/contact/info_contact_diff.hh"
#include "libfred/registrable_object/nsset/create_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset.hh"
#include "libfred/registrable_object/nsset/info_nsset_diff.hh"
#include "libfred/registrable_object/keyset/create_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset.hh"
#include "libfred/registrable_object/keyset/info_keyset_diff.hh"
#include "libfred/registrable_object/domain/create_domain.hh"
#include "libfred/registrable_object/domain/info_domain.hh"
#include "libfred/registrable_object/domain/info_domain_diff.hh"
#include "libfred/registrar/create_registrar.hh"
#include "libfred/registrar/info_registrar.hh"
#include "libfred/registrar/info_registrar_diff.hh"

#include "libfred/object_state/get_object_states.hh"
#include "src/deprecated/libfred/object_state/object_state_name.hh"
#include "libfred/object_state/object_has_state.hh"
#include "libfred/object_state/perform_object_state_request.hh"
#include "libfred/object_state/create_object_state_request_id.hh"
#include "libfred/object_state/lock_object_state_request_lock.hh"

// obsolete
namespace LibFred {
namespace ObjectState {

const std::string CONTACT_IN_MANUAL_VERIFICATION = "contactInManualVerification"; // 26
const std::string CONTACT_PASSED_MANUAL_VERIFICATION = "contactPassedManualVerification"; // 25
const std::string CONTACT_FAILED_MANUAL_VERIFICATION = "contactFailedManualVerification"; // 27

}
}

namespace Test {
namespace LibFred {
namespace Contact {
namespace MergeContactAutoProc {


    /**
     * Setup test data for MergeContact related tests.
     *
     * Nsset handle:
     *
     * NSS-LS<linked_object_state_case_number>-Q<number>OF<quantity_case_number>-T-<tech_contact_handles>
     *
     * Keyset handle:
     *
     * KS-LS<linked_object_state_case_number>-Q<number>OF<quantity_case_number>-T-<tech_contact_handles>
     *
     * Domain name linked via owner:
     *
     * dm-own-ls<linked_object_state_case_number>-q<number>of<quantity_case_number>.own-<owner_contact_handle>.adm.<admin_contact_handles>.cz
     *
     * Domain name linked via admin:
     *
     * dm-adm-ls<linked_object_state_case_number>-q<number>of<quantity_case_number>.own-<owner_contact_handle>.adm.<admin_contact_handles>.cz
     *
     * Contact handle:
     * CT-MC<contact_data_case_number>-GRP<number_of_group>-<registar_handle>-ST<state_case_number>-LO<linked_objects_case_number>-LS<linked_object_state_case_number>-Q<quantity_case_number>
     *
     * - contact_data_case_number is 1 if contact is considered mergeable or 0 if it's not meant to be merged
     * - number of group, default: 0 or 1
     * - registar handle, default: REG1 or REG2
     *
     * Linked object handle or fqdn contains handles of linked contacts.
     *
     * In default config or its subset have numbers prefixed with uppercase letters in handle or lowercase letters in fqdn following meaning:
     *
     * -ST<number> or -st<number> designates contact states configuration where number means:
     * - 0 - no states
     * - 1 - no states
     * - 2 - SERVER_UPDATE_PROHIBITED
     * - 3 - SERVER_TRANSFER_PROHIBITED
     * - 4 - SERVER_DELETE_PROHIBITED
     * - 5 - SERVER_BLOCKED
     * - 6 - MOJEID_CONTACT
     * - 7 - CONTACT_IN_MANUAL_VERIFICATION
     * - 8 - CONTACT_FAILED_MANUAL_VERIFICATION
     * - 9 - CONTACT_PASSED_MANUAL_VERIFICATION
     *
     * -LS<number> or -ls<number> designates linked object (e.g. domain, nsset or keyset) states configuration where number means:
     * - 0 - no states
     * - 1 - SERVER_UPDATE_PROHIBITED
     * - 2 - SERVER_BLOCKED
     * - 3 - SERVER_BLOCKED SERVER_UPDATE_PROHIBITED
     *
     * -LO<number> or -lo<number> designates linked objects (e.g. domains, nssets or keysets) configuration where number means:
     * - 0 - no objects linked to contact
     * - 1 - nsset linked to mergeable tech contact
     * - 2 - nsset linked to mergeable tech contact and to other non-mergeable tech contact
     * - 3 - nsset linked to mergeable tech contact and to other mergeable tech contact
     * - 4 - nsset linked to mergeable tech contact, to other mergeable tech contact and to other non-mergeable tech contact
     * - 5 - keyset linked to mergeable tech contact
     * - 6 - keyset linked to mergeable tech contact and to other non-mergeable tech contact
     * - 7 - keyset linked to mergeable tech contact and to other mergeable tech contact
     * - 8 - keyset linked to mergeable tech contact, to other mergeable tech contact and to other non-mergeable tech contact
     * - 9 - domain linked to mergeable admin contact
     * - 10 - domain linked to mergeable admin contact and to other non-mergeable admin contact
     * - 11 - domain linked to mergeable admin contact and to other mergeable admin contact
     * - 12 - domain linked to mergeable admin contact, to other mergeable admin contact and to other non-mergeable admin contact
     * - 13 - domain linked to mergeable owner
     * - 14 - domain linked to mergeable owner contact and other mergeable admin contact
     * - 15 - nsset and keyset linked to tech contact, domain linked as admin to the same contact, and another domain linked as owner to the same contact, with no linked object states
     * - 16 - nsset and keyset linked to mergeable tech contact, domain linked as admin to the same contact, and another domain linked as owner to the same contact, with linked object states on nsset
     * - 17 - nsset and keyset linked to mergeable tech contact, domain linked as admin to the same contact, and another domain linked as owner to the same contact, with linked object states on keyset
     * - 18 - nsset and keyset linked to mergeable tech contact, domain linked as admin to the same contact, and another domain linked as owner to the same contact, with linked object states on domain linked via admin contact
     * - 19 - nsset and keyset linked to mergeable tech contact, domain linked as admin to the same contact, and another domain linked as owner to the same contact, with linked object states on domain linked via owner contact
     * - 20 - domain linked to mergeable contact as owner and admin, and to other to mergeable contact as admin
     *
     * -Q<number> or -q<number> designates how many linked object configurations will be linked to given contact
     * , default quantities are: 0, 1, 2, 5
     *
     * OF<number> or of<number> is ordinal number in the linked object configuration quantity, starting from 0
     *
     */
    struct mergeable_contact_grps_with_linked_objects_and_blocking_states : Test::instantiate_db_template
    {
        /**
         * Create handle of test contact.
         * @param registrar_handle is contact registrar
         * @param contact_data_case designates how data of contact are filled with regard to mergeability
         * @param grpidtag is identification number of group of test data with otherwise the same configuration
         * @param state_case designates object states configuration of given contact as index to @ref contact_states
         * @param linked_object_case is configuration of objects linked to the contact
         * @param linked_object_state_case is configuration of object states of linked object
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @return contact handle composed of given params
         */
        std::string create_contact_handle(
            const std::string& registrar_handle
            , unsigned contact_data_case
            , unsigned grpidtag
            , unsigned state_case
            , unsigned linked_object_case
            , unsigned linked_object_state_case
            , unsigned quantity_case
            )
        {
            std::string s_state_case = boost::lexical_cast<std::string>(state_case);
            std::string s_linked_objects_case = boost::lexical_cast<std::string>(linked_object_case);
            std::string s_grpidtag = boost::lexical_cast<std::string>(grpidtag);
            std::string s_linked_object_state_case = boost::lexical_cast<std::string>(linked_object_state_case);
            return contact_handle_prefix + boost::lexical_cast<std::string>(contact_data_case)
                +"-GRP"+ s_grpidtag+ "-" + registrar_handle
                +"-ST"+ s_state_case + "-LO" + s_linked_objects_case + "-LS" + s_linked_object_state_case
                +"-Q" + boost::lexical_cast<std::string>(quantity_case);
        }

        /**
         * Create handle of nsset linked to some contact.
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param tech_contact_handle is linked contact handle
         * @param additional_tech_contacts are other linked contact handles
         * @return nsset handle
         */
        std::string create_nsset_with_tech_contact_handle(
                    unsigned linked_object_state_case
                    , unsigned quantity_case
                    , unsigned number_in_quantity
                    , const std::string& tech_contact_handle
                    , std::vector<std::string> additional_tech_contacts = std::vector<std::string>()
                    )
        {
            std::string handle = std::string("NSS")
            +"-LS"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"-Q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "OF" + boost::lexical_cast<std::string>(quantity_case)
            +"-T-"+tech_contact_handle;

            if(!additional_tech_contacts.empty())
            {
                handle += "-T-";
                handle += Util::format_container(additional_tech_contacts,"-T-");
            }
            return handle;
        }

        /**
         * Create handle of keyset linked to some contact.
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param tech_contact_handle is linked contact handle
         * @param additional_tech_contacts are other linked contact handles
         * @return keyset handle
         */
        std::string create_keyset_with_tech_contact_handle(
            unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            , const std::string& tech_contact_handle
            , std::vector<std::string> additional_tech_contacts = std::vector<std::string>()
            )
        {
            std::string handle = std::string("KS")
            +"-LS"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"-Q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "OF" + boost::lexical_cast<std::string>(quantity_case)
            +"-T-"+tech_contact_handle;

            if(!additional_tech_contacts.empty())
            {
                handle += "-T-";
                handle += Util::format_container(additional_tech_contacts,"-T-");
            }

            return handle;
        }

        /**
         * Create fqdn of domain linked to some contact via owner.
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param owner_contact_handle is linked owner contact handle
         * @param admin_contacts are other linked contact handles
         * @return fqdn
         */
        std::string create_domain_with_owner_contact_fqdn(
            unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            , const std::string& owner_contact_handle
            , std::vector<std::string> admin_contacts = std::vector<std::string>()
            )
        {
            std::string admin_contacts_in_fqdn;
            if(!admin_contacts.empty())
            {
                admin_contacts_in_fqdn += ".";
                admin_contacts_in_fqdn += Util::format_container(admin_contacts,".");
                boost::algorithm::to_lower(admin_contacts_in_fqdn);
            }

            std::string fqdn = std::string("dm-own")
            +"-ls"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"-q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "of" + boost::lexical_cast<std::string>(quantity_case)
            +".own-"+boost::algorithm::to_lower_copy(owner_contact_handle)
            + (admin_contacts_in_fqdn.empty() ? "" : ".adm"+ admin_contacts_in_fqdn) + ".cz";
            return fqdn;
        }

        /**
         * Create fqdn of domain linked to some contact via admin.
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param owner_contact_handle is linked owner contact handle
         * @param admin_contact_handle is linked admin contact handle
         * @param admin_contacts are other linked admin contact handles
         * @return fqdn
         */
        std::string create_domain_with_admin_contact_fqdn(
            unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            , const std::string& owner_contact_handle
            , const std::string& admin_contact_handle
            , std::vector<std::string> additional_admin_contacts = std::vector<std::string>()
            )
        {
            std::string additional_admin_contacts_in_fqdn;
            if(!additional_admin_contacts.empty())
            {
                additional_admin_contacts_in_fqdn += ".";
                additional_admin_contacts_in_fqdn += Util::format_container(additional_admin_contacts,".");
                boost::algorithm::to_lower(additional_admin_contacts_in_fqdn);
            }

            std::string fqdn = std::string("dm-adm")
            +"-ls"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"-q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "of" + boost::lexical_cast<std::string>(quantity_case)
            +".own-"+boost::algorithm::to_lower_copy(owner_contact_handle)
            +".adm"+boost::algorithm::to_lower_copy(admin_contact_handle) + additional_admin_contacts_in_fqdn + ".cz";
            return fqdn;
        }
    /**
     * Default set of configurations of linked objects.
     * Need to be kept in sync with implementation in @ref create_linked_object .
     */
        static std::set<unsigned> init_linked_object_combinations()
        {
            return Util::set_of<unsigned>(0)(1)(2)(3)(4)(5)(6)(7)(8)(9)(10)(11)(12)(13)(14)(15)(16)(17)(18)(19)(20);
        }

        /**
         * Default set of quantities of linked object configurations.
         */
        static std::vector<unsigned> init_linked_object_quantities()
        {
            return Util::vector_of<unsigned>(0)(1)(2)(5);
        }

        /**
         * Default set of configurations of mergeable contact states.
         * First two stateless states are abused in linked objects configuration (when two almost the same contacts are needed), second stateless state is also used in tests as dest. contact.
         */
        static std::vector<std::set<std::string> > init_set_of_contact_state_combinations()
        {
            std::vector<std::set<std::string> > states_;
                using namespace ::LibFred::ObjectState;

                //first two stateless states abused in linked objects configuration
                states_.push_back(std::set<std::string>());//state_case 0
                states_.push_back(std::set<std::string>());//state_case 1
                // other states
                states_.push_back(Util::set_of<std::string>(SERVER_UPDATE_PROHIBITED));//state_case 2
                states_.push_back(Util::set_of<std::string>(SERVER_TRANSFER_PROHIBITED));//state_case 3
                states_.push_back(Util::set_of<std::string>(SERVER_DELETE_PROHIBITED));//state_case 4
                states_.push_back(Util::set_of<std::string>(SERVER_BLOCKED));//state_case 5
                states_.push_back(Util::set_of<std::string>(MOJEID_CONTACT));//state_case 6
                states_.push_back(Util::set_of<std::string>(CONTACT_IN_MANUAL_VERIFICATION));//state_case 7
                states_.push_back(Util::set_of<std::string>(CONTACT_FAILED_MANUAL_VERIFICATION));//state_case 8
                states_.push_back(Util::set_of<std::string>(CONTACT_PASSED_MANUAL_VERIFICATION));//state_case 9


                /* other state cases
                serverTransferProhibited serverUpdateProhibited
                serverDeleteProhibited serverUpdateProhibited
                serverDeleteProhibited serverTransferProhibited
                serverDeleteProhibited serverTransferProhibited serverUpdateProhibited

                serverBlocked serverUpdateProhibited
                serverBlocked serverTransferProhibited
                serverBlocked serverTransferProhibited serverUpdateProhibited
                serverBlocked serverDeleteProhibited
                serverBlocked serverDeleteProhibited serverUpdateProhibited
                serverBlocked serverDeleteProhibited serverTransferProhibited
                serverBlocked serverDeleteProhibited serverTransferProhibited serverUpdateProhibited

                mojeidContact serverUpdateProhibited
                mojeidContact serverTransferProhibited
                mojeidContact serverTransferProhibited serverUpdateProhibited
                mojeidContact serverDeleteProhibited
                mojeidContact serverDeleteProhibited serverUpdateProhibited
                mojeidContact serverDeleteProhibited serverTransferProhibited
                mojeidContact serverDeleteProhibited serverTransferProhibited serverUpdateProhibited
                mojeidContact serverBlocked
                mojeidContact serverBlocked serverUpdateProhibited
                mojeidContact serverBlocked serverTransferProhibited
                mojeidContact serverBlocked serverTransferProhibited serverUpdateProhibited
                mojeidContact serverBlocked serverDeleteProhibited
                mojeidContact serverBlocked serverDeleteProhibited serverUpdateProhibited
                mojeidContact serverBlocked serverDeleteProhibited serverTransferProhibited
                mojeidContact serverBlocked serverDeleteProhibited serverTransferProhibited serverUpdateProhibited
                 */

                /*
                for( int j = 1; j < 32; ++j)//2^5 = 32 state combinations
                {
                    std::set<std::string> state_case;
                    if(j & (1 << 0)) state_case.insert(SERVER_UPDATE_PROHIBITED);
                    if(j & (1 << 1)) state_case.insert(SERVER_TRANSFER_PROHIBITED);
                    if(j & (1 << 2)) state_case.insert(SERVER_DELETE_PROHIBITED);
                    if(j & (1 << 3)) state_case.insert(SERVER_BLOCKED);
                    if(j & (1 << 4)) state_case.insert(MOJEID_CONTACT);
                    states_.push_back(state_case);
                    BOOST_TEST_MESSAGE(Util::format_container(state_case));
                }
                 */
            return states_;
        }

        /**
         * Default set of configurations of states of primary linked object within linked object configuration.
         */
        static std::vector<std::set<std::string> > init_set_of_linked_object_state_combinations()
        {
            std::vector<std::set<std::string> > states_;
                using namespace ::LibFred::ObjectState;

                states_.push_back(std::set<std::string>());
                states_.push_back(Util::set_of<std::string>(SERVER_UPDATE_PROHIBITED));
                states_.push_back(Util::set_of<std::string>(SERVER_BLOCKED));
                states_.push_back(Util::set_of<std::string>(SERVER_BLOCKED)(SERVER_UPDATE_PROHIBITED));
                return states_;
        }

    private:

        /**
         * Create contact not meant to be merged with unique enough data and save its data for later comparison.
         * @param registrar_handle is registrar of the contact
         * @param idtag is number to make object different
         * @return handle of created object
         */
        std::string create_non_mergeable_contact(::LibFred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned idtag //to make object unique
            )
        {
            std::string s_idtag = boost::lexical_cast<std::string>(idtag);
            std::string handle = create_contact_handle(registrar_handle, 0,idtag, 0, 15, 0, 1);

            BOOST_TEST_MESSAGE(handle);
            try
            {
                ::LibFred::ContactAddress contact_address = 
                        ::LibFred::ContactAddress(
                            Optional<std::string>(),
                            ::LibFred::Contact::PlaceAddress(
                                "Test"+s_idtag+" St1",
                                Optional< std::string >("Test"+s_idtag+" St2"),
                                Optional< std::string >("Test"+s_idtag+" St3"),
                                "Praha "+s_idtag,
                                Optional< std::string >(""),
                                "12000",
                                "CZ"
                            )
                        );

                ::LibFred::ContactAddressList contact_address_list;
                contact_address_list[::LibFred::ContactAddressType::MAILING] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::BILLING] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::SHIPPING] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::SHIPPING_2] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::SHIPPING_3] = contact_address;

                contact_info.insert(std::make_pair(handle
                    , Test::exec(Test::CreateX_factory<::LibFred::CreateContact>()
                    .make(registrar_handle, Optional<std::string>(handle))
                    .set_name("Name"+s_idtag+s_idtag+" Name"+s_idtag+s_idtag+" Name"+s_idtag+s_idtag+" Name"+s_idtag+s_idtag)
                    .set_organization("")
                    .set_place(::LibFred::Contact::PlaceAddress(
                        "Test"+s_idtag+" St1",
                        Optional< std::string >("Test"+s_idtag+" St2"),
                        Optional< std::string >("Test"+s_idtag+" St3"),
                        "Praha "+s_idtag,
                        Optional< std::string >(""),
                        "12000",
                        "Czech Republic"))
                    .set_telephone("22222222"+s_idtag)
                    .set_fax("222222222"+s_idtag)
                    .set_email("testeml"+s_idtag+"@nic.cz"+s_idtag)
                    .set_notifyemail("testnotifyeml"+s_idtag+"@nic.cz")
                    .set_vat("222222222"+s_idtag)
                    .set_ssn("22222222"+s_idtag)
                    .set_ssntype("OP")
                    .set_disclosename(false)
                    .set_discloseorganization(false)
                    .set_discloseaddress(true)
                    .set_disclosetelephone(false)
                    .set_disclosefax(false)
                    .set_discloseemail(false)
                    .set_disclosevat(false)
                    .set_discloseident(false)
                    .set_disclosenotifyemail(false)
                    .set_domain_expiration_warning_letter_enabled((idtag % 2) == 0)
                    .set_addresses(contact_address_list)
                    , ctx)));

            }
            catch(...)
            {
                BOOST_ERROR(handle + " reg: "+registrar_handle);
            }
            return handle;
        }

        /**
         * Create contact meant to be mergeable within group of mergeable contacts and save its data for later comparison.
         * @param registrar_handle is contact registar
         * @param grpidtag is identification number of group of test data with otherwise the same configuration
         * @param state_case designates object states configuration of given contact as index to @ref contact_states
         * @param linked_object_case is configuration of objects linked to the contact
         * @param linked_object_state_case is configuration of object states of linked object
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @return handle of created object
         */
        std::string create_mergeable_contact(//except the contact states that shall block merge
            ::LibFred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned grpidtag //to make group of mergeable objects unique
            , unsigned state_case
            , unsigned linked_object_case
            , unsigned linked_object_state_case
            , unsigned quantity_case
            )
        {
            std::string s_grpidtag = boost::lexical_cast<std::string>(grpidtag);
            std::string handle = create_contact_handle(registrar_handle, 1
                , grpidtag, state_case, linked_object_case, linked_object_state_case
                , quantity_case);

            BOOST_TEST_MESSAGE(handle);

            try
            {

                ::LibFred::ContactAddress contact_address =
                        ::LibFred::ContactAddress(
                            Optional<std::string>(),
                            ::LibFred::Contact::PlaceAddress(
                                "Test"+s_grpidtag+" St1",
                                Optional< std::string >("Test"+s_grpidtag+" St2"),
                                Optional< std::string >("Test"+s_grpidtag+" St3"),
                                "Praha "+s_grpidtag,
                                Optional< std::string >(""),
                                "1"+s_grpidtag+"000",
                                "CZ"
                            )
                        );

                ::LibFred::ContactAddressList contact_address_list;
                contact_address_list[::LibFred::ContactAddressType::MAILING] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::BILLING] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::SHIPPING] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::SHIPPING_2] = contact_address;
                contact_address_list[::LibFred::ContactAddressType::SHIPPING_3] = contact_address;

                contact_info.insert(std::make_pair(handle
                    , Test::exec(Test::CreateX_factory<::LibFred::CreateContact>()
                    .make(registrar_handle, Optional<std::string>(handle))
                    .set_name("Name"+s_grpidtag+" Name"+s_grpidtag+" Name"+s_grpidtag+" Name"+s_grpidtag)
                    .set_organization("Org"+s_grpidtag+" s.r.o")
                    .set_place(::LibFred::Contact::PlaceAddress(
                        "Test"+s_grpidtag+" St1",
                        Optional< std::string >("Test"+s_grpidtag+" St2"),
                        Optional< std::string >("Test"+s_grpidtag+" St3"),
                        "Praha "+s_grpidtag,
                        Optional< std::string >(""),
                        "1"+s_grpidtag+"000",
                        "Czech Republic"))
                    .set_telephone("11111111"+s_grpidtag)
                    .set_fax("11111111"+s_grpidtag)
                    .set_email("testeml"+s_grpidtag+"@nic.cz")
                    .set_notifyemail("testnotifyeml"+s_grpidtag+"@nic.cz")
                    .set_vat("111111111"+s_grpidtag)
                    .set_ssn("11111111"+s_grpidtag)
                    .set_ssntype("OP")
                    .set_disclosename(true)
                    .set_discloseorganization(true)
                    .set_discloseaddress(true)
                    .set_disclosetelephone(true)
                    .set_disclosefax(true)
                    .set_discloseemail(true)
                    .set_disclosevat(false)
                    .set_discloseident(false)
                    .set_disclosenotifyemail(false)
                    .set_addresses(contact_address_list)
                    .set_domain_expiration_warning_letter_enabled((grpidtag % 2) == 0)
                    , ctx)));

            }
            catch(...)
            {
                BOOST_ERROR(handle + " reg: "+registrar_handle);
            }

            return handle;
        }

        /**
         * Create nsset linked via tech contact and save its data for later comparison.
         * @param registrar_handle is nsset registar
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param tech_contact_handle is linked contact handle
         * @param additional_tech_contacts are other linked contact handles
         * @return handle of created object
         */
        std::string create_nsset_with_tech_contact(
            ::LibFred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            , const std::string& tech_contact_handle
            , std::vector<std::string> additional_tech_contacts = std::vector<std::string>()
            )
        {
            std::string handle = create_nsset_with_tech_contact_handle(
                linked_object_state_case
                , quantity_case
                , number_in_quantity
                , tech_contact_handle
                , additional_tech_contacts);

            BOOST_TEST_MESSAGE(handle);
            try
            {
                nsset_info.insert(std::make_pair(handle
                , Test::exec(Test::CreateX_factory<::LibFred::CreateNsset>()
                    .make(registrar_handle, Optional<std::string>(handle))
                    .set_tech_contacts(Util::vector_of<std::string>(tech_contact_handle)(additional_tech_contacts))
                    ,ctx)));
            }
            catch(...)
            {
                BOOST_ERROR(handle + " reg: "+registrar_handle + " tech: " + handle + " additional tech: " + Util::format_container(additional_tech_contacts));
            }

            return handle;
        }

        /**
         * Create keyset linked via tech contact and save its data for later comparison.
         * @param registrar_handle is keyset registar
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param tech_contact_handle is linked contact handle
         * @param additional_tech_contacts are other linked contact handles
         * @return handle of created object
         */
        std::string create_keyset_with_tech_contact(
            ::LibFred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            , const std::string& tech_contact_handle
            , std::vector<std::string> additional_tech_contacts = std::vector<std::string>()
            )
        {
            std::string handle = create_keyset_with_tech_contact_handle(
                linked_object_state_case
                , quantity_case
                , number_in_quantity
                , tech_contact_handle
                , additional_tech_contacts);

            BOOST_TEST_MESSAGE(handle);
            try
            {
                keyset_info.insert(std::make_pair(handle
                , Test::exec(Test::CreateX_factory<::LibFred::CreateKeyset>()
                    .make(registrar_handle, Optional<std::string>(handle))
                    .set_tech_contacts(Util::vector_of<std::string>(tech_contact_handle)(additional_tech_contacts))
                    ,ctx)));
            }
            catch(...)
            {
                BOOST_ERROR(handle + " reg: "+registrar_handle + " tech: " + handle + " additional tech: " + Util::format_container(additional_tech_contacts));
            }

            return handle;
        }

        /**
         * Create domain linked via owner contact and save its data for later comparison.
         * @param registrar_handle is domain sponsoring registar
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param owner_contact_handle is linked owner contact handle
         * @param admin_contacts are other linked contact handles
         * @return fqdn
         */
        std::string create_domain_with_owner_contact(
            ::LibFred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            , const std::string& owner_contact_handle
            , std::vector<std::string> admin_contacts = std::vector<std::string>()
            )
        {
            std::string fqdn = create_domain_with_owner_contact_fqdn(
                linked_object_state_case
                , quantity_case
                , number_in_quantity
                , owner_contact_handle
                , admin_contacts);

            BOOST_TEST_MESSAGE(fqdn);
            try
            {
            domain_info.insert(std::make_pair(fqdn
            , Test::exec(Test::CreateX_factory<::LibFred::CreateDomain>()
                .make(registrar_handle, owner_contact_handle, Optional<std::string>(fqdn))
                .set_admin_contacts(admin_contacts)
                ,ctx)));
            }
            catch(...)
            {
                BOOST_ERROR(fqdn + " reg: "+registrar_handle + " own: "+ owner_contact_handle + " adm: " + Util::format_container(admin_contacts));
            }
            return fqdn;
        }

        /**
         * Create domain linked via admin contact and save its data for later comparison.
         * @param registrar_handle is domain sponsoring registar
         * @param linked_object_state_case is configuration of object states of linked object, might not be this one but linked to the common contact
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @param owner_contact_handle is linked owner contact handle
         * @param admin_contact_handle is linked admin contact handle
         * @param admin_contacts are other linked admin contact handles
         * @return fqdn
         */
        std::string create_domain_with_admin_contact(
            ::LibFred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            , const std::string& owner_contact_handle
            , const std::string& admin_contact_handle
            , std::vector<std::string> additional_admin_contacts = std::vector<std::string>()
            )
        {
            std::string fqdn = create_domain_with_admin_contact_fqdn(
                linked_object_state_case
                , quantity_case
                , number_in_quantity
                , owner_contact_handle
                , admin_contact_handle
                , additional_admin_contacts);

            BOOST_TEST_MESSAGE(fqdn);
            try
            {
                domain_info.insert(std::make_pair(fqdn
                , Test::exec(Test::CreateX_factory<::LibFred::CreateDomain>()
                    .make(registrar_handle, owner_contact_handle, Optional<std::string>(fqdn))
                    .set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle)(additional_admin_contacts))
                    ,ctx)));
            }
            catch(...)
            {
                BOOST_ERROR(fqdn + " reg: "+registrar_handle + " own: " + owner_contact_handle
                    + " adm: " + admin_contact_handle
                    +" additional adm: " + Util::format_container(additional_admin_contacts));
            }

            return fqdn;
        }

        /**
         * Create object state requests.
         * Regardless of states allowed in enum_object_states.types.
         * @param id is database id of the object
         * @param state_set is set of required states from enum_object_states.name
         */
        void insert_state_requests(
            ::LibFred::OperationContext& ctx
            , unsigned long long id
            , std::set<std::string>  state_set
            )
        {
            ::LibFred::LockObjectStateRequestLock(id).exec(ctx);
            for(std::set<std::string>::const_iterator ci = state_set.begin(); ci != state_set.end(); ++ci)
            {
                ctx.get_conn().exec_params(
                "INSERT INTO object_state_request (object_id, state_id)"
                " VALUES ($1::integer, (SELECT id FROM enum_object_states"
                " WHERE name = $2::text))", Database::query_param_list(id)(*ci));
            }
        }
        /**
         * Create linked object configurations with numbered cases.
         * @param contact_handle is contact linked to created objects, there may also be other contacts linked to the same objects, specified in implementation of linked_object_case
         * @param registrar_handle is default registar, others may be specified in implementation
         * @param grpidtag is identification number of group of test data with otherwise the same configuration
         * @param contact_state_case designates object states configuration of given contact as index to @ref contact_states
         * @param linked_object_case is configuration of objects linked to given contact and specification of object that may have set some states according to @ref linked_object_states
         * @param linked_object_state_case is configuration of object states of linked object designated by returned id
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @param number_in_quantity is ordinal number in the linked object configuration quantity
         * @return id of primary object created according to linked objects case or 0, id is meant to be used for setting object states configurations
         */
        unsigned long long create_linked_object(
            ::LibFred::OperationContext& ctx
            , const std::string& contact_handle
            , const std::string& registrar_handle
            , unsigned grpidtag
            , unsigned contact_state_case
            , unsigned linked_object_case
            , unsigned linked_object_state_case
            , unsigned quantity_case
            , unsigned number_in_quantity
            )
        {
            switch (linked_object_case)
            {
                case 0://no linked objects
                    break;

                case 1://mergeable nsset tech contact
                {
                    return map_at(nsset_info,create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle)).id;
                }
                    break;

                case 2://1 mergeable, 1 non-mergeable nsset tech contact
                {
                    return map_at(nsset_info,create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1)))).id;
                }
                    break;

                case 3://1 mergeable nsset tech contact and 1 different mergeable nsset tech contact
                {
                    return map_at(nsset_info,create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_contact_handle(
                            registrar_handle//registrar_handle
                            , 1 //contact data
                            , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                            , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                            , linked_object_case //unsigned linked_objects_case
                            , linked_object_state_case
                            , quantity_case
                    )))).id;
                }
                    break;

                case 4://1 mergeable nsset tech contact, 1 different mergeable nsset tech contact and 1 non-mergeable nsset tech contact
                {
                    return map_at(nsset_info,create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_contact_handle(
                            registrar_handle//registrar_handle
                            , 1 //contact data
                            , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                            , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                            , linked_object_case //unsigned linked_objects_case
                            , linked_object_state_case
                            , quantity_case
                        ))
                        (create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1))
                        )).id;
                }
                    break;

                case 5://mergeable keyset tech contact
                {
                    return map_at(keyset_info,create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle)).id;
                }
                    break;

                case 6://1 mergeable, 1 non-mergeable keyset tech contact
                {
                    return map_at(keyset_info,create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1)))).id;
                }
                    break;

                case 7://1 mergeable keyset tech contact and 1 different mergeable keyset tech contact
                {
                    return map_at(keyset_info,create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_contact_handle(
                            registrar_handle//registrar_handle
                            , 1 //contact data
                            , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                            , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                            , linked_object_case //unsigned linked_objects_case
                            , linked_object_state_case
                            , quantity_case
                    )))).id;
                }
                    break;

                case 8://1 mergeable keyset tech contact, 1 different mergeable keyset tech contact and 1 non-mergeable keyset tech contact
                {
                    return map_at(keyset_info,create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_contact_handle(
                            registrar_handle//registrar_handle
                            , 1 //contact data
                            , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                            , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                            , linked_object_case //unsigned linked_objects_case
                            , linked_object_state_case
                            , quantity_case
                        ))
                        (create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1))
                        )).id;
                }
                    break;

                case 9://1 mergeable domain admin contact
                {
                    return map_at(domain_info,create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle)).id;
                }
                    break;

                case 10://1 mergeable domain admin contact and 1 non-mergeable
                {
                    return map_at(domain_info,create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle
                        , Util::vector_of<std::string>(create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1)))).id;
                }
                    break;

                case 11://1 mergeable domain admin contact and 1 different mergeable
                {
                    return map_at(domain_info, create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle
                        , Util::vector_of<std::string>(
                            create_contact_handle(
                                registrar_handle//registrar_handle
                                , 1 //contact data
                                , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                                , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                                , linked_object_case //unsigned linked_objects_case
                                , linked_object_state_case
                                , quantity_case
                        )))).id;
                }
                    break;

                case 12://1 mergeable domain admin contact, 1 different mergeable and 1 non-mergeable
                {
                    return map_at(domain_info, create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle
                        , Util::vector_of<std::string>(
                            create_contact_handle(
                                registrar_handle//registrar_handle
                                , 1 //contact data
                                , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                                , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                                , linked_object_case //unsigned linked_objects_case
                                , linked_object_state_case
                                , quantity_case
                        ))
                        (create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1))
                        )).id;
                }
                    break;

                case 13://1 mergeable domain owner contact
                {
                    return map_at(domain_info,create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle)).id;
                }
                    break;

                case 14://1 mergeable domain owner contact and 1 mergeable admin contact
                {
                    return map_at(domain_info,create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(
                            create_contact_handle(
                                registrar_handle//registrar_handle
                                , 1 //contact data
                                , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                                , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                                , linked_object_case //unsigned linked_objects_case
                                , linked_object_state_case
                                , quantity_case
                        )))).id;
                }
                    break;

                case 15:// nsset and keyset via mergeable tech contact
                    // domain via mergeable admin contact
                    // domain via mergeable owner contact
                    // no linked object states
                {
                    create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                    create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                    create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle);
                    create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                }
                    break;

                case 16:// nsset and keyset via mergeable tech contact
                    // domain via mergeable admin contact
                    // domain via mergeable owner contact
                    // states with nsset
                {
                    create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                    create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle);
                    create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                    return map_at(nsset_info,create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle)).id;
                }
                    break;

                case 17:// nsset and keyset via mergeable tech contact
                    // domain via mergeable admin contact
                    // domain via mergeable owner contact
                    // states with keyset
                {
                    create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);

                    create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle);
                    create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);

                    return map_at(keyset_info,create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle)).id;
                }
                    break;

                case 18:// nsset and keyset via mergeable tech contact
                    // domain via mergeable admin contact
                    // domain via mergeable owner contact
                    // states with domain via admin contact
                {
                    create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                    create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);

                    create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);

                    return map_at(domain_info,create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle)).id;
                }
                    break;

                case 19:// nsset and keyset via mergeable tech contact
                    // domain via mergeable admin contact
                    // domain via mergeable owner contact
                    // states with domain via owner contact
                {
                    create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                    create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle);
                    create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_contact_handle(registrar_handle, 0, 1, 0, 15, 0, 1), contact_handle);

                    return map_at(domain_info,create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle)).id;
                }
                    break;

                case 20://domain owner contact, the same admin contact and different mergeable admin contact
                {
                    return map_at(domain_info,create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(contact_handle)(
                            create_contact_handle(
                                registrar_handle//registrar_handle
                                , 1 //contact data
                                , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                                , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                                , linked_object_case //unsigned linked_objects_case
                                , linked_object_state_case
                                , quantity_case
                        )))).id;
                }
                    break;

                default:
                {
                    throw std::runtime_error("unexpected linked object state");
                }
                    break;
            };
            return 0u;
        }

    public:
        std::string registrar_mc_1_handle;
        std::string registrar_mc_2_handle;
        std::string registrar_mojeid_handle;
        std::string registrar_sys_handle;
        std::vector<std::string> registrar_vect;/**< test registrar handles*/
        std::map<std::string, ::LibFred::InfoRegistrarData> registrar_info;/**< map of test registrar info data by handle*/

        std::string contact_handle_prefix;

        std::map<std::string, ::LibFred::InfoContactData> contact_info;/**< map of test contact info data by handle*/
        std::map<std::string, ::LibFred::InfoNssetData> nsset_info;/**< map of test nsset info data by handle*/
        std::map<std::string, ::LibFred::InfoKeysetData> keyset_info;/**< map of test keyset info data by handle*/
        std::map<std::string, ::LibFred::InfoDomainData> domain_info;/**< map of test domain info data by fqdn*/
        unsigned mergeable_contact_group_count;/**< number of groups of mergeable contacts */
        std::vector<std::set<std::string> > contact_states; /**< set of combinations of contact states*/
        std::set<unsigned> linked_object_cases;/**< set of combinations of linked objects configurations*/
        std::vector<std::set<std::string> > linked_object_states; /**< set of combinations of primary linked object states*/
         std::vector<unsigned> linked_object_quantities;/**< set of quantities of linked objects configurations*/

         /**
          * Get contacts changed or deleted since fixture init.
          * Contacts are expected to be deleted in tests, therefore saved info data are compared against the last record in history.
          * @return map of changed contact handles with changed data
          */
         std::map<std::string, ::LibFred::InfoContactDiff> diff_contacts()
         {
             ::LibFred::OperationContextCreator ctx;
             std::map<std::string, ::LibFred::InfoContactDiff> diff_map;
             for(std::map<std::string, ::LibFred::InfoContactData>::const_iterator ci = contact_info.begin(); ci != contact_info.end(); ++ci)
             {
                 ::LibFred::InfoContactDiff diff = ::LibFred::diff_contact_data(ci->second
                     , ::LibFred::InfoContactHistoryById(ci->second.id).exec(ctx).at(0).info_contact_data);//including deleted contacts in history
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get nssets changed since fixture init.
          * Nssets are not expected to be deleted in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed nsset handles with changed data
          */
         std::map<std::string, ::LibFred::InfoNssetDiff> diff_nssets()
         {
             ::LibFred::OperationContextCreator ctx;
             std::map<std::string, ::LibFred::InfoNssetDiff> diff_map;
             for(std::map<std::string, ::LibFred::InfoNssetData>::const_iterator ci = nsset_info.begin(); ci != nsset_info.end(); ++ci)
             {
                 ::LibFred::InfoNssetDiff diff = ::LibFred::diff_nsset_data(ci->second, ::LibFred::InfoNssetByHandle(ci->first).exec(ctx).info_nsset_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get keysets changed since fixture init.
          * Keysets are not expected to be deleted in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed keyset handles with changed data
          */
         std::map<std::string, ::LibFred::InfoKeysetDiff> diff_keysets()
         {
             ::LibFred::OperationContextCreator ctx;
             std::map<std::string, ::LibFred::InfoKeysetDiff> diff_map;
             for(std::map<std::string, ::LibFred::InfoKeysetData>::const_iterator ci = keyset_info.begin(); ci != keyset_info.end(); ++ci)
             {
                 ::LibFred::InfoKeysetDiff diff = ::LibFred::diff_keyset_data(ci->second, ::LibFred::InfoKeysetByHandle(ci->first).exec(ctx).info_keyset_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get domains changed since fixture init.
          * Domains are not expected to be deleted in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed domains, fqdn with changed data
          */
         std::map<std::string, ::LibFred::InfoDomainDiff> diff_domains()
         {
             ::LibFred::OperationContextCreator ctx;
             std::map<std::string, ::LibFred::InfoDomainDiff> diff_map;
             for(std::map<std::string, ::LibFred::InfoDomainData>::const_iterator ci = domain_info.begin(); ci != domain_info.end(); ++ci)
             {
                 ::LibFred::InfoDomainDiff diff = ::LibFred::diff_domain_data(ci->second, ::LibFred::InfoDomainByFqdn(ci->first).exec(ctx).info_domain_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get registrars changed since fixture init.
          * Registrars are not expected to be deleted (nor changed) in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed keyset handles with changed data
          */
         std::map<std::string, ::LibFred::InfoRegistrarDiff> diff_registrars()
         {
             ::LibFred::OperationContextCreator ctx;
             std::map<std::string, ::LibFred::InfoRegistrarDiff> diff_map;
             for(std::map<std::string, ::LibFred::InfoRegistrarData>::const_iterator ci = registrar_info.begin(); ci != registrar_info.end(); ++ci)
             {
                 ::LibFred::InfoRegistrarDiff diff = ::LibFred::diff_registrar_data(ci->second, ::LibFred::InfoRegistrarByHandle(ci->first).exec(ctx).info_registrar_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Check given registrar is system registrar
          * @param registrar_handle
          * @return true if registrar is system registrar, or false if it's not
          */
         bool is_system_registrar(const std::string registrar_handle)
         {
             ::LibFred::OperationContextCreator ctx;
             return ctx.get_conn().exec_params("SELECT id FROM registrar r WHERE r.system IS TRUE AND r.handle = $1::text"
             , Database::query_param_list(registrar_handle)).size() == 1;
         }

         /**
          * Get poll notifications of deleted contacts
          */
         std::map<std::string, unsigned long long> get_del_contact_poll_msg()
         {
             ::LibFred::OperationContextCreator ctx;
             Database::Result poll_res =
             ctx.get_conn().exec("SELECT oreg.name as handle, m.id as msgid "
                 " FROM poll_eppaction pe "
                 " JOIN message m  ON pe.msgid= m.id "
                 " JOIN object_registry oreg ON oreg.id = pe.objid "
                 " JOIN messagetype mt ON mt.id = m.msgtype "
                 " WHERE mt.name = 'delete_contact' ");
             std::map<std::string, unsigned long long> ret;
             for(unsigned long long i = 0; i < poll_res.size(); ++i)
             {
                 ret[poll_res[i]["handle"]]=poll_res[i]["msgid"];
             }
             return ret;
         }

    private:

         /**
          * Common init procedure.
          * Create two test registrars, assemble vector of registrar handles from test registrars and pre-created mojeid registrar and save registrar info data into map by handle for later comparison.
          * For each test registrar create two different nonmergeable test contacts with linked nsset, keyset, domain via owner and domain via admin, that are not meant to be updated.
          * Testcase implementation should check, that nonmergeable objects are not changed by test of merge contact operation.
          * For each test registrar create mergeable test contacts according to set of parameters describing test data configuration:
          *
          * grpidtag enable to have more groups of test data with otherwise the same configuration, these numbers just have to be unique so customization in ctor is count of groups @ref mergeable_contact_group_count
          *
          * state_case designates object states configuration of given contact as index of @ref contact_states, default set of contact state configuration is provided by @ref init_set_of_contact_state_combinations()
          * and can be customized in corresponding ctor parameter @ref contact_state_combinations
          *
          * linked_object_case designates configuration of objects linked to given contact implemented if @ref create_linked_object, it also contains specification of object that may later have set some states according to @ref linked_object_states
          *
          * linked_object_state_case designates configuration of object states set to object given by @ref create_linked_object (if any) according to linked_object_case configuration
          *
          * quantity_case designates how many linked object configurations will be linked to given contact
          *
          * Contacts are created before linked objects because linked object may be linked to more then one contact
          * , this also creates situations where e.g. similar contacts with state_case 0 and 1 are linked to one object so
          * , that merge contact may update linked objects with both contact state cases.
          *
          * The same set of parameters is used for creation of linked objects with their states. At the end is called db function update_object_states for all objects.
          */

        void init_fixture()
        {
            ::LibFred::OperationContextCreator ctx;

            //registrar
            registrar_vect = Util::vector_of<std::string>
                (registrar_mc_1_handle)(registrar_mc_2_handle);
            for(std::vector<std::string>::const_iterator reg_ci = registrar_vect.begin()
                ; reg_ci != registrar_vect.end(); ++reg_ci)
            {
                BOOST_TEST_MESSAGE(*reg_ci);
                registrar_info.insert(std::make_pair(*reg_ci,Test::registrar::make(ctx, Optional<std::string>(*reg_ci))));
            }

            BOOST_TEST_MESSAGE(registrar_mojeid_handle);
            registrar_info.insert(std::make_pair(registrar_mojeid_handle,::LibFred::InfoRegistrarByHandle(registrar_mojeid_handle).exec(ctx).info_registrar_data));

            registrar_info.insert(std::make_pair(registrar_sys_handle
                            , Test::exec(Test::CreateX_factory<::LibFred::CreateRegistrar>()
                            .make(registrar_sys_handle)
                            .set_system(true)
                            ,ctx)));

            //contact
            for(std::vector<std::string>::const_iterator reg_ci = registrar_vect.begin()
                ; reg_ci != registrar_vect.end(); ++reg_ci)
            {
                std::string nmch1 = create_non_mergeable_contact(ctx, *reg_ci, 1);
                std::string nss_nmch1 = create_nsset_with_tech_contact(ctx,*reg_ci,0, 1, 0, nmch1);
                std::string ks_nmch1 = create_keyset_with_tech_contact(ctx,*reg_ci, 0, 1, 0, nmch1);
                std::string dmo_nmch1 = create_domain_with_owner_contact(ctx, *reg_ci, 0, 1, 0, nmch1);

                std::string nmch2 = create_non_mergeable_contact(ctx, *reg_ci, 2);
                std::string nss_nmch2 = create_nsset_with_tech_contact(ctx,*reg_ci,0, 1, 0, nmch2);
                std::string ks_nmch2 = create_keyset_with_tech_contact(ctx,*reg_ci,0, 1, 0, nmch2);
                std::string dmo_nmch2 = create_domain_with_owner_contact(ctx, *reg_ci, 0, 1, 0, nmch2);

                //domain owners crossed
                std::string dma_nmch1 = create_domain_with_admin_contact(ctx, *reg_ci, 0, 1, 0, nmch2, nmch1);
                std::string dma_nmch2 = create_domain_with_admin_contact(ctx, *reg_ci, 0, 1, 0, nmch1, nmch2);

                //mergeable contacts
                for(unsigned grpidtag = 0; grpidtag < mergeable_contact_group_count; ++grpidtag)
                {
                    for(unsigned state_num = 0; state_num < contact_states.size(); ++state_num)
                    {
                        BOOST_TEST_MESSAGE("States S" + boost::lexical_cast<std::string>(state_num) + " " + Util::format_container(contact_states.at(state_num)));
                        for(std::set<unsigned>::const_iterator linked_object_cases_ci = linked_object_cases.begin()
                            ; linked_object_cases_ci != linked_object_cases.end(); ++linked_object_cases_ci)
                        {
                            for(unsigned linked_object_state_case = 0; linked_object_state_case < linked_object_states.size(); ++linked_object_state_case)
                            {
                                for(std::vector<unsigned>::const_iterator q_ci = linked_object_quantities.begin(); q_ci != linked_object_quantities.end(); ++q_ci)
                                {
                                    std::string mch1 = create_mergeable_contact(ctx,*reg_ci, grpidtag, state_num, *linked_object_cases_ci, linked_object_state_case
                                        , *q_ci);
                                    insert_state_requests(ctx, contact_info[mch1].id, contact_states.at(state_num));//set states mch1
                                }
                            }
                        }
                    }
                }
            }

            //linked objects need pre-created contacts
            BOOST_TEST_MESSAGE("create_linked_object");
            for(std::vector<std::string>::const_iterator reg_ci = registrar_vect.begin()
                ; reg_ci != registrar_vect.end(); ++reg_ci)
            {
                BOOST_TEST_MESSAGE(std::string("registrar: ") + *reg_ci);
                for(unsigned grpidtag = 0; grpidtag < mergeable_contact_group_count; ++grpidtag)
                {
                    BOOST_TEST_MESSAGE(std::string("grp: ") + boost::lexical_cast<std::string>(grpidtag));
                    for(unsigned contact_state_case = 0; contact_state_case < contact_states.size(); ++contact_state_case)
                    {
                        BOOST_TEST_MESSAGE(std::string("contact_state_case: ") + boost::lexical_cast<std::string>(contact_state_case));

                        for(std::set<unsigned>::const_iterator linked_object_cases_ci = linked_object_cases.begin()
                            ; linked_object_cases_ci != linked_object_cases.end(); ++linked_object_cases_ci)
                        {
                            BOOST_TEST_MESSAGE(std::string("linked_object_case: ") + boost::lexical_cast<std::string>(*linked_object_cases_ci));
                            for(unsigned linked_object_state_case = 0; linked_object_state_case < linked_object_states.size(); ++linked_object_state_case)
                            {
                                BOOST_TEST_MESSAGE(std::string("linked_object_state_case: ") + boost::lexical_cast<std::string>(linked_object_state_case));
                                for(std::vector<unsigned>::const_iterator q_ci = linked_object_quantities.begin(); q_ci != linked_object_quantities.end(); ++q_ci)
                                {
                                    BOOST_TEST_MESSAGE(std::string("linked_object_quantity: ") + boost::lexical_cast<std::string>(*q_ci));
                                    std::string contact_handle = create_contact_handle(
                                        *reg_ci//registrar_handle
                                        , 1 //contact data
                                        , grpidtag
                                        , contact_state_case
                                        , *linked_object_cases_ci
                                        , linked_object_state_case
                                        , *q_ci
                                        );//return contact handle composed of given params
                                    for(unsigned number = 0; number < *q_ci; ++number)
                                    {
                                        BOOST_TEST_MESSAGE(std::string("linked_object_quantity_case: ") + boost::lexical_cast<std::string>(number) +" of "+ boost::lexical_cast<std::string>(*q_ci));
                                        unsigned long long object_id = create_linked_object(ctx, contact_handle, *reg_ci, grpidtag, contact_state_case
                                            , *linked_object_cases_ci, linked_object_state_case, *q_ci, number);
                                        if(object_id != 0)//else no linked object states
                                        {
                                            insert_state_requests(ctx, object_id, linked_object_states.at(linked_object_state_case));//set states
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            ctx.get_conn().exec("SELECT update_object_states(0)");

            BOOST_TEST_MESSAGE("commit");
            ctx.commit_transaction();
        }
public:
        /**
         * Default fixture setup.
         */
        mergeable_contact_grps_with_linked_objects_and_blocking_states()
        : registrar_mc_1_handle("REG1")
        , registrar_mc_2_handle("REG2")
        , registrar_mojeid_handle("REG-MOJEID")
        , registrar_sys_handle("REG-SYS")

        , contact_handle_prefix("CT-MC")

        , mergeable_contact_group_count(2)
        , contact_states(init_set_of_contact_state_combinations())
        , linked_object_cases(init_linked_object_combinations())
        , linked_object_states(init_set_of_linked_object_state_combinations())
        , linked_object_quantities(init_linked_object_quantities())
        {
            init_fixture();
        }

        /**
         * Custom fixture setup.
         * db_name_suffix
         * @param db_name_suffix is unique suffix of test database instance name
         * @param mergeable_contact_group_count is number of different groups of test data with otherwise the same configuration
         * @param _linked_object_cases is selection of linked object configurations from @ref create_linked_object
         * @param contact_state_combinations is selection of contact states combinations with first two stateless cases, something like provided by @ref init_set_of_contact_state_combinations()
         * @param linked_object_state_combinations is selection of object state configurations of object selected by _linked_object_cases , something like provided by @ref init_set_of_linked_object_state_combinations()
         * @param _linked_object_quantities is selection of linked object configurations quantities per contact, like @ref init_linked_object_quantities()
         */
        explicit mergeable_contact_grps_with_linked_objects_and_blocking_states(
            const std::string& db_name_suffix,
            unsigned mergeable_contact_group_count,
            std::set<unsigned> _linked_object_cases,
            std::vector<std::set<std::string> > contact_state_combinations = Util::vector_of<std::set<std::string> > (std::set<std::string>())(std::set<std::string>()),//stateless states 0, 1
            std::vector<std::set<std::string> > linked_object_state_combinations = Util::vector_of<std::set<std::string> > (std::set<std::string>()),
            std::vector<unsigned> _linked_object_quantities = Util::vector_of<unsigned>(0)
            )
        : Test::instantiate_db_template(db_name_suffix)
        , registrar_mc_1_handle("REG1")
        , registrar_mc_2_handle("REG2")
        , registrar_mojeid_handle("REG-MOJEID")
        , registrar_sys_handle("REG-SYS")

        , contact_handle_prefix("CT-MC")

        , mergeable_contact_group_count(mergeable_contact_group_count)
        , contact_states(contact_state_combinations)
        , linked_object_cases(_linked_object_cases)
        , linked_object_states(linked_object_state_combinations)
        , linked_object_quantities(_linked_object_quantities)
        {
            init_fixture();
        }

        virtual ~mergeable_contact_grps_with_linked_objects_and_blocking_states(){}
    };


} // namespace Test::LibFred::Contact::MergeContactAutoProc
} // namespace Test::LibFred::Contact
} // namespace Test::LibFred
} // namespace Test

#endif
