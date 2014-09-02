/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  merge contact test fixture
 */

#ifndef TEST_MERGE_CONTACT_FIXTURE_H_7b8f6ad0c0a540419a8a9b52ba626425
#define TEST_MERGE_CONTACT_FIXTURE_H_7b8f6ad0c0a540419a8a9b52ba626425

#include <math.h>
#include <string>
#include <vector>
#include <map>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include "util/util.h"
#include "util/printable.h"
#include "util/map_at.h"
#include "src/fredlib/opcontext.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/info_contact_diff.h"
#include "src/fredlib/nsset/create_nsset.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/nsset/info_nsset_diff.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/keyset/info_keyset_diff.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/domain/info_domain_diff.h"
#include "src/fredlib/registrar/create_registrar.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/registrar/info_registrar_diff.h"

#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/perform_object_state_request.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"



namespace MergeContactFixture
{
    /**
     * Setup test data for MergeContact related tests
     */
    struct mergeable_contact_grps_with_linked_objects_and_blocking_states : virtual Test::Fixture::instantiate_db_template
    {
        /**
         * Create handle of contact not meant to be merged because of unique data.
         * @param registrar_handle is registrar of the contact
         * @param idtag is number to make object different
         * @return contact handle composed of given params
         */
        std::string create_non_mergeable_contact_handle(
            const std::string& registrar_handle
            , unsigned idtag
            )
        {
            std::string s_idtag = boost::lexical_cast<std::string>(idtag);
            return non_mergeable_contact_handle + registrar_handle + "_" + s_idtag;
        }

        /**
         * Create handle of contact meant to be merged because of the same data with some other contact in group of mergeable contacts.
         * @param registrar_handle is contact registar
         * @param grpidtag is identification number of group of test data with otherwise the same configuration
         * @param state_case designates object states configuration of given contact as index to @ref contact_states
         * @param linked_object_case is configuration of objects linked to the contact
         * @param linked_object_state_case is configuration of object states of linked object
         * @param quantity_case designates how many linked object configurations will be linked to given contact
         * @return contact handle composed of given params
         */
        std::string create_mergeable_contact_handle(
            const std::string& registrar_handle
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
            return mergeable_contact_handle + s_grpidtag+ "_" + registrar_handle
                +"_S"+ s_state_case + "_LO" + s_linked_objects_case + "_LOS" + s_linked_object_state_case
                +"_Q" + boost::lexical_cast<std::string>(quantity_case);
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
            std::string handle = std::string("NSS_TECH_")
            +"LOS"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"_Q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "OF" + boost::lexical_cast<std::string>(quantity_case)
            +"_"+tech_contact_handle;

            if(!additional_tech_contacts.empty())
            {
                handle += "_";
                handle += Util::format_container(additional_tech_contacts,"_");
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
            std::string handle = std::string("KS_TECH_")
            +"LOS"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"_Q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "OF" + boost::lexical_cast<std::string>(quantity_case)
            +"_"+tech_contact_handle;

            if(!additional_tech_contacts.empty())
            {
                handle += "_";
                handle += Util::format_container(additional_tech_contacts,"_");
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
                boost::algorithm::replace_all(admin_contacts_in_fqdn, "_", "-");
                boost::algorithm::to_lower(admin_contacts_in_fqdn);
            }

            std::string fqdn = std::string("dm-own-")
            +"los"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"-q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "of" + boost::lexical_cast<std::string>(quantity_case)
            +"-"+boost::algorithm::to_lower_copy(
                boost::algorithm::replace_all_copy(owner_contact_handle,"_", "-"))
                + admin_contacts_in_fqdn + ".cz";
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
                boost::algorithm::replace_all(additional_admin_contacts_in_fqdn, "_", "-");
                boost::algorithm::to_lower(additional_admin_contacts_in_fqdn);
            }

            std::string fqdn = std::string("dm-adm-")
            +"los"+ boost::lexical_cast<std::string>(linked_object_state_case)
            +"-q" + boost::lexical_cast<std::string>(number_in_quantity)
            + "of" + boost::lexical_cast<std::string>(quantity_case)
            +"-"+boost::algorithm::to_lower_copy(
                boost::algorithm::replace_all_copy(admin_contact_handle,"_", "-")) + additional_admin_contacts_in_fqdn + ".cz";
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
                using namespace Fred::ObjectState;

                //first two stateless states abused in linked objects configuration
                states_.push_back(std::set<std::string>());//state_case 0
                states_.push_back(std::set<std::string>());//state_case 1

                //state_case 2 - 6
                states_.push_back(Util::set_of<std::string>(SERVER_UPDATE_PROHIBITED));
                states_.push_back(Util::set_of<std::string>(SERVER_TRANSFER_PROHIBITED));
                states_.push_back(Util::set_of<std::string>(SERVER_DELETE_PROHIBITED));
                states_.push_back(Util::set_of<std::string>(SERVER_BLOCKED));
                states_.push_back(Util::set_of<std::string>(MOJEID_CONTACT));


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
                    BOOST_MESSAGE(Util::format_container(state_case));
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
                using namespace Fred::ObjectState;

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
        std::string create_non_mergeable_contact(Fred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned idtag //to make object unique
            )
        {
            std::string s_idtag = boost::lexical_cast<std::string>(idtag);
            std::string handle = create_non_mergeable_contact_handle(registrar_handle, idtag);

            BOOST_MESSAGE(handle);
            contact_info.insert(std::make_pair(handle
                , Test::exec(Test::CreateX_factory<Fred::CreateContact>()
                .make(registrar_handle, Optional<std::string>(handle))
                .set_name("Name"+s_idtag+s_idtag+" Name"+s_idtag+s_idtag+" Name"+s_idtag+s_idtag+" Name"+s_idtag+s_idtag)
                .set_organization("")
                .set_street1("Test"+s_idtag+" St1")
                .set_street2("Test"+s_idtag+" St2")
                .set_street3("Test"+s_idtag+" St3")
                .set_city("Praha "+s_idtag)
                .set_postalcode("12000")
                .set_stateorprovince("")
                .set_country("Czech Republic")
                .set_telephone("22222222"+s_idtag)
                .set_fax("222222222"+s_idtag)
                .set_email("testeml"+s_idtag+"@nic.cz"+s_idtag)
                .set_notifyemail("testnotifyeml"+s_idtag+"@nic.cz")
                .set_vat("222222222"+s_idtag)
                .set_ssn("22222222"+s_idtag)
                .set_ssntype("OP")
                .set_disclosename(false)
                .set_discloseorganization(false)
                .set_discloseaddress(false)
                .set_disclosetelephone(false)
                .set_disclosefax(false)
                .set_discloseemail(false)
                .set_disclosevat(false)
                .set_discloseident(false)
                .set_disclosenotifyemail(false)
                ,ctx)));
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
            Fred::OperationContext& ctx
            , const std::string& registrar_handle
            , unsigned grpidtag //to make group of mergeable objects unique
            , unsigned state_case
            , unsigned linked_object_case
            , unsigned linked_object_state_case
            , unsigned quantity_case
            )
        {
            std::string s_grpidtag = boost::lexical_cast<std::string>(grpidtag);
            std::string handle = create_mergeable_contact_handle(registrar_handle
                , grpidtag, state_case, linked_object_case, linked_object_state_case
                , quantity_case);

            BOOST_MESSAGE(handle);
            contact_info.insert(std::make_pair(handle
                , Test::exec(Test::CreateX_factory<Fred::CreateContact>()
                .make(registrar_handle, Optional<std::string>(handle))
                .set_name("Name"+s_grpidtag+" Name"+s_grpidtag+" Name"+s_grpidtag+" Name"+s_grpidtag)
                .set_organization("Org"+s_grpidtag+" s.r.o")
                .set_street1("Test"+s_grpidtag+" St1")
                .set_street2("Test"+s_grpidtag+" St2")
                .set_street3("Test"+s_grpidtag+" St3")
                .set_city("Praha "+s_grpidtag)
                .set_postalcode("1"+s_grpidtag+"000")
                .set_stateorprovince("")
                .set_country("Czech Republic")
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
                ,ctx)));
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
            Fred::OperationContext& ctx
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

            BOOST_MESSAGE(handle);
            nsset_info.insert(std::make_pair(handle
            , Test::exec(Test::CreateX_factory<Fred::CreateNsset>()
                .make(registrar_handle, Optional<std::string>(handle))
                .set_tech_contacts(Util::vector_of<std::string>(tech_contact_handle)(additional_tech_contacts))
                ,ctx)));
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
            Fred::OperationContext& ctx
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

            BOOST_MESSAGE(handle);
            keyset_info.insert(std::make_pair(handle
            , Test::exec(Test::CreateX_factory<Fred::CreateKeyset>()
                .make(registrar_handle, Optional<std::string>(handle))
                .set_tech_contacts(Util::vector_of<std::string>(tech_contact_handle)(additional_tech_contacts))
                ,ctx)));
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
            Fred::OperationContext& ctx
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

            BOOST_MESSAGE(fqdn);
            domain_info.insert(std::make_pair(fqdn
            , Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                .make(registrar_handle, owner_contact_handle, Optional<std::string>(fqdn))
                .set_admin_contacts(admin_contacts)
                ,ctx)));
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
            Fred::OperationContext& ctx
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

            BOOST_MESSAGE(fqdn);
            domain_info.insert(std::make_pair(fqdn
            , Test::exec(Test::CreateX_factory<Fred::CreateDomain>()
                .make(registrar_handle, owner_contact_handle, Optional<std::string>(fqdn))
                .set_admin_contacts(Util::vector_of<std::string>(admin_contact_handle)(additional_admin_contacts))
                ,ctx)));
            return fqdn;
        }

        /**
         * Create object state requests.
         * Regardless of states allowed in enum_object_states.types.
         * @param id is database id of the object
         * @param state_set is set of required states from enum_object_states.name
         */
        void insert_state_requests(
            Fred::OperationContext& ctx
            , unsigned long long id
            , std::set<std::string>  state_set
            )
        {
            for(std::set<std::string>::const_iterator ci = state_set.begin(); ci != state_set.end(); ++ci)
            {
                Fred::LockObjectStateRequestLock(*ci, id).exec(ctx);
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
            Fred::OperationContext& ctx
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
                        , Util::vector_of<std::string>(create_non_mergeable_contact_handle(registrar_handle, 1)))).id;
                }
                    break;

                case 3://1 mergeable nsset tech contact and 1 different mergeable nsset tech contact
                {
                    return map_at(nsset_info,create_nsset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_mergeable_contact_handle(
                            registrar_handle//registrar_handle
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
                        , Util::vector_of<std::string>(create_mergeable_contact_handle(
                            registrar_handle//registrar_handle
                            , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                            , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                            , linked_object_case //unsigned linked_objects_case
                            , linked_object_state_case
                            , quantity_case
                        ))
                        (create_non_mergeable_contact_handle(registrar_handle, 1))
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
                        , Util::vector_of<std::string>(create_non_mergeable_contact_handle(registrar_handle, 1)))).id;
                }
                    break;

                case 7://1 mergeable keyset tech contact and 1 different mergeable keyset tech contact
                {
                    return map_at(keyset_info,create_keyset_with_tech_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(create_mergeable_contact_handle(
                            registrar_handle//registrar_handle
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
                        , Util::vector_of<std::string>(create_mergeable_contact_handle(
                            registrar_handle//registrar_handle
                            , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                            , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                            , linked_object_case //unsigned linked_objects_case
                            , linked_object_state_case
                            , quantity_case
                        ))
                        (create_non_mergeable_contact_handle(registrar_handle, 1))
                        )).id;
                }
                    break;

                case 9://1 mergeable domain admin contact
                {
                    return map_at(domain_info,create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle)).id;
                }
                    break;

                case 10://1 mergeable domain admin contact and 1 non-mergeable
                {
                    return map_at(domain_info,create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle
                        , Util::vector_of<std::string>(create_non_mergeable_contact_handle(registrar_handle, 1)))).id;
                }
                    break;

                case 11://1 mergeable domain admin contact and 1 different mergeable
                {
                    return map_at(domain_info, create_domain_with_admin_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle
                        , Util::vector_of<std::string>(
                            create_mergeable_contact_handle(
                                registrar_handle//registrar_handle
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
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle
                        , Util::vector_of<std::string>(
                            create_mergeable_contact_handle(
                                registrar_handle//registrar_handle
                                , grpidtag //unsigned grpidtag //to make group of mergeable objects unique
                                , contact_state_case ? 0 : 1 //unsigned state_case, different contact from the same set with no blocking states
                                , linked_object_case //unsigned linked_objects_case
                                , linked_object_state_case
                                , quantity_case
                        ))
                        (create_non_mergeable_contact_handle(registrar_handle, 1))
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
                            create_mergeable_contact_handle(
                                registrar_handle//registrar_handle
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
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle);
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
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle);
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
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle);
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
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle)).id;
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
                        , create_non_mergeable_contact_handle(registrar_handle, 1), contact_handle);

                    return map_at(domain_info,create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle)).id;
                }
                    break;

                case 20://domain owner contact, the same admin contact and different mergeable admin contact
                {
                    return map_at(domain_info,create_domain_with_owner_contact(ctx,registrar_handle, linked_object_state_case
                        , quantity_case, number_in_quantity, contact_handle
                        , Util::vector_of<std::string>(contact_handle)(
                            create_mergeable_contact_handle(
                                registrar_handle//registrar_handle
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
        std::map<std::string, Fred::InfoRegistrarData> registrar_info;/**< map of test registrar info data by handle*/

        std::string mergeable_contact_handle;
        std::string non_mergeable_contact_handle;/**< prefix of handle of contact that is not meant to be merged */

        std::map<std::string, Fred::InfoContactData> contact_info;/**< map of test contact info data by handle*/
        std::map<std::string, Fred::InfoNssetData> nsset_info;/**< map of test nsset info data by handle*/
        std::map<std::string, Fred::InfoKeysetData> keyset_info;/**< map of test keyset info data by handle*/
        std::map<std::string, Fred::InfoDomainData> domain_info;/**< map of test domain info data by fqdn*/
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
         std::map<std::string, Fred::InfoContactDiff> diff_contacts()
         {
             Fred::OperationContext ctx;
             std::map<std::string, Fred::InfoContactDiff> diff_map;
             for(std::map<std::string, Fred::InfoContactData>::const_iterator ci = contact_info.begin(); ci != contact_info.end(); ++ci)
             {
                 Fred::InfoContactDiff diff = Fred::diff_contact_data(ci->second
                     , Fred::InfoContactHistoryById(ci->second.id).exec(ctx).at(0).info_contact_data);//including deleted contacts in history
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get nssets changed since fixture init.
          * Nssets are not expected to be deleted in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed nsset handles with changed data
          */
         std::map<std::string, Fred::InfoNssetDiff> diff_nssets()
         {
             Fred::OperationContext ctx;
             std::map<std::string, Fred::InfoNssetDiff> diff_map;
             for(std::map<std::string, Fred::InfoNssetData>::const_iterator ci = nsset_info.begin(); ci != nsset_info.end(); ++ci)
             {
                 Fred::InfoNssetDiff diff = Fred::diff_nsset_data(ci->second, Fred::InfoNssetByHandle(ci->first).exec(ctx).info_nsset_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get keysets changed since fixture init.
          * Keysets are not expected to be deleted in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed keyset handles with changed data
          */
         std::map<std::string, Fred::InfoKeysetDiff> diff_keysets()
         {
             Fred::OperationContext ctx;
             std::map<std::string, Fred::InfoKeysetDiff> diff_map;
             for(std::map<std::string, Fred::InfoKeysetData>::const_iterator ci = keyset_info.begin(); ci != keyset_info.end(); ++ci)
             {
                 Fred::InfoKeysetDiff diff = Fred::diff_keyset_data(ci->second, Fred::InfoKeysetByHandle(ci->first).exec(ctx).info_keyset_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get domains changed since fixture init.
          * Domains are not expected to be deleted in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed domains, fqdn with changed data
          */
         std::map<std::string, Fred::InfoDomainDiff> diff_domains()
         {
             Fred::OperationContext ctx;
             std::map<std::string, Fred::InfoDomainDiff> diff_map;
             for(std::map<std::string, Fred::InfoDomainData>::const_iterator ci = domain_info.begin(); ci != domain_info.end(); ++ci)
             {
                 Fred::InfoDomainDiff diff = Fred::diff_domain_data(ci->second, Fred::InfoDomainByHandle(ci->first).exec(ctx).info_domain_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
         }

         /**
          * Get registrars changed since fixture init.
          * Registrars are not expected to be deleted (nor changed) in tests, therefore saved info data are compared against current record of object, in case of deleted or missing object it shall fail.
          * @return map of changed keyset handles with changed data
          */
         std::map<std::string, Fred::InfoRegistrarDiff> diff_registrars()
         {
             Fred::OperationContext ctx;
             std::map<std::string, Fred::InfoRegistrarDiff> diff_map;
             for(std::map<std::string, Fred::InfoRegistrarData>::const_iterator ci = registrar_info.begin(); ci != registrar_info.end(); ++ci)
             {
                 Fred::InfoRegistrarDiff diff = Fred::diff_registrar_data(ci->second, Fred::InfoRegistrarByHandle(ci->first).exec(ctx).info_registrar_data);
                 if(!diff.is_empty()) diff_map.insert(std::make_pair(ci->first, diff));
             }
             return diff_map;
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
            Fred::OperationContext ctx;

            //registrar
            registrar_vect = Util::vector_of<std::string>
                (registrar_mc_1_handle)(registrar_mc_2_handle);
            for(std::vector<std::string>::const_iterator reg_ci = registrar_vect.begin()
                ; reg_ci != registrar_vect.end(); ++reg_ci)
            {
                BOOST_MESSAGE(*reg_ci);
                registrar_info.insert(std::make_pair(*reg_ci,Test::registrar::make(ctx, Optional<std::string>(*reg_ci))));
            }

            BOOST_MESSAGE(registrar_mojeid_handle);
            registrar_info.insert(std::make_pair(registrar_mojeid_handle,Fred::InfoRegistrarByHandle(registrar_mojeid_handle).exec(ctx).info_registrar_data));

            registrar_info.insert(std::make_pair(registrar_sys_handle
                            , Test::exec(Test::CreateX_factory<Fred::CreateRegistrar>()
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
                        BOOST_MESSAGE("States S" + boost::lexical_cast<std::string>(state_num) + " " + Util::format_container(contact_states.at(state_num)));
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
            BOOST_MESSAGE("create_linked_object");
            for(std::vector<std::string>::const_iterator reg_ci = registrar_vect.begin()
                ; reg_ci != registrar_vect.end(); ++reg_ci)
            {
                BOOST_MESSAGE(std::string("registrar: ") + *reg_ci);
                for(unsigned grpidtag = 0; grpidtag < mergeable_contact_group_count; ++grpidtag)
                {
                    BOOST_MESSAGE(std::string("grp: ") + boost::lexical_cast<std::string>(grpidtag));
                    for(unsigned contact_state_case = 0; contact_state_case < contact_states.size(); ++contact_state_case)
                    {
                        BOOST_MESSAGE(std::string("contact_state_case: ") + boost::lexical_cast<std::string>(contact_state_case));

                        for(std::set<unsigned>::const_iterator linked_object_cases_ci = linked_object_cases.begin()
                            ; linked_object_cases_ci != linked_object_cases.end(); ++linked_object_cases_ci)
                        {
                            BOOST_MESSAGE(std::string("linked_object_case: ") + boost::lexical_cast<std::string>(*linked_object_cases_ci));
                            for(unsigned linked_object_state_case = 0; linked_object_state_case < linked_object_states.size(); ++linked_object_state_case)
                            {
                                BOOST_MESSAGE(std::string("linked_object_state_case: ") + boost::lexical_cast<std::string>(linked_object_state_case));
                                for(std::vector<unsigned>::const_iterator q_ci = linked_object_quantities.begin(); q_ci != linked_object_quantities.end(); ++q_ci)
                                {
                                    BOOST_MESSAGE(std::string("linked_object_quantity: ") + boost::lexical_cast<std::string>(*q_ci));
                                    std::string contact_handle = create_mergeable_contact_handle(
                                        *reg_ci//registrar_handle
                                        , grpidtag
                                        , contact_state_case
                                        , *linked_object_cases_ci
                                        , linked_object_state_case
                                        , *q_ci
                                        );//return contact handle composed of given params
                                    for(unsigned number = 0; number < *q_ci; ++number)
                                    {
                                        BOOST_MESSAGE(std::string("linked_object_quantity_case: ") + boost::lexical_cast<std::string>(number) +" of "+ boost::lexical_cast<std::string>(*q_ci));
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

            //bug: Fred::PerformObjectStateRequest().exec(ctx);
            ctx.get_conn().exec("SELECT update_object_states(0)");

            BOOST_MESSAGE("commit");
            ctx.commit_transaction();
        }
public:
        /**
         * Default fixture setup.
         */
        mergeable_contact_grps_with_linked_objects_and_blocking_states()
        : registrar_mc_1_handle("REG-1")
        , registrar_mc_2_handle("REG-2")
        , registrar_mojeid_handle("REG-MOJEID")
        , registrar_sys_handle("REG-SYS")

        , mergeable_contact_handle("CT-MC")
        , non_mergeable_contact_handle("CT-NMC_")

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
         * @param mergeable_contact_group_count is number of different groups of test data with otherwise the same configuration
         * @param _linked_object_cases is selection of linked object configurations from @ref create_linked_object
         * @param contact_state_combinations is selection of contact states combinations with first two stateless cases, something like provided by @ref init_set_of_contact_state_combinations()
         * @param linked_object_state_combinations is selection of object state configurations of object selected by _linked_object_cases , something like provided by @ref init_set_of_linked_object_state_combinations()
         * @param _linked_object_quantities is selection of linked object configurations quantities per contact, like @ref init_linked_object_quantities()
         */
        explicit mergeable_contact_grps_with_linked_objects_and_blocking_states(
            unsigned mergeable_contact_group_count,
            std::set<unsigned> _linked_object_cases,
            std::vector<std::set<std::string> > contact_state_combinations = Util::vector_of<std::set<std::string> > (std::set<std::string>())(std::set<std::string>()),//stateless states 0, 1
            std::vector<std::set<std::string> > linked_object_state_combinations = Util::vector_of<std::set<std::string> > (std::set<std::string>()),
            std::vector<unsigned> _linked_object_quantities = Util::vector_of<unsigned>(0)
            )
        : registrar_mc_1_handle("REG-1")
        , registrar_mc_2_handle("REG-2")
        , registrar_mojeid_handle("REG-MOJEID")
        , registrar_sys_handle("REG-SYS")

        , mergeable_contact_handle("CT-MC")
        , non_mergeable_contact_handle("CT-NMC_")

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
}

#endif // TEST_MERGE_CONTACT_FIXTURE_H_7b8f6ad0c0a540419a8a9b52ba626425
