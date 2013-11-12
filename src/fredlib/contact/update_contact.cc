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
 *  contact update
 */

#include <string>
#include <vector>
#include <sstream>

#include <boost/algorithm/string.hpp>

#include "fredlib/contact/update_contact.h"
#include "fredlib/object/object.h"
#include "fredlib/contact/contact_enum.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    UpdateContactById::UpdateContactById(unsigned long long id, const std::string& registrar)
        : UpdateContact<UpdateContactById>(registrar)
        , id_(id)
        , select_contact_by_id_(Fred::InfoContactById(id_).set_lock())
        {}

    UpdateContactById::UpdateContactById(unsigned long long id
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& authinfo
            , const Optional<std::string>& name
            , const Optional<std::string>& organization
            , const Optional<std::string>& street1
            , const Optional<std::string>& street2
            , const Optional<std::string>& street3
            , const Optional<std::string>& city
            , const Optional<std::string>& stateorprovince
            , const Optional<std::string>& postalcode
            , const Optional<std::string>& country
            , const Optional<std::string>& telephone
            , const Optional<std::string>& fax
            , const Optional<std::string>& email
            , const Optional<std::string>& notifyemail
            , const Optional<std::string>& vat
            , const Optional<std::string>& ssntype
            , const Optional<std::string>& ssn
            , const Optional<bool>& disclosename
            , const Optional<bool>& discloseorganization
            , const Optional<bool>& discloseaddress
            , const Optional<bool>& disclosetelephone
            , const Optional<bool>& disclosefax
            , const Optional<bool>& discloseemail
            , const Optional<bool>& disclosevat
            , const Optional<bool>& discloseident
            , const Optional<bool>& disclosenotifyemail
            , const Optional<unsigned long long>& logd_request_id
            )
    : UpdateContact<UpdateContactById>(registrar
              , sponsoring_registrar
              , authinfo
              , name
              , organization
              , street1
              , street2
              , street3
              , city
              , stateorprovince
              , postalcode
              , country
              , telephone
              , fax
              , email
              , notifyemail
              , vat
              , ssntype
              , ssn
              , disclosename
              , discloseorganization
              , discloseaddress
              , disclosetelephone
              , disclosefax
              , discloseemail
              , disclosevat
              , discloseident
              , disclosenotifyemail
              , logd_request_id
              )
    , id_(id)
    , select_contact_by_id_(Fred::InfoContactById(id_).set_lock())
    {}

    unsigned long long UpdateContactById::exec(Fred::OperationContext& ctx)
    {
        unsigned long long history_id = 0;//to return

        try
        {
            ExceptionType update_exception;
            Fred::InfoContactOutput contact;//contact selected for update
            try
            {
                contact = select_contact_by_id_.exec(ctx);
            }
            catch(const Fred::InfoContactById::Exception& info_exception)
            {
                if(info_exception.is_set_unknown_object_id())
                {
                    update_exception.set_unknown_contact_id(
                        info_exception.get_unknown_object_id());
                }
                else throw;//rethrow unexpected
            }

            try
            {
                history_id = UpdateContact<UpdateContactById>::exec(ctx,contact);
            }
            catch(const UpdateContact<UpdateContactById>::Exception& update_contact_exception)
            {
                if(update_contact_exception.is_set_unknown_registrar_handle())
                {
                    //fatal good path, need valid registrar performing update
                    BOOST_THROW_EXCEPTION(ExceptionType().set_unknown_registrar_handle(
                            update_contact_exception.get_unknown_registrar_handle()));
                }
                else if(update_contact_exception.is_set_unknown_contact_handle()
                    ||  update_contact_exception.is_set_unknown_sponsoring_registrar_handle()
                    ||  update_contact_exception.is_set_unknown_ssntype()
                    ||  update_contact_exception.is_set_unknown_country()
                    )//non-fatal good path, update can continue to check input
                {
                    if(update_contact_exception.is_set_unknown_contact_handle())
                    {
                        update_exception.set_unknown_contact_id(id_);
                    }

                    if(update_contact_exception.is_set_unknown_sponsoring_registrar_handle())
                    {
                        update_exception.set_unknown_sponsoring_registrar_handle(
                                update_contact_exception.get_unknown_sponsoring_registrar_handle());
                    }

                    if(update_contact_exception.is_set_unknown_ssntype())
                    {
                        update_exception.set_unknown_ssntype(
                                update_contact_exception.get_unknown_ssntype());
                    }

                    if(update_contact_exception.is_set_unknown_country())
                    {
                        update_exception.set_unknown_country(
                                update_contact_exception.get_unknown_country());
                    }
                }
                else throw;//rethrow unexpected
            }

            if(update_exception.throw_me())
            {
                BOOST_THROW_EXCEPTION(update_exception);
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }

    std::string UpdateContactById::to_string() const
    {
        return Util::format_operation_state("UpdateContactById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("select_contact_by_id", select_contact_by_id_.to_string()))//member operation
        (std::make_pair("UpdateContact<UpdateContactById>",UpdateContact<UpdateContactById>::to_string()))//parent operation
        );
    }

    UpdateContactByHandle::UpdateContactByHandle(const std::string& handle, const std::string& registrar)
        : UpdateContact<UpdateContactByHandle>(registrar)
        , handle_(handle)
        , select_contact_by_handle_(Fred::InfoContactByHandle(handle_).set_lock())
        {}

    UpdateContactByHandle::UpdateContactByHandle(const std::string& handle
            , const std::string& registrar
            , const Optional<std::string>& sponsoring_registrar
            , const Optional<std::string>& authinfo
            , const Optional<std::string>& name
            , const Optional<std::string>& organization
            , const Optional<std::string>& street1
            , const Optional<std::string>& street2
            , const Optional<std::string>& street3
            , const Optional<std::string>& city
            , const Optional<std::string>& stateorprovince
            , const Optional<std::string>& postalcode
            , const Optional<std::string>& country
            , const Optional<std::string>& telephone
            , const Optional<std::string>& fax
            , const Optional<std::string>& email
            , const Optional<std::string>& notifyemail
            , const Optional<std::string>& vat
            , const Optional<std::string>& ssntype
            , const Optional<std::string>& ssn
            , const Optional<bool>& disclosename
            , const Optional<bool>& discloseorganization
            , const Optional<bool>& discloseaddress
            , const Optional<bool>& disclosetelephone
            , const Optional<bool>& disclosefax
            , const Optional<bool>& discloseemail
            , const Optional<bool>& disclosevat
            , const Optional<bool>& discloseident
            , const Optional<bool>& disclosenotifyemail
            , const Optional<unsigned long long> logd_request_id
            )
    : UpdateContact<UpdateContactByHandle>(registrar
            , sponsoring_registrar
            , authinfo
            , name
            , organization
            , street1
            , street2
            , street3
            , city
            , stateorprovince
            , postalcode
            , country
            , telephone
            , fax
            , email
            , notifyemail
            , vat
            , ssntype
            , ssn
            , disclosename
            , discloseorganization
            , discloseaddress
            , disclosetelephone
            , disclosefax
            , discloseemail
            , disclosevat
            , discloseident
            , disclosenotifyemail
            , logd_request_id
            )
    , handle_(handle)
    , select_contact_by_handle_(Fred::InfoContactByHandle(handle_).set_lock())
    {}

    unsigned long long UpdateContactByHandle::exec(Fred::OperationContext& ctx)
    {
        unsigned long long history_id = 0;//to return

        try
        {
            ExceptionType update_exception;
            Fred::InfoContactOutput contact;
            try
            {
                contact = select_contact_by_handle_.exec(ctx);
            }
            catch(const Fred::InfoContactByHandle::Exception& info_exception)
            {
                if(info_exception.is_set_unknown_contact_handle())
                {
                    update_exception.set_unknown_contact_handle(
                        info_exception.get_unknown_contact_handle());
                }
                else throw;//rethrow unexpected
            }

            try
            {
                history_id = UpdateContact<UpdateContactByHandle>::exec(ctx,contact);
            }
            catch(const UpdateContact<UpdateContactByHandle>::Exception& update_contact_exception)
            {
                if(update_contact_exception.is_set_unknown_registrar_handle())
                {
                    //fatal good path, need valid registrar performing update
                    BOOST_THROW_EXCEPTION(ExceptionType().set_unknown_registrar_handle(
                            update_contact_exception.get_unknown_registrar_handle()));
                }
                else if(update_contact_exception.is_set_unknown_contact_handle()
                    ||  update_contact_exception.is_set_unknown_sponsoring_registrar_handle()
                    ||  update_contact_exception.is_set_unknown_ssntype()
                    ||  update_contact_exception.is_set_unknown_country()
                        )//non-fatal good path, update can continue to check input
                {
                    if(update_contact_exception.is_set_unknown_contact_handle())
                    {
                        update_exception.set_unknown_contact_handle(handle_);
                    }

                    if(update_contact_exception.is_set_unknown_sponsoring_registrar_handle())
                    {   //non-fatal good path, update can continue to check input
                        update_exception.set_unknown_sponsoring_registrar_handle(
                                update_contact_exception.get_unknown_sponsoring_registrar_handle());
                    }

                    if(update_contact_exception.is_set_unknown_ssntype())
                    {   //non-fatal good path, update can continue to check input
                        update_exception.set_unknown_ssntype(
                                update_contact_exception.get_unknown_ssntype());
                    }

                    if(update_contact_exception.is_set_unknown_country())
                    {   //non-fatal good path, update can continue to check input
                        update_exception.set_unknown_country(
                                update_contact_exception.get_unknown_country());
                    }
                }
                else throw;//rethrow unexpected
            }

            if(update_exception.throw_me())
            {
                BOOST_THROW_EXCEPTION(update_exception);
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }

        return history_id;
    }

    std::string UpdateContactByHandle::to_string() const
    {
        return Util::format_operation_state("UpdateContactByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("select_contact_by_handle", select_contact_by_handle_.to_string()))//member operation
        (std::make_pair("UpdateContact<UpdateContactByHandle>",UpdateContact<UpdateContactByHandle>::to_string()))//parent operation
        );
    }

}//namespace Fred

