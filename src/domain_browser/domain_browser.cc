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
        //dummy decl - impl
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

            //check user_contact_id
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
            result.name = registar_info.info_registrar_data.name.get_value();
            result.phone = registar_info.info_registrar_data.telephone.get_value();
            result.fax = registar_info.info_registrar_data.fax.get_value();
            result.url = registar_info.info_registrar_data.url.get_value();

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


    }//namespace DomainBrowserImpl
}//namespace Registry

