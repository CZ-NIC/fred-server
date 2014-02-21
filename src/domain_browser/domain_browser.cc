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

#include "src/fredlib/opcontext.h"
#include "src/fredlib/opexception.h"
#include "src/fredlib/object/object_impl.h"
#include "src/fredlib/object_state/object_has_state.h"
#include "src/fredlib/object_state/object_state_name.h"
#include "src/fredlib/registrar/info_registrar.h"
#include "src/fredlib/contact/info_contact.h"
#include "domain_browser.h"

namespace Registry
{
    namespace DomainBrowserImpl
    {
        void DomainBrowser::check_user_contact_id(Fred::OperationContext& ctx, unsigned long long user_contact_id)
        {
            try
            {
                Fred::InfoContactById(user_contact_id).set_lock(true).exec(ctx);
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
                contact_info = Fred::InfoContactById(contact_id).set_lock(true).exec(ctx);
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

            Database::Result contact_states_result = ctx.get_conn().exec_params(
            "SELECT eos.name, COALESCE(osd.description, '') "
            " FROM object_state os "
                " JOIN enum_object_states eos ON eos.id = os.state_id "
                " JOIN enum_object_states_desc osd ON osd.state_id = eos.id AND lang = $2::text "
                " WHERE os.object_id = $1::bigint "
                " AND eos.importance > 0 "
                " AND eos.external = TRUE "
                    " AND os.valid_from <= CURRENT_TIMESTAMP "
                    " AND (os.valid_to IS NULL OR os.valid_to > CURRENT_TIMESTAMP) "
                " ORDER BY eos.importance "
            , Database::query_param_list(contact_info.info_contact_data.id)(lang)
            );

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

            Util::HeadSeparator states_separator("","|");
            Util::HeadSeparator state_codes_separator("",",");
            for(unsigned long long i = 0; i < contact_states_result.size(); ++i)
            {
                detail.states += states_separator.get();
                detail.states += static_cast<std::string>(contact_states_result[i][0]);

                detail.state_codes += state_codes_separator.get();
                detail.state_codes += static_cast<std::string>(contact_states_result[i][1]);
            }

            return detail;
        }


    }//namespace DomainBrowserImpl
}//namespace Registry

