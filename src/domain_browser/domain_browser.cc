/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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
 *  file@
 *  domain browser implementation
 */

#include <string>
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "util/map_at.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/object/object_impl.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"

#include "domain_browser.h"

namespace Registry
{
    namespace DomainBrowserImpl
    {
        void DomainBrowser::check_user_contact_id(Fred::OperationContext& ctx, unsigned long long user_contact_id)
        {
            try
            {
                Fred::InfoContactById(user_contact_id).exec(ctx);
            }
            catch(const Fred::InfoContactById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    BOOST_THROW_EXCEPTION(UserNotExists());
                }
                else
                    throw;
            }

            if(!Fred::ObjectHasState(user_contact_id,Fred::ObjectState::MOJEID_CONTACT).exec(ctx))
            {
                BOOST_THROW_EXCEPTION(UserNotExists());
            }
        }

        void DomainBrowser::get_object_states(Fred::OperationContext& ctx, unsigned long long object_id, const std::string& lang
            , std::string& state_codes, std::string& states)
        {

            std::vector<Fred::ObjectStateData> state_data = Fred::GetObjectStates(object_id).exec(ctx);
            std::map<unsigned long long, std::string> state_desc_map = Fred::GetObjectStateDescriptions(lang).exec(ctx);

            Util::HeadSeparator states_separator("","|");
            Util::HeadSeparator state_codes_separator("",",");
            for(unsigned long long i = 0; i < state_data.size(); ++i)
            {
                state_codes += state_codes_separator.get();
                state_codes += state_data.at(i).state_name;

                if(state_data.at(i).is_external)
                {
                   states += states_separator.get();
                   states += map_at(state_desc_map, state_data.at(i).state_id);
                }
            }
        }


        DomainBrowser::DomainBrowser(const std::string& server_name)
        : server_name_(server_name)
        {}

        DomainBrowser::~DomainBrowser()
        {}

        unsigned long long DomainBrowser::getObjectRegistryId(const std::string& objtype, const std::string& handle)
        {
            return 0;
        }



        RegistrarDetail DomainBrowser::getRegistrarDetail(
            unsigned long long user_contact_id,
            const std::string& registrar_handle)
        {
            Fred::OperationContext ctx;
            check_user_contact_id(ctx, user_contact_id);

            Fred::InfoRegistrarOutput registar_info;
            try
            {
                registar_info = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx);
            }
            catch(const Fred::InfoRegistrarByHandle::Exception& ex)
            {
                if(ex.is_set_unknown_registrar_handle())
                {
                    BOOST_THROW_EXCEPTION(ObjectNotExists());
                }
                else
                    throw;
            }

            RegistrarDetail result;

            result.id = registar_info.info_registrar_data.id;
            result.handle = registar_info.info_registrar_data.handle;
            result.name = registar_info.info_registrar_data.name.get_value_or_default();
            result.phone = registar_info.info_registrar_data.telephone.get_value_or_default();
            result.fax = registar_info.info_registrar_data.fax.get_value_or_default();
            result.url = registar_info.info_registrar_data.url.get_value_or_default();

            Util::HeadSeparator addr_separator("",", ");

            if(!registar_info.info_registrar_data.street1.isnull())
            {
                result.address +=addr_separator.get();
                result.address += registar_info.info_registrar_data.street1.get_value();
            }

            if(!registar_info.info_registrar_data.street2.isnull())
            {
                result.address +=addr_separator.get();
                result.address += registar_info.info_registrar_data.street2.get_value();
            }

            if(!registar_info.info_registrar_data.street3.isnull())
            {
                result.address +=addr_separator.get();
                result.address += registar_info.info_registrar_data.street3.get_value();
            }

            if(!registar_info.info_registrar_data.city.isnull())
            {
                result.address +=addr_separator.get();
                if(!registar_info.info_registrar_data.postalcode.isnull())
                {
                    result.address += registar_info.info_registrar_data.postalcode.get_value();
                    result.address += " ";
                }
                result.address += registar_info.info_registrar_data.city.get_value();
            }

            if(!registar_info.info_registrar_data.stateorprovince.isnull())
            {
                result.address +=addr_separator.get();
                result.address += registar_info.info_registrar_data.stateorprovince.get_value();
            }

            return result;
        }

        std::string DomainBrowser::get_server_name()
        {
            return "";
        }

        ContactDetail DomainBrowser::getContactDetail(unsigned long long user_contact_id,
                unsigned long long contact_id,
                const std::string& lang)
        {
            Fred::OperationContext ctx;
            check_user_contact_id(ctx, user_contact_id);

            Fred::InfoContactOutput contact_info;
            try
            {
                contact_info = Fred::InfoContactById(contact_id).exec(ctx);
            }
            catch(const Fred::InfoContactById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    BOOST_THROW_EXCEPTION(ObjectNotExists());
                }
                else
                    throw;
            }

            Fred::InfoRegistrarOutput sponsoring_registar_info = Fred::InfoRegistrarByHandle(
                contact_info.info_contact_data.sponsoring_registrar_handle).exec(ctx);

            RegistryReference sponsoring_registrar;
            sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
            sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
            sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();

            ContactDiscloseFlags disclose_flags;
            disclose_flags.name = contact_info.info_contact_data.disclosename.get_value_or_default();
            disclose_flags.organization = contact_info.info_contact_data.discloseorganization.get_value_or_default();
            disclose_flags.email = contact_info.info_contact_data.discloseemail.get_value_or_default();
            disclose_flags.address = contact_info.info_contact_data.discloseaddress.get_value_or_default();
            disclose_flags.telephone = contact_info.info_contact_data.disclosetelephone.get_value_or_default();
            disclose_flags.fax = contact_info.info_contact_data.disclosefax.get_value_or_default();
            disclose_flags.ident = contact_info.info_contact_data.discloseident.get_value_or_default();
            disclose_flags.vat = contact_info.info_contact_data.disclosevat.get_value_or_default();
            disclose_flags.notify_email = contact_info.info_contact_data.disclosenotifyemail.get_value_or_default();

            ContactDetail detail;
            detail.id = contact_info.info_contact_data.id;
            detail.handle = contact_info.info_contact_data.handle;
            detail.roid = contact_info.info_contact_data.roid;
            detail.sponsoring_registrar = sponsoring_registrar;

            detail.creation_time = contact_info.info_contact_data.creation_time;
            detail.update_time = contact_info.info_contact_data.update_time;
            detail.transfer_time = contact_info.info_contact_data.transfer_time;

            detail.is_owner = (user_contact_id == contact_id);
            if(detail.is_owner)//if user contact is the same as requested contact
            {
                detail.authinfopw = contact_info.info_contact_data.authinfopw;
            }
            else
            {
                detail.authinfopw ="********";
            }

            detail.name = contact_info.info_contact_data.name;
            detail.organization = contact_info.info_contact_data.organization;
            detail.street1 = contact_info.info_contact_data.street1;
            detail.street2 = contact_info.info_contact_data.street2;
            detail.street3 = contact_info.info_contact_data.street3;
            detail.city = contact_info.info_contact_data.city;
            detail.stateorprovince = contact_info.info_contact_data.stateorprovince;
            detail.postalcode = contact_info.info_contact_data.postalcode;
            detail.country = contact_info.info_contact_data.country;
            detail.telephone = contact_info.info_contact_data.telephone;
            detail.fax = contact_info.info_contact_data.fax;
            detail.email = contact_info.info_contact_data.email;
            detail.notifyemail = contact_info.info_contact_data.notifyemail;
            detail.vat = contact_info.info_contact_data.vat;
            detail.ssntype = contact_info.info_contact_data.ssntype;
            detail.ssn = contact_info.info_contact_data.ssn;
            detail.disclose_flags = disclose_flags;

            //get states
            get_object_states(ctx, contact_info.info_contact_data.id,lang
                , detail.state_codes, detail.states);

            return detail;
        }

        DomainDetail DomainBrowser::getDomainDetail(unsigned long long user_contact_id,
                unsigned long long domain_id,
                const std::string& lang)
        {
            Fred::OperationContext ctx;
            check_user_contact_id(ctx, user_contact_id);

            Fred::InfoDomainOutput domain_info;
            try
            {
                domain_info = Fred::InfoDomainById(domain_id).exec(ctx);
            }
            catch(const Fred::InfoDomainById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    BOOST_THROW_EXCEPTION(ObjectNotExists());
                }
                else
                    throw;
            }

            Fred::InfoRegistrarOutput sponsoring_registar_info = Fred::InfoRegistrarByHandle(
                domain_info.info_domain_data.sponsoring_registrar_handle).exec(ctx);

            RegistryReference sponsoring_registrar;
            sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
            sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
            sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();

            Fred::InfoContactOutput registrant_contact_info = Fred::InfoContactById(
                domain_info.info_domain_data.registrant.id).exec(ctx);

            RegistryReference registrant;
            registrant.id = registrant_contact_info.info_contact_data.id;
            registrant.handle = registrant_contact_info.info_contact_data.handle;
            registrant.name = registrant_contact_info.info_contact_data.organization.get_value_or_default().empty()
                ? registrant_contact_info.info_contact_data.name.get_value_or_default()
                : registrant_contact_info.info_contact_data.organization.get_value();

            RegistryReference nsset;
            nsset.id = domain_info.info_domain_data.nsset.get_value_or_default().id;
            nsset.handle = domain_info.info_domain_data.nsset.get_value_or_default().handle;

            RegistryReference keyset;
            keyset.id = domain_info.info_domain_data.keyset.get_value_or_default().id;
            keyset.handle = domain_info.info_domain_data.keyset.get_value_or_default().handle;

            DomainDetail detail;
            detail.id = domain_info.info_domain_data.id;
            detail.fqdn = domain_info.info_domain_data.fqdn;
            detail.roid = domain_info.info_domain_data.roid;
            detail.sponsoring_registrar = sponsoring_registrar;
            detail.creation_time = domain_info.info_domain_data.creation_time;
            detail.update_time = domain_info.info_domain_data.update_time;

            detail.is_owner = (user_contact_id == registrant_contact_info.info_contact_data.id);
            if(detail.is_owner)//if user contact is the owner of requested domain
            {
                detail.authinfopw = domain_info.info_domain_data.authinfopw;
            }
            else
            {
                detail.authinfopw ="********";
            }

            detail.registrant = registrant;
            detail.expiration_date = domain_info.info_domain_data.expiration_date;
            detail.enum_domain_validation = domain_info.info_domain_data.enum_domain_validation;
            detail.nsset = nsset;
            detail.keyset = keyset;

            for(std::vector<Fred::ObjectIdHandlePair>::const_iterator ci = domain_info.info_domain_data.admin_contacts.begin();
                    ci != domain_info.info_domain_data.admin_contacts.end(); ++ci)
            {
                Fred::InfoContactOutput admin_contact_info = Fred::InfoContactById(ci->id).exec(ctx);

                RegistryReference admin;
                admin.id = admin_contact_info.info_contact_data.id;
                admin.handle = admin_contact_info.info_contact_data.handle;
                admin.name = admin_contact_info.info_contact_data.organization.get_value_or_default().empty()
                    ? admin_contact_info.info_contact_data.name.get_value_or_default()
                    : admin_contact_info.info_contact_data.organization.get_value();

                detail.admins.push_back(admin);
            }

            get_object_states(ctx, domain_info.info_domain_data.id,lang
                , detail.state_codes, detail.states);

            return detail;
        }


    }//namespace DomainBrowserImpl
}//namespace Registry

