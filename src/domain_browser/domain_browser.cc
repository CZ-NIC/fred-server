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
 *  @file
 *  domain browser implementation
 */

#include <string>
#include <vector>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/algorithm/string.hpp>

#include "util/random.h"
#include "util/log/context.h"
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

#include "domain_browser.h"

namespace Registry
{
    namespace DomainBrowserImpl
    {
        /**
         * String for logging context
         * @param _name is server name
         * @return "server_name-<call_id>"
         */
        static const std::string create_ctx_name(const std::string &_name)
        {
            return boost::str(boost::format("%1%-<%2%>")% _name % Random::integer(0, 10000));
        }

        /**
         * Logs std::exception children and other exceptions as error, then rethrows.
         * @param ctx contains reference to database and logging interface
         */
        static void log_and_rethrow_exception_handler(Fred::OperationContext& ctx)
        {
            try
            {
                throw;
            }
            catch(const std::exception& ex)
            {
                ctx.get_log().error(ex.what());
                throw;
            }
            catch(...)
            {
                ctx.get_log().error("unknown exception");
                throw;
            }
        }

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
            Database::Result state_res = ctx.get_conn().exec_params(
                "SELECT ARRAY_TO_STRING(ARRAY_AGG(CASE WHEN eos.external THEN eosd.description ELSE NULL END ORDER BY eos.importance), '|') AS state_descs, "
                    " ARRAY_TO_STRING(ARRAY_AGG(eos.name ORDER BY eos.importance), ',') AS state_codes "
                " FROM object_state os "
                " JOIN enum_object_states eos ON eos.id = os.state_id "
                " LEFT JOIN enum_object_states_desc eosd ON os.state_id = eosd.state_id AND UPPER(eosd.lang) = UPPER($2::text) "
                " WHERE os.object_id = $1::bigint "
                    " AND os.valid_from <= CURRENT_TIMESTAMP "
                    " AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) "
                    , Database::query_param_list(object_id)(lang));

            state_codes = static_cast<std::string>(state_res[0]["state_codes"]);
            states = static_cast<std::string>(state_res[0]["state_descs"]);
        }

        std::string DomainBrowser::filter_authinfo(bool user_is_owner, const std::string& authinfopw)
        {
            if(user_is_owner)
            {
                return authinfopw;
            }

            return "********";//if not
        }

        NextDomainState DomainBrowser::getNextDomainState(
            const boost::gregorian::date& today_date,
            const boost::gregorian::date& expiration_date,
            const boost::gregorian::date& outzone_date,
            const boost::gregorian::date& delete_date)
        {
            NextDomainState next;
            if(today_date < expiration_date)
            {
                next = NextDomainState("expired", expiration_date);
            }
            else if((today_date < delete_date) || (today_date < outzone_date))
                {
                    if(outzone_date < delete_date)
                    {
                        if(today_date < outzone_date)
                        {
                            next = NextDomainState("outzone", outzone_date);
                        }
                        else
                        {
                            next = NextDomainState("deleteCandidate", delete_date);
                        }
                    }
                    else //posibly bad config
                    {
                        if(today_date < delete_date)
                        {
                            next = NextDomainState("deleteCandidate", delete_date);
                        }
                        else
                        {
                            next = NextDomainState("outzone", outzone_date);
                        }
                    }
                }
            return next;
        }

        DomainBrowser::DomainBrowser(const std::string& server_name,
            const std::string& update_registrar_handle,
            unsigned int domain_list_limit,
            unsigned int nsset_list_limit,
            unsigned int keyset_list_limit)
        : server_name_(server_name)
        , update_registrar_(update_registrar_handle)
        , domain_list_limit_(domain_list_limit)
        , nsset_list_limit_(nsset_list_limit)
        , keyset_list_limit_(keyset_list_limit)
        {
            Logging::Context lctx_server(server_name_);
            Logging::Context lctx("init");
            Fred::OperationContext ctx;
            Database::Result db_config = ctx.get_conn().exec(
                "SELECT MAX(importance) * 2 AS minimal_status_importance FROM enum_object_states");
            minimal_status_importance_ = static_cast<unsigned int>(db_config[0]["minimal_status_importance"]);
        }

        DomainBrowser::~DomainBrowser()
        {}

        //exception with dummy set handle
        struct ObjectNotExistsWithDummyHandleSetter : ObjectNotExists
        {
            ObjectNotExistsWithDummyHandleSetter& set_handle(const std::string&)
            {
                return *this;
            }
        };

        unsigned long long DomainBrowser::getObjectRegistryId(const std::string& objtype, const std::string& handle)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-object-registry-id");
            Fred::OperationContext ctx;
            try
            {
                try
                {
                    get_object_type_id(ctx, objtype);
                }
                catch(const std::exception&)
                {
                    throw IncorrectUsage();
                }

                return Fred::get_object_id_by_handle_and_type_with_lock(ctx,handle, objtype,
                    static_cast<ObjectNotExistsWithDummyHandleSetter*>(NULL),
                    &ObjectNotExistsWithDummyHandleSetter::set_handle);
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return 0;
        }

        RegistrarDetail DomainBrowser::getRegistrarDetail(
            unsigned long long user_contact_id,
            const std::string& registrar_handle)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-registrar-detail");
            Fred::OperationContext ctx;
            try
            {
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
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return RegistrarDetail();
        }

        std::string DomainBrowser::get_server_name()
        {
            return server_name_;
        }

        ContactDetail DomainBrowser::getContactDetail(unsigned long long user_contact_id,
                unsigned long long contact_id,
                const std::string& lang)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-contact-detail");
            Fred::OperationContext ctx;
            try
            {
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
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return ContactDetail();
        }

        DomainDetail DomainBrowser::getDomainDetail(unsigned long long user_contact_id,
                unsigned long long domain_id,
                const std::string& lang)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-domain-detail");
            Fred::OperationContext ctx;
            try
            {
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

                detail.registrant = registrant;
                detail.expiration_date = domain_info.info_domain_data.expiration_date;
                detail.enum_domain_validation = domain_info.info_domain_data.enum_domain_validation;
                detail.nsset = nsset;
                detail.keyset = keyset;

                bool set_authinfo = detail.is_owner;
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

                    if(admin.id == user_contact_id) set_authinfo = true;//reveal authinfo to admin
                }

                detail.authinfopw =filter_authinfo(set_authinfo, domain_info.info_domain_data.authinfopw);

                get_object_states(ctx, domain_info.info_domain_data.id,lang
                    , detail.state_codes, detail.states);

                return detail;
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return DomainDetail();
        }

        NssetDetail DomainBrowser::getNssetDetail(unsigned long long user_contact_id,
                unsigned long long nsset_id,
                const std::string& lang)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-nsset-detail");
            Fred::OperationContext ctx;
            try
            {
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
                    update_registrar.id = update_registar_info.info_registrar_data.id;
                    update_registrar.handle = update_registar_info.info_registrar_data.handle;
                    update_registrar.name = update_registar_info.info_registrar_data.name.get_value_or_default();
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
                    host.inet_addr = boost::algorithm::join(ci->get_inet_addr(), ", ");

                    detail.hosts.push_back(host);
                }

                get_object_states(ctx, nsset_info.info_nsset_data.id,lang
                    , detail.state_codes, detail.states);

                detail.report_level = nsset_info.info_nsset_data.tech_check_level.get_value_or_default();

                return detail;
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return NssetDetail();
        }

        KeysetDetail DomainBrowser::getKeysetDetail(unsigned long long user_contact_id,
                unsigned long long keyset_id,
                const std::string& lang)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-keyset-detail");
            Fred::OperationContext ctx;

            try
            {
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
                    update_registrar.id = update_registar_info.info_registrar_data.id;
                    update_registrar.handle = update_registar_info.info_registrar_data.handle;
                    update_registrar.name = update_registar_info.info_registrar_data.name.get_value_or_default();
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
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return KeysetDetail();
        }

        bool DomainBrowser::setContactDiscloseFlags(
            unsigned long long contact_id,
            const ContactDiscloseFlagsToSet& flags,
            unsigned long long request_id)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("set-contact-disclose-flags");
            Fred::OperationContext ctx;
            try
            {
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
                    update_contact.set_logd_request_id(request_id).exec(ctx);
                    ctx.commit_transaction();
                }
                else
                {
                    return false;
                }
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return true;
        }

        bool DomainBrowser::setContactAuthInfo(
            unsigned long long user_contact_id,
            unsigned long long contact_id,
            const std::string& authinfo,
            unsigned long long request_id)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("set-contact-auth-info");
            Fred::OperationContext ctx;
            try
            {
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

                if(contact_info.info_contact_data.authinfopw == authinfo)
                {
                    return false;
                }

                Fred::UpdateContactById(contact_id, update_registrar_).set_authinfo(authinfo).set_logd_request_id(request_id).exec(ctx);
                ctx.commit_transaction();
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return true;
        }

        bool DomainBrowser::setObjectBlockStatus(unsigned long long user_contact_id,
            const std::string& objtype,
            const std::vector<unsigned long long>& object_id,
            unsigned block_type,
            std::vector<std::string>& blocked_objects)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("set-object-block-status");
            Fred::OperationContext ctx;
            try
            {
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
                if(objtype == "contact")
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
                    if(objtype == "nsset")//user have to be tech contact
                    {
                        object_sql << " JOIN nsset_contact_map map ON map.nssetid = oreg.id AND map.contactid = $"
                                << params.size() << "::bigint ";
                    }
                    else if(objtype == "domain")//user have to be admin contact or registrant
                    {
                        object_sql << " LEFT JOIN domain d ON oreg.id = d.id AND d.registrant = $" << params.size() << "::bigint "
                            " LEFT JOIN domain_contact_map map ON map.domainid = oreg.id AND map.contactid = $" << params.size() << "::bigint ";
                    }
                    else if(objtype == "keyset")//user have to be tech contact
                    {
                        object_sql << " JOIN keyset_contact_map map ON map.keysetid = oreg.id AND map.contactid = $"
                                << params.size() << "::bigint ";
                    }
                    else
                    {
                        throw InternalServerError();//unknown object type, should'v been checked before
                    }

                    object_sql << " WHERE oreg.type = $1::integer AND oreg.erdate IS NULL ";

                    if(objtype == "domain")//user have to be admin contact or registrant
                    {
                        object_sql << " AND (d.id IS NOT NULL OR map.domainid IS NOT NULL) ";
                    }

                    object_sql << " AND (";

                    Util::HeadSeparator or_separator(""," OR ");
                    for(std::set<unsigned long long>::const_iterator ci = object_id_set.begin(); ci != object_id_set.end(); ++ci)
                    {
                        params.push_back(*ci);
                        object_sql << or_separator.get() << "oreg.id = $" << params.size() << "::bigint";
                    }
                    object_sql << ") FOR SHARE OF oreg";//lock to prevent change of linked contacts

                    Database::Result object_result = ctx.get_conn().exec_params(object_sql.str(), params);
                    if(object_id_set.size() != object_result.size()) throw ObjectNotExists();//given objects was not found all in database belonging to user contact

                    for(std::size_t i = 0; i < object_result.size(); ++i)
                    {
                        object_id_name_pairs.insert(std::make_pair(static_cast<unsigned long long>(object_result[i][0]),
                            static_cast<std::string>(object_result[i][1])));
                    }
                }

                bool retval = false;
                for(std::set<std::pair<unsigned long long, std::string> >::const_iterator ci = object_id_name_pairs.begin()
                    ; ci != object_id_name_pairs.end(); ++ci)
                {
                    Fred::OperationContext ctx_per_object;
                    if(Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_BLOCKED).exec(ctx_per_object))//object administratively blocked
                    {
                        blocked_objects.push_back(ci->second);
                        continue;
                    }

                    if((block_type == UNBLOCK_TRANSFER)
                        && Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx_per_object))//forbidden partial unblocking
                    {
                        blocked_objects.push_back(ci->second);
                        continue;
                    }

                    if((block_type == BLOCK_TRANSFER) || (block_type == BLOCK_TRANSFER_AND_UPDATE))//block transfer
                    {
                        if(!Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx_per_object))
                        {
                            Fred::CreateObjectStateRequestId(ci->first,
                                Util::set_of<std::string>(Fred::ObjectState::SERVER_TRANSFER_PROHIBITED)).exec(ctx_per_object);
                        }
                    }

                    if(block_type == BLOCK_TRANSFER_AND_UPDATE)//block update
                    {
                        if(!Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx_per_object))
                        {
                            Fred::CreateObjectStateRequestId(ci->first,
                                Util::set_of<std::string>(Fred::ObjectState::SERVER_UPDATE_PROHIBITED)).exec(ctx_per_object);
                        }
                    }

                    if((block_type == UNBLOCK_TRANSFER) || (block_type == UNBLOCK_TRANSFER_AND_UPDATE))//unblock transfer
                    {
                        if(Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_TRANSFER_PROHIBITED).exec(ctx_per_object))
                        {
                            Fred::CancelObjectStateRequestId(ci->first,
                                Util::set_of<std::string>(Fred::ObjectState::SERVER_TRANSFER_PROHIBITED)).exec(ctx_per_object);
                        }
                    }

                    if(block_type == UNBLOCK_TRANSFER_AND_UPDATE)//unblock transfer and update
                    {
                        if(Fred::ObjectHasState(ci->first, Fred::ObjectState::SERVER_UPDATE_PROHIBITED).exec(ctx_per_object))
                        {
                            Fred::CancelObjectStateRequestId(ci->first,
                                Util::set_of<std::string>(Fred::ObjectState::SERVER_UPDATE_PROHIBITED)).exec(ctx_per_object);
                        }
                    }

                    if(block_type == INVALID_BLOCK_TYPE) throw InternalServerError(); //bug in implementation

                    Fred::PerformObjectStateRequest(ci->first).exec(ctx_per_object);
                    ctx_per_object.commit_transaction();
                    retval = true;//ok at least one object have required state
                }

                ctx.commit_transaction();
                return retval;
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return false;
        }

        bool DomainBrowser::getDomainList(unsigned long long user_contact_id,
            const Optional<unsigned long long>& list_domains_for_nsset_id,
            const Optional<unsigned long long>& list_domains_for_keyset_id,
            const std::string& lang,
            unsigned long long offset,
            std::vector<std::vector<std::string> >& domain_list_out)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-domain-list");
            Fred::OperationContext ctx;
            try
            {
                check_user_contact_id<UserNotExists>(ctx, user_contact_id);

                if(list_domains_for_nsset_id.isset())
                {
                    //check nsset owned by user contact
                    Database::Result nsset_ownership_result = ctx.get_conn().exec_params(
                    "SELECT * FROM nsset_contact_map WHERE nssetid = $1::bigint AND contactid = $2::bigint"
                    , Database::query_param_list (list_domains_for_nsset_id.get_value())(user_contact_id));
                    if(nsset_ownership_result.size() == 0) throw AccessDenied();
                }

                if(list_domains_for_keyset_id.isset())
                {
                    //check keyset owned by user contact
                    Database::Result keyset_ownership_result = ctx.get_conn().exec_params(
                    "SELECT * FROM keyset_contact_map WHERE keysetid = $1::bigint AND contactid = $2::bigint"
                    , Database::query_param_list (list_domains_for_keyset_id.get_value())(user_contact_id));
                    if(keyset_ownership_result.size() == 0) throw AccessDenied();
                }

                Database::QueryParams params;
                std::ostringstream sql;
                params.push_back(user_contact_id);
                sql <<  "SELECT domain_list.id, domain_list.fqdn, domain_list.registrar_handle, domain_list.registrar_name, "
                        " domain_list.expiration_date, domain_list.registrant_id, domain_list.have_keyset, domain_list.user_role, "
                        " domain_list.today_date, domain_list.outzone_date,domain_list.delete_date, "
                        " COALESCE(bit_or(CASE WHEN eos.external THEN eos.importance ELSE NULL END),0) AS external_importance, "
                        " SUM(CASE WHEN eos.name = 'serverBlocked' THEN 1 ELSE 0 END) AS is_server_blocked, "
                        " array_to_string(ARRAY_AGG((CASE WHEN eos.external THEN eosd.description ELSE NULL END) ORDER BY eos.importance), '|') AS state_desc "
                        " FROM "
                        "(SELECT "
                        "oreg.id AS id, "
                        "oreg.name AS fqdn, "
                        "registrar.handle AS registrar_handle, "
                        "registrar.name AS registrar_name, "
                        "domain.exdate AS expiration_date, "
                        "domain.registrant AS registrant_id, "
                        "domain.keyset IS NOT NULL AS have_keyset, "
                        "CASE WHEN domain.registrant = $" << params.size() << "::bigint THEN 'holder'::text ELSE "
                            "CASE WHEN domain_contact_map.contactid = $" << params.size() << "::bigint THEN 'admin'::text ELSE ''::text END "
                        "END AS user_role,"
                        "CURRENT_DATE AS today_date, "
                        "(domain.exdate + (SELECT val || ' day' FROM enum_parameters "
                        " WHERE name = 'expiration_dns_protection_period')::interval)::date as outzone_date, "
                        "(domain.exdate + (SELECT val || ' day' FROM enum_parameters "
                        " WHERE name = 'expiration_registration_protection_period')::interval)::date as delete_date "
                    "FROM object_registry oreg "
                    "JOIN domain ON oreg.id = domain.id "
                    "JOIN object ON object.id = oreg.id "
                    "JOIN registrar ON registrar.id = object.clid "
                    "LEFT JOIN domain_contact_map ON domain_contact_map.domainid = domain.id "
                        "AND domain_contact_map.role = 1 " //admin
                        "AND domain_contact_map.contactid = $" << params.size() << "::bigint "
                    "WHERE oreg.type = (SELECT id FROM enum_object_type eot WHERE eot.name='domain'::text) AND oreg.erdate IS NULL ";

                    if(!list_domains_for_nsset_id.isset() && !list_domains_for_keyset_id.isset())
                    {   //select domains related to user_contact_id
                        sql << "AND (domain_contact_map.contactid = $" << params.size() << "::bigint "
                                "OR domain.registrant = $" << params.size() << "::bigint) ";
                    }

                    if(list_domains_for_nsset_id.isset())
                    {   //select domains with given nsset
                        params.push_back(list_domains_for_nsset_id.get_value());
                        sql << "AND domain.nsset = $" << params.size() << "::bigint ";
                    }

                    if(list_domains_for_keyset_id.isset())
                    {   //select domains with given keyset
                        params.push_back(list_domains_for_keyset_id.get_value());
                        sql << "AND domain.keyset = $" << params.size() << "::bigint ";
                    }

                params.push_back(lang);
                sql <<" ) AS domain_list "
                " LEFT JOIN object_state os ON os.object_id = domain_list.id AND os.valid_from <= CURRENT_TIMESTAMP AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) "
                " LEFT JOIN enum_object_states eos ON eos.id = os.state_id "
                " LEFT JOIN enum_object_states_desc eosd ON os.state_id = eosd.state_id AND UPPER(eosd.lang) = UPPER($" << params.size() << "::text) "//lang
                " GROUP BY domain_list.id, domain_list.fqdn, domain_list.registrar_handle, domain_list.registrar_name, "
                " domain_list.expiration_date, domain_list.registrant_id, domain_list.have_keyset, domain_list.user_role, "
                " domain_list.today_date, domain_list.outzone_date,domain_list.delete_date ";

                params.push_back(domain_list_limit_+1);
                sql << "ORDER BY domain_list.expiration_date,domain_list.id "
                    "LIMIT $" << params.size() << "::bigint ";

                params.push_back(offset);
                sql << "OFFSET $" << params.size() << "::bigint ";

                Database::Result domain_list_result = ctx.get_conn().exec_params(sql.str(), params);

                unsigned long long limited_domain_list_size = (domain_list_result.size() > domain_list_limit_)
                    ? domain_list_limit_ : domain_list_result.size();

                domain_list_out.reserve(limited_domain_list_size);
                for (unsigned long long i = 0;i < limited_domain_list_size;++i)
                {
                    std::vector<std::string> row(11);
                    row.at(0) = static_cast<std::string>(domain_list_result[i]["id"]);
                    row.at(1) = static_cast<std::string>(domain_list_result[i]["fqdn"]);

                    unsigned int external_status_importance = static_cast<unsigned int>(domain_list_result[i]["external_importance"]);
                    row.at(2) = boost::lexical_cast<std::string>(external_status_importance == 0 ? minimal_status_importance_ : external_status_importance);

                    boost::gregorian::date today_date = domain_list_result[i]["today_date"].isnull() ? boost::gregorian::date()
                                : boost::gregorian::from_string(static_cast<std::string>(domain_list_result[i]["today_date"]));
                    boost::gregorian::date expiration_date = domain_list_result[i]["expiration_date"].isnull() ? boost::gregorian::date()
                                : boost::gregorian::from_string(static_cast<std::string>(domain_list_result[i]["expiration_date"]));
                    boost::gregorian::date outzone_date = domain_list_result[i]["outzone_date"].isnull() ? boost::gregorian::date()
                                : boost::gregorian::from_string(static_cast<std::string>(domain_list_result[i]["outzone_date"]));
                    boost::gregorian::date delete_date = domain_list_result[i]["delete_date"].isnull() ? boost::gregorian::date()
                                : boost::gregorian::from_string(static_cast<std::string>(domain_list_result[i]["delete_date"]));
                    NextDomainState next = getNextDomainState(today_date,expiration_date,outzone_date,delete_date);

                    row.at(3) = next.state;
                    row.at(4) = boost::gregorian::to_iso_extended_string(next.state_date);
                    row.at(5) = static_cast<std::string>(domain_list_result[i]["have_keyset"]);
                    row.at(6) = static_cast<std::string>(domain_list_result[i]["user_role"]);
                    row.at(7) = static_cast<std::string>(domain_list_result[i]["registrar_handle"]);
                    row.at(8) = static_cast<std::string>(domain_list_result[i]["registrar_name"]);

                    row.at(9) = static_cast<std::string>(domain_list_result[i]["state_desc"]);

                    bool server_blocked = static_cast<unsigned int>(domain_list_result[i]["is_server_blocked"]);
                    row.at(10) = server_blocked ? "t":"f";

                    domain_list_out.push_back(row);
                }

                return domain_list_result.size() > domain_list_limit_;
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return false;
        }

        bool DomainBrowser::getNssetList(unsigned long long user_contact_id,
                    const std::string& lang,
                    unsigned long long offset,
                    std::vector<std::vector<std::string> >& nsset_list_out)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-nsset-list");
            Fred::OperationContext ctx;
            try
            {
                check_user_contact_id<UserNotExists>(ctx, user_contact_id);

                Database::Result nsset_list_result = ctx.get_conn().exec_params(
                    "SELECT nsset_list.id, nsset_list.handle, "
                        " nsset_list.registrar_handle, nsset_list.registrar_name "
                        " , nsset_list.domain_number, "
                    " COALESCE(BIT_OR(CASE WHEN eos.external THEN eos.importance ELSE NULL END), 0) AS external_importance, "
                    " SUM(CASE WHEN eos.name = 'serverBlocked' THEN 1 ELSE 0 END) AS is_server_blocked, "
                    " ARRAY_TO_STRING(ARRAY_AGG((CASE WHEN eos.external THEN eosd.description ELSE NULL END) ORDER BY eos.importance), '|') AS state_desc "
                    "FROM "
                    "(SELECT oreg.id AS id "
                    ", oreg.name AS handle "
                    ", registrar.handle AS registrar_handle "
                    ", registrar.name AS registrar_name "
                    ", COALESCE(domains.number,0) AS domain_number "
                    " FROM object_registry oreg "
                    " JOIN object obj ON obj.id = oreg.id "
                    " JOIN registrar ON registrar.id = obj.clid "
                    " JOIN nsset_contact_map ncm ON ncm.nssetid = oreg.id "
                    " LEFT JOIN (SELECT d.nsset AS nsset, count(d.id) AS number FROM domain d "
                    " JOIN nsset_contact_map ncm ON  d.nsset = ncm.nssetid "
                    " WHERE ncm.contactid = $1::bigint "
                    " GROUP BY d.nsset) AS domains ON domains.nsset = oreg.id "
                    " WHERE ncm.contactid = $1::bigint "
                    " AND oreg.type = (SELECT id FROM enum_object_type eot WHERE eot.name='nsset'::text) AND oreg.erdate IS NULL "
                    ") AS nsset_list "
                    " LEFT JOIN object_state os ON os.object_id = nsset_list.id "
                        " AND os.valid_from <= CURRENT_TIMESTAMP "
                        " AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) "
                    " LEFT JOIN enum_object_states eos ON eos.id = os.state_id "
                    " LEFT JOIN enum_object_states_desc eosd ON os.state_id = eosd.state_id AND UPPER(eosd.lang) = UPPER($4::text) "//lang
                    " GROUP BY nsset_list.id, nsset_list.handle, "
                        " nsset_list.registrar_handle, nsset_list.registrar_name "
                        " , nsset_list.domain_number "
                    " ORDER BY nsset_list.id "
                    " LIMIT $2::bigint OFFSET $3::bigint ",
                    Database::query_param_list(user_contact_id)(nsset_list_limit_+1)(offset)(lang));

                unsigned long long limited_nsset_list_size = (nsset_list_result.size() > nsset_list_limit_)
                    ? nsset_list_limit_ : nsset_list_result.size();

                nsset_list_out.reserve(limited_nsset_list_size);
                for (unsigned long long i = 0;i < limited_nsset_list_size;++i)
                {
                    std::vector<std::string> row(8);
                    row.at(0) = static_cast<std::string>(nsset_list_result[i]["id"]);
                    row.at(1) = static_cast<std::string>(nsset_list_result[i]["handle"]);
                    row.at(2) = static_cast<std::string>(nsset_list_result[i]["domain_number"]);
                    row.at(3) = static_cast<std::string>(nsset_list_result[i]["registrar_handle"]);
                    row.at(4) = static_cast<std::string>(nsset_list_result[i]["registrar_name"]);

                    unsigned int external_status_importance = static_cast<unsigned int>(nsset_list_result[i]["external_importance"]);
                    row.at(5) = boost::lexical_cast<std::string>(external_status_importance == 0 ? minimal_status_importance_ : external_status_importance);

                    row.at(6) = static_cast<std::string>(nsset_list_result[i]["state_desc"]);

                    bool server_blocked = static_cast<unsigned int>(nsset_list_result[i]["is_server_blocked"]);
                    row.at(7) = server_blocked ? "t":"f";

                    nsset_list_out.push_back(row);
                }

                return nsset_list_result.size() > nsset_list_limit_;
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return false;
        }

        bool DomainBrowser::getKeysetList(unsigned long long user_contact_id,
            const std::string& lang,
            unsigned long long offset,
            std::vector<std::vector<std::string> >& keyset_list_out)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-keyset-list");
            Fred::OperationContext ctx;
            try
            {
                check_user_contact_id<UserNotExists>(ctx, user_contact_id);

                Database::Result keyset_list_result = ctx.get_conn().exec_params(
                    "SELECT keyset_list.id, keyset_list.handle, "
                        " keyset_list.registrar_handle, keyset_list.registrar_name "
                        " , keyset_list.domain_number, "
                    " COALESCE(BIT_OR(CASE WHEN eos.external THEN eos.importance ELSE NULL END), 0) AS external_importance, "
                    " SUM(CASE WHEN eos.name = 'serverBlocked' THEN 1 ELSE 0 END) AS is_server_blocked, "
                    " ARRAY_TO_STRING(ARRAY_AGG((CASE WHEN eos.external THEN eosd.description ELSE NULL END) ORDER BY eos.importance), '|') AS state_desc "
                    "FROM "
                    "(SELECT oreg.id AS id "
                    ", oreg.name AS handle "
                    ", registrar.handle AS registrar_handle "
                    ", registrar.name AS registrar_name "
                    ", COALESCE(domains.number,0) AS domain_number "
                    " FROM object_registry oreg "
                    " JOIN object obj ON obj.id = oreg.id "
                    " JOIN registrar ON registrar.id = obj.clid "
                    " JOIN keyset_contact_map kcm ON kcm.keysetid = oreg.id "
                    " LEFT JOIN (SELECT d.keyset AS keyset, count(d.id) AS number FROM domain d "
                    " JOIN keyset_contact_map kcm ON  d.keyset = kcm.keysetid "
                    " WHERE kcm.contactid = $1::bigint "
                    " GROUP BY d.keyset) AS domains ON domains.keyset = oreg.id "
                    " WHERE kcm.contactid = $1::bigint "
                    " AND oreg.type = (SELECT id FROM enum_object_type eot WHERE eot.name='keyset'::text) AND oreg.erdate IS NULL "
                    ") AS keyset_list "
                    " LEFT JOIN object_state os ON os.object_id = keyset_list.id "
                        " AND os.valid_from <= CURRENT_TIMESTAMP "
                        " AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) "
                    " LEFT JOIN enum_object_states eos ON eos.id = os.state_id "
                    " LEFT JOIN enum_object_states_desc eosd ON os.state_id = eosd.state_id AND UPPER(eosd.lang) = UPPER($4::text) "//lang
                    " GROUP BY keyset_list.id, keyset_list.handle, "
                        " keyset_list.registrar_handle, keyset_list.registrar_name "
                        " , keyset_list.domain_number "
                    " ORDER BY keyset_list.id "
                    " LIMIT $2::bigint OFFSET $3::bigint ",
                    Database::query_param_list(user_contact_id)(keyset_list_limit_+1)(offset)(lang));

                unsigned long long limited_keyset_list_size = (keyset_list_result.size() > keyset_list_limit_)
                    ? keyset_list_limit_ : keyset_list_result.size();

                keyset_list_out.reserve(limited_keyset_list_size);
                for (unsigned long long i = 0;i < limited_keyset_list_size;++i)
                {
                    std::vector<std::string> row(8);
                    row.at(0) = static_cast<std::string>(keyset_list_result[i]["id"]);
                    row.at(1) = static_cast<std::string>(keyset_list_result[i]["handle"]);
                    row.at(2) = static_cast<std::string>(keyset_list_result[i]["domain_number"]);
                    row.at(3) = static_cast<std::string>(keyset_list_result[i]["registrar_handle"]);
                    row.at(4) = static_cast<std::string>(keyset_list_result[i]["registrar_name"]);

                    unsigned int external_status_importance = static_cast<unsigned int>(keyset_list_result[i]["external_importance"]);
                    row.at(5) = boost::lexical_cast<std::string>(external_status_importance == 0 ? minimal_status_importance_ : external_status_importance);

                    row.at(6) = static_cast<std::string>(keyset_list_result[i]["state_desc"]);

                    bool server_blocked = static_cast<unsigned int>(keyset_list_result[i]["is_server_blocked"]);
                    row.at(7) = server_blocked ? "t":"f";

                    keyset_list_out.push_back(row);
                }

                return keyset_list_result.size() > keyset_list_limit_;
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
            return false;
        }


        void DomainBrowser::getPublicStatusDesc(const std::string& lang,
            std::vector<std::string>& status_description_out)
        {
            Logging::Context lctx_server(create_ctx_name(get_server_name()));
            Logging::Context lctx("get-public-status-desc");
            Fred::OperationContext ctx;
            try
            {
                std::map<unsigned long long, std::string> state_desc_map = Fred::GetObjectStateDescriptions(lang)
                    .set_external().exec(ctx);

                status_description_out.reserve(state_desc_map.size());
                for(std::map<unsigned long long, std::string>::const_iterator ci = state_desc_map.begin()
                    ; ci != state_desc_map.end(); ++ci) status_description_out.push_back(ci->second);
            }
            catch(...)
            {
                log_and_rethrow_exception_handler(ctx);
            }
        }

    }//namespace DomainBrowserImpl
}//namespace Registry

