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
#include "util/util.h"
#include "src/fredlib/opcontext.h"
#include "src/fredlib/object/object_impl.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/object_state/create_object_state_request_id.h"
#include "src/fredlib/object_state/cancel_object_state_request_id.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/contact/info_contact.h"
#include "src/fredlib/contact/update_contact.h"
#include "src/fredlib/domain/info_domain.h"
#include "src/fredlib/nsset/info_nsset.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "cfg/handle_mojeid_args.h"
#include "cfg/config_handler_decl.h"

#include "domain_browser.h"

namespace Registry
{
    namespace DomainBrowserImpl
    {

        /**
         * Check user contact.
         * @param EXCEPTION is type of exception used for reporting when contact is not found or not in required state
         * @param ctx contains reference to database and logging interface
         * @param user_contact_id is database id of user contact
         * @param lock_contact_for_update indicates whether to lock contact for update (true) or for share (false)
         * @return contact info or if user contact is deleted or don't have mojeidContact state throw @ref EXCEPTION.
         */
        template <class EXCEPTION> Fred::InfoContactOutput check_user_contact_id(Fred::OperationContext& ctx,
                unsigned long long user_contact_id, bool lock_contact_for_update = false)
        {
            Fred::InfoContactOutput info;
            try
            {
                Fred::InfoContactById info_contact_by_id(user_contact_id);
                if(lock_contact_for_update) info_contact_by_id.set_lock();
                info = info_contact_by_id.exec(ctx);
            }
            catch(const Fred::InfoContactById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    throw EXCEPTION();
                }
                else
                    throw;
            }

            if(!Fred::ObjectHasState(user_contact_id,Fred::ObjectState::MOJEID_CONTACT).exec(ctx))
            {
                throw EXCEPTION();
            }

            return info;
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

        std::string DomainBrowser::filter_authinfo(bool user_is_owner, const std::string& authinfopw)
        {
            if(user_is_owner)
            {
                return authinfopw;
            }

            return "********";//if not
        }


        DomainBrowser::DomainBrowser(const std::string& server_name)
        : server_name_(server_name)
        , update_registrar_(CfgArgs::instance()->get_handler_ptr_by_type<HandleMojeIDArgs>()->registrar_handle)//MojeID registrar
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
            check_user_contact_id<UserNotExists>(ctx, user_contact_id);

            Fred::InfoRegistrarOutput registar_info;
            try
            {
                registar_info = Fred::InfoRegistrarByHandle(registrar_handle).exec(ctx);
            }
            catch(const Fred::InfoRegistrarByHandle::Exception& ex)
            {
                if(ex.is_set_unknown_registrar_handle())
                {
                    throw ObjectNotExists();
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
            check_user_contact_id<UserNotExists>(ctx, user_contact_id);

            Fred::InfoContactOutput contact_info;
            try
            {
                contact_info = Fred::InfoContactById(contact_id).exec(ctx);
            }
            catch(const Fred::InfoContactById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    throw ObjectNotExists();
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
            detail.authinfopw =filter_authinfo(detail.is_owner, contact_info.info_contact_data.authinfopw);

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
            check_user_contact_id<UserNotExists>(ctx, user_contact_id);

            Fred::InfoDomainOutput domain_info;
            try
            {
                domain_info = Fred::InfoDomainById(domain_id).exec(ctx);
            }
            catch(const Fred::InfoDomainById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    throw ObjectNotExists();
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
            detail.authinfopw =filter_authinfo(detail.is_owner, domain_info.info_domain_data.authinfopw);

            detail.registrant = registrant;
            detail.expiration_date = domain_info.info_domain_data.expiration_date;
            detail.enum_domain_validation = domain_info.info_domain_data.enum_domain_validation;
            detail.nsset = nsset;
            detail.keyset = keyset;

            detail.admins.reserve(domain_info.info_domain_data.admin_contacts.size());
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

        NssetDetail DomainBrowser::getNssetDetail(unsigned long long user_contact_id,
                unsigned long long nsset_id,
                const std::string& lang)
        {
            Fred::OperationContext ctx;
            check_user_contact_id<UserNotExists>(ctx, user_contact_id);

            Fred::InfoNssetOutput nsset_info;
            try
            {
                nsset_info = Fred::InfoNssetById(nsset_id).exec(ctx);
            }
            catch(const Fred::InfoNssetById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    throw ObjectNotExists();
                }
                else
                    throw;
            }

            Fred::InfoRegistrarOutput sponsoring_registar_info = Fred::InfoRegistrarByHandle(
                nsset_info.info_nsset_data.sponsoring_registrar_handle).exec(ctx);
            RegistryReference sponsoring_registrar;
            sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
            sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
            sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();


            Fred::InfoRegistrarOutput create_registar_info = Fred::InfoRegistrarByHandle(
                nsset_info.info_nsset_data.create_registrar_handle).exec(ctx);
            RegistryReference create_registrar;
            create_registrar.id = create_registar_info.info_registrar_data.id;
            create_registrar.handle = create_registar_info.info_registrar_data.handle;
            create_registrar.name = create_registar_info.info_registrar_data.name.get_value_or_default();

            RegistryReference update_registrar;
            if(!nsset_info.info_nsset_data.update_registrar_handle.isnull())
            {
                Fred::InfoRegistrarOutput update_registar_info = Fred::InfoRegistrarByHandle(
                    nsset_info.info_nsset_data.update_registrar_handle.get_value()).exec(ctx);
                create_registrar.id = create_registar_info.info_registrar_data.id;
                create_registrar.handle = create_registar_info.info_registrar_data.handle;
                create_registrar.name = create_registar_info.info_registrar_data.name.get_value_or_default();
            }


            NssetDetail detail;

            detail.id = nsset_info.info_nsset_data.id;
            detail.handle = nsset_info.info_nsset_data.handle;
            detail.roid = nsset_info.info_nsset_data.roid;
            detail.sponsoring_registrar = sponsoring_registrar;
            detail.creation_time = nsset_info.info_nsset_data.creation_time;
            detail.transfer_time = nsset_info.info_nsset_data.transfer_time;
            detail.update_time = nsset_info.info_nsset_data.update_time;

            detail.create_registrar = create_registrar;
            detail.update_registrar = update_registrar;

            detail.is_owner = false;

            detail.admins.reserve(nsset_info.info_nsset_data.tech_contacts.size());
            for(std::vector<Fred::ObjectIdHandlePair>::const_iterator ci = nsset_info.info_nsset_data.tech_contacts.begin();
                    ci != nsset_info.info_nsset_data.tech_contacts.end(); ++ci)
            {
                Fred::InfoContactOutput tech_contact_info = Fred::InfoContactById(ci->id).exec(ctx);

                RegistryReference admin;
                admin.id = tech_contact_info.info_contact_data.id;
                admin.handle = tech_contact_info.info_contact_data.handle;
                admin.name = tech_contact_info.info_contact_data.organization.get_value_or_default().empty()
                    ? tech_contact_info.info_contact_data.name.get_value_or_default()
                    : tech_contact_info.info_contact_data.organization.get_value();
                detail.admins.push_back(admin);

                if(admin.id == user_contact_id) detail.is_owner = true;//reveal authinfo
            }
            detail.authinfopw =filter_authinfo(detail.is_owner, nsset_info.info_nsset_data.authinfopw);

            detail.hosts.reserve(nsset_info.info_nsset_data.dns_hosts.size());
            for(std::vector<Fred::DnsHost>::const_iterator ci = nsset_info.info_nsset_data.dns_hosts.begin();
                    ci != nsset_info.info_nsset_data.dns_hosts.end(); ++ci)
            {
                DNSHost host;
                host.fqdn = ci->get_fqdn();
                Util::HeadSeparator add_separator("",", ");
                std::vector<std::string> inet_addr_list = ci->get_inet_addr();
                for(std::vector<std::string>::const_iterator cj = inet_addr_list.begin();
                    cj != inet_addr_list.end(); ++cj)
                {
                    host.inet_addr += add_separator.get();
                    host.inet_addr += *cj;
                }

                detail.hosts.push_back(host);
            }

            get_object_states(ctx, nsset_info.info_nsset_data.id,lang
                , detail.state_codes, detail.states);

            detail.report_level = nsset_info.info_nsset_data.tech_check_level.get_value_or_default();

            return detail;
        }

        KeysetDetail DomainBrowser::getKeysetDetail(unsigned long long user_contact_id,
                unsigned long long keyset_id,
                const std::string& lang)
        {
            Fred::OperationContext ctx;
            check_user_contact_id<UserNotExists>(ctx, user_contact_id);

            Fred::InfoKeysetOutput keyset_info;
            try
            {
                keyset_info = Fred::InfoKeysetById(keyset_id).exec(ctx);
            }
            catch(const Fred::InfoKeysetById::Exception& ex)
            {
                if(ex.is_set_unknown_object_id())
                {
                    throw ObjectNotExists();
                }
                else
                    throw;
            }

            Fred::InfoRegistrarOutput sponsoring_registar_info = Fred::InfoRegistrarByHandle(
                keyset_info.info_keyset_data.sponsoring_registrar_handle).exec(ctx);
            RegistryReference sponsoring_registrar;
            sponsoring_registrar.id = sponsoring_registar_info.info_registrar_data.id;
            sponsoring_registrar.handle = sponsoring_registar_info.info_registrar_data.handle;
            sponsoring_registrar.name = sponsoring_registar_info.info_registrar_data.name.get_value_or_default();

            Fred::InfoRegistrarOutput create_registar_info = Fred::InfoRegistrarByHandle(
                keyset_info.info_keyset_data.create_registrar_handle).exec(ctx);
            RegistryReference create_registrar;
            create_registrar.id = create_registar_info.info_registrar_data.id;
            create_registrar.handle = create_registar_info.info_registrar_data.handle;
            create_registrar.name = create_registar_info.info_registrar_data.name.get_value_or_default();

            RegistryReference update_registrar;
            if(!keyset_info.info_keyset_data.update_registrar_handle.isnull())
            {
                Fred::InfoRegistrarOutput update_registar_info = Fred::InfoRegistrarByHandle(
                    keyset_info.info_keyset_data.update_registrar_handle.get_value()).exec(ctx);
                create_registrar.id = create_registar_info.info_registrar_data.id;
                create_registrar.handle = create_registar_info.info_registrar_data.handle;
                create_registrar.name = create_registar_info.info_registrar_data.name.get_value_or_default();
            }

            KeysetDetail detail;

            detail.id = keyset_info.info_keyset_data.id;
            detail.handle = keyset_info.info_keyset_data.handle;
            detail.roid = keyset_info.info_keyset_data.roid;
            detail.sponsoring_registrar = sponsoring_registrar;
            detail.creation_time = keyset_info.info_keyset_data.creation_time;
            detail.transfer_time = keyset_info.info_keyset_data.transfer_time;
            detail.update_time = keyset_info.info_keyset_data.update_time;

            detail.create_registrar = create_registrar;
            detail.update_registrar = update_registrar;

            detail.is_owner = false;

            detail.admins.reserve(keyset_info.info_keyset_data.tech_contacts.size());
            for(std::vector<Fred::ObjectIdHandlePair>::const_iterator ci = keyset_info.info_keyset_data.tech_contacts.begin();
                    ci != keyset_info.info_keyset_data.tech_contacts.end(); ++ci)
            {
                Fred::InfoContactOutput tech_contact_info = Fred::InfoContactById(ci->id).exec(ctx);

                RegistryReference admin;
                admin.id = tech_contact_info.info_contact_data.id;
                admin.handle = tech_contact_info.info_contact_data.handle;
                admin.name = tech_contact_info.info_contact_data.organization.get_value_or_default().empty()
                    ? tech_contact_info.info_contact_data.name.get_value_or_default()
                    : tech_contact_info.info_contact_data.organization.get_value();
                detail.admins.push_back(admin);

                if(admin.id == user_contact_id) detail.is_owner = true;//reveal authinfo
            }

            detail.authinfopw =filter_authinfo(detail.is_owner, keyset_info.info_keyset_data.authinfopw);

            detail.dnskeys.reserve(keyset_info.info_keyset_data.dns_keys.size());
            for(std::vector<Fred::DnsKey>::const_iterator ci = keyset_info.info_keyset_data.dns_keys.begin();
                    ci != keyset_info.info_keyset_data.dns_keys.end(); ++ci)
            {
                DNSKey dnskey;
                dnskey.flags = ci->get_flags();
                dnskey.protocol = ci->get_protocol();
                dnskey.alg = ci->get_alg();
                dnskey.key = ci->get_key();

                detail.dnskeys.push_back(dnskey);
            }

            get_object_states(ctx, keyset_info.info_keyset_data.id,lang
                , detail.state_codes, detail.states);

            return detail;
        }

        bool DomainBrowser::setContactDiscloseFlags(
            unsigned long long contact_id,
            const ContactDiscloseFlagsToSet& flags,
            unsigned long long request_id)
        {
            Fred::OperationContext ctx;
            Fred::InfoContactOutput contact_info = check_user_contact_id<UserNotExists>(ctx, contact_id, true);

            if(!(Fred::ObjectHasState(contact_id,Fred::ObjectState::IDENTIFIED_CONTACT).exec(ctx)
                || Fred::ObjectHasState(contact_id,Fred::ObjectState::VALIDATED_CONTACT).exec(ctx)))
            {
                throw AccessDenied();
            }

            if(Fred::ObjectHasState(contact_id,Fred::ObjectState::SERVER_BLOCKED).exec(ctx))
            {
                throw ObjectBlocked();
            }

            //when organization is set it's not allowed to hide address
            if((!contact_info.info_contact_data.organization.get_value_or_default().empty()) && (flags.address == false))
            {
                throw IncorrectUsage();
            }

            Fred::UpdateContactById update_contact(contact_id, update_registrar_);
            bool exec_update = false;
            if(flags.email != contact_info.info_contact_data.discloseemail.get_value_or_default())
            {
                update_contact.set_discloseemail(flags.email);
                exec_update = true;
            }

            if(flags.address != contact_info.info_contact_data.discloseaddress.get_value_or_default())
            {
                update_contact.set_discloseaddress(flags.address);
                exec_update = true;
            }

            if(flags.telephone != contact_info.info_contact_data.disclosetelephone.get_value_or_default())
            {
                update_contact.set_disclosetelephone(flags.telephone);
                exec_update = true;
            }

            if(flags.fax != contact_info.info_contact_data.disclosefax.get_value_or_default())
            {
                update_contact.set_disclosefax(flags.fax);
                exec_update = true;
            }

            if(flags.ident != contact_info.info_contact_data.discloseident.get_value_or_default())
            {
                update_contact.set_discloseident(flags.ident);
                exec_update = true;
            }

            if(flags.vat != contact_info.info_contact_data.disclosevat.get_value_or_default())
            {
                update_contact.set_disclosevat(flags.vat);
                exec_update = true;
            }

            if(flags.notify_email != contact_info.info_contact_data.disclosenotifyemail.get_value_or_default())
            {
                update_contact.set_disclosenotifyemail(flags.notify_email);
                exec_update = true;
            }

            if(exec_update)
            {
                update_contact.exec(ctx);
                ctx.commit_transaction();
            }
            else
            {
                return false;
            }

            return true;
        }

        bool DomainBrowser::setContactAuthInfo(
            unsigned long long user_contact_id,
            unsigned long long contact_id,
            const std::string& authinfo,
            unsigned long long request_id)
        {
            Fred::OperationContext ctx;
            Fred::InfoContactOutput contact_info = check_user_contact_id<UserNotExists>(ctx, user_contact_id, true);

            if(contact_id != contact_info.info_contact_data.id)
            {
                throw AccessDenied();
            }

            const unsigned MAX_AUTH_INFO_LENGTH = 300u;
            if(authinfo.length() > MAX_AUTH_INFO_LENGTH)
            {
                throw IncorrectUsage();
            }

            if(!(Fred::ObjectHasState(contact_id,Fred::ObjectState::IDENTIFIED_CONTACT).exec(ctx)
                || Fred::ObjectHasState(contact_id,Fred::ObjectState::VALIDATED_CONTACT).exec(ctx)))
            {
                throw AccessDenied();
            }

            if(Fred::ObjectHasState(contact_id,Fred::ObjectState::SERVER_BLOCKED).exec(ctx))
            {
                throw ObjectBlocked();
            }

            if(contact_info.info_contact_data.authinfopw.compare(authinfo) == 0)
            {
                return false;
            }

            Fred::UpdateContactById(contact_id, update_registrar_).set_authinfo(authinfo).exec(ctx);
            ctx.commit_transaction();
            return true;
        }

        bool DomainBrowser::setObjectBlockStatus(unsigned long long user_contact_id,
            const std::string& objtype,
            const std::vector<unsigned long long>& object_id,
            unsigned block_type,
            std::vector<std::string>& blocked_objects)
        {
            Fred::OperationContext ctx;
            Fred::InfoContactOutput contact_info = check_user_contact_id<UserNotExists>(ctx, user_contact_id);

            if(!Fred::ObjectHasState(user_contact_id,Fred::ObjectState::VALIDATED_CONTACT).exec(ctx))
            {
                throw AccessDenied();
            }

            unsigned long long object_type_id=0;
            try //check objtype exists and get id
            {
                object_type_id = Fred::get_object_type_id(ctx,objtype);
            }
            catch(const Fred::InternalError&)
            {
                throw IncorrectUsage();
            }

            //check input size
            if(object_id.size() == 0) return false;//nothing to do
            const unsigned SET_STATUS_MAX_ITEMS = 500u;
            if(object_id.size() > SET_STATUS_MAX_ITEMS) throw IncorrectUsage();//input too big

            //object ids made unique
            std::set<unsigned long long> object_id_set(object_id.begin(), object_id.end());

            //checked object id-handle pairs
            std::set<std::pair<unsigned long long, std::string> > object_id_name_pairs;

            //check contact object type
            if(objtype.compare("contact") == 0)
            {
                throw IncorrectUsage();//contact is not valid object type in this use case
            }
            else
            {//check ownership for other object types
                Database::QueryParams params;
                std::ostringstream object_sql;
                object_sql << "SELECT oreg.id, oreg.name FROM object_registry oreg ";

                params.push_back(object_type_id);//$1
                params.push_back(user_contact_id);//$2
                if(objtype.compare("nsset") == 0)//user have to be tech contact
                {
                    object_sql << " JOIN nsset_contact_map map ON map.nssetid = oreg.id AND map.contactid = $"
                            << params.size() << "::bigint ";
                }

                if(objtype.compare("domain") == 0)//user have to be admin contact or registrant
                {
                    object_sql << " LEFT JOIN domain_contact_map map ON map.domainid = oreg.id"
                        " JOIN domain d ON oreg.id = d.id"
                        " AND ( map.contactid = $" << params.size() << "::bigint"
                        " OR d.registrant = $" << params.size() << "::bigint) ";
                }

                if(objtype.compare("keyset") == 0)//user have to be tech contact
                {
                    object_sql << " JOIN keyset_contact_map map ON map.keysetid = oreg.id AND map.contactid = $"
                            << params.size() << "::bigint ";
                }

                object_sql << " WHERE oreg.type = $1::integer AND oreg.erdate IS NULL AND (";

                Util::HeadSeparator or_separator(""," OR ");
                for(std::set<unsigned long long>::const_iterator ci = object_id_set.begin(); ci != object_id_set.end(); ++ci)
                {
                    params.push_back(*ci);
                    object_sql << or_separator.get() << "oreg.id = $" << params.size() << "::bigint";
                }
                object_sql << ") FOR UPDATE OF oreg";

                Database::Result object_result = ctx.get_conn().exec_params(object_sql.str(), params);
                if(object_id_set.size() != object_result.size()) throw ObjectNotExists();//given objects was not found all in database belonging to user contact

                for(std::size_t i = 0; i < object_result.size(); ++i)
                {
                    object_id_name_pairs.insert(std::make_pair(static_cast<unsigned long long>(object_result[i][0]),
                        static_cast<std::string>(object_result[i][1])));
                }
            }

            for(std::set<std::pair<unsigned long long, std::string> >::const_iterator ci = object_id_name_pairs.begin()
                ; ci != object_id_name_pairs.end(); ++ci)
            {
                if((block_type == BLOCK_TRANSFER) || (block_type == BLOCK_TRANSFER_AND_UPDATE))
                {
                    if(!Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx))
                    {
                        Fred::CreateObjectStateRequestId(ci->first,
                            Util::set_of<std::string>(Fred::ObjectState::SERVER_TRANSFER_PROHIBITED)).exec(ctx);
                    }
                }

                if((block_type == UNBLOCK_TRANSFER) || (block_type == UNBLOCK_TRANSFER_AND_UPDATE))
                {
                    if(Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx))
                    {
                        Fred::CancelObjectStateRequestId(ci->first,
                            Util::set_of<std::string>(Fred::ObjectState::SERVER_TRANSFER_PROHIBITED)).exec(ctx);
                    }
                }

                if(block_type == INVALID_BLOCK_TYPE) throw InternalServerError(); //bug in implementation

                Fred::PerformObjectStateRequest(ci->first).exec(ctx);
                blocked_objects.push_back(ci->second);
            }

            ctx.commit_transaction();
            return false;
        }

    }//namespace DomainBrowserImpl
}//namespace Registry

