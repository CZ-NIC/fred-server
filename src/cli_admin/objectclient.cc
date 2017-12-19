/*
 *  Copyright (C) 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/cli_admin/objectclient.h"

#include "src/cli_admin/commonclient.h"
#include "src/fredlib/registry.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/object_states.h"
#include "src/fredlib/poll/create_state_messages.h"
#include "src/fredlib/poll/create_low_credit_messages.h"
#include "src/fredlib/poll/message_type_set.h"
#include "src/cli_admin/remove_delete_candidates.hh"

#include "util/log/logger.h"

#include <stdexcept>

#include <omniORB4/CORBA.h>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/optional.hpp>
#include <boost/lexical_cast.hpp>

namespace Admin {

ObjectClient::ObjectClient()
    : restricted_handles(false),
      docgen_domain_count_limit(0),
      object_new_state_request_name(false),
      object_update_states(false),
      object_regular_procedure(false),
      object_delete_candidates(false)
{ }

ObjectClient::ObjectClient(
        const std::string& _connstring,
        const std::string& _nsAddr,
        const std::string& _nameservice_context,
        const optional_string& _docgen_path,
        const optional_string& _docgen_template_path,
        const optional_string& _fileclient_path,
        bool _restricted_handles,
        unsigned long long _docgen_domain_count_limit,
        bool _object_new_state_request_name,
        const ObjectNewStateRequestNameArgs& _object_new_state_request_name_params,
        bool _object_update_states,
        const ObjectUpdateStatesArgs& _object_update_states_params,
        bool _object_regular_procedure,
        const ObjectRegularProcedureArgs& _object_regular_procedure_params,
        bool _object_delete_candidates,
        const DeleteObjectsArgs& _delete_objects_params)
    : BaseClient(_connstring, _nsAddr),
      nameservice_context(_nameservice_context),
      docgen_path(_docgen_path),
      docgen_template_path(_docgen_template_path),
      fileclient_path(_fileclient_path),
      restricted_handles(_restricted_handles),
      docgen_domain_count_limit(_docgen_domain_count_limit),
      object_new_state_request_name(_object_new_state_request_name),
      object_new_state_request_name_params(_object_new_state_request_name_params),
      object_update_states(_object_update_states),
      object_update_states_params(_object_update_states_params),
      object_regular_procedure(_object_regular_procedure),
      object_regular_procedure_params(_object_regular_procedure_params),
      object_delete_candidates(_object_delete_candidates),
      delete_objects_params(_delete_objects_params)
{
    Database::Connection conn = Database::Manager::acquire();
    m_db.reset(new DB(conn));
}

void
ObjectClient::runMethod()
{
    if (object_new_state_request_name)
    {
        this->new_state_request_name();
    }
    else if (object_update_states)//m_conf.hasOpt(OBJECT_UPDATE_STATES_NAME)
    {
        this->update_states();
    }
    else if (object_regular_procedure)//m_conf.hasOpt(OBJECT_REGULAR_PROCEDURE_NAME)
    {
        this->regular_procedure();
    }
    else if (object_delete_candidates)//m_conf.hasOpt(OBJECT_DELETE_CANDIDATE)
    {
        this->delete_candidates();
    }
}

void
ObjectClient::new_state_request_name()
{
    Fred::createObjectStateRequestName(
            object_new_state_request_name_params.object_name,
            object_new_state_request_name_params.object_type,
            object_new_state_request_name_params.object_state_name,
            object_new_state_request_name_params.valid_from,
            object_new_state_request_name_params.valid_to,
            object_new_state_request_name_params.update_object_state);
}

void
ObjectClient::list()
{
    std::cout << "not implemented" << std::endl;
}

void
ObjectClient::update_states()
{
    std::unique_ptr<Fred::Manager> regMan(
            Fred::Manager::create(
                    m_db,
                    restricted_handles));//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
    const unsigned long long id = object_update_states_params.object_id.is_value_set()
            ? object_update_states_params.object_id.get_value()
            : 0;
    Logging::Manager::instance_ref().get(PACKAGE).debug(
            "regMan->updateObjectStates id: " + boost::lexical_cast<std::string>(id));
    regMan->updateObjectStates(id);
}

/// delete objects with status deleteCandidate
ObjectClient::DeleteObjectsResult
ObjectClient::deleteObjects(const std::string& _of_given_types)
{
    LOGGER("tracer").trace("ObjectClient::deleteObjects");
    const auto object_types = construct_set_of_object_types_from_string(_of_given_types);
    const boost::optional<unsigned> max_number_of_selected_candidates =
            delete_objects_params.object_delete_limit.is_value_set() &&
            (0 < delete_objects_params.object_delete_limit.get_value())
        ? boost::optional<unsigned>(delete_objects_params.object_delete_limit.get_value())
        : boost::none;
    const int fraction = delete_objects_params.object_delete_parts.is_value_set()
        ? delete_objects_params.object_delete_parts.get_value()
        : 1;
    const Seconds spread_deletion_in_time(
            delete_objects_params.object_delete_spread_during_time.is_value_set()
            ? static_cast<double>(delete_objects_params.object_delete_spread_during_time.get_value())
            : 0.0);
    if (delete_objects_params.object_delete_debug)
    {
        delete_objects_marked_as_delete_candidate<Debug::on>(
                fraction,
                max_number_of_selected_candidates,
                object_types,
                spread_deletion_in_time);
        return DeleteObjectsResult::success;
    }

    try
    {
        delete_objects_marked_as_delete_candidate<Debug::off>(
                fraction,
                max_number_of_selected_candidates,
                object_types,
                spread_deletion_in_time);
        return DeleteObjectsResult::success;
    }
    catch (const std::exception& e)
    {
        LOG(ERROR_LOG, "deleteObjects(): Exception caught: %s", e.what());
        return DeleteObjectsResult::failure;
    }
    catch (...)
    {
        LOG(ERROR_LOG, "deleteObjects(): Unknown exception caught");
        return DeleteObjectsResult::failure;
    }
}

void
ObjectClient::regular_procedure()
{
    try
    {
        std::unique_ptr<Fred::Document::Manager> docMan(
                Fred::Document::Manager::create(
                    docgen_path.get_value(),//m_conf.get<std::string>(REG_DOCGEN_PATH_NAME)
                    docgen_template_path.get_value(),//m_conf.get<std::string>(REG_DOCGEN_TEMPLATE_PATH_NAME)
                    fileclient_path.get_value(),//m_conf.get<std::string>(REG_FILECLIENT_PATH_NAME)
                    m_nsAddr));
        std::unique_ptr<CorbaClient> cc;
        static const int resolve_tries = 3;
        for (int idx = 0; idx < resolve_tries; ++idx)
        {
            try
            {
                cc.reset(new CorbaClient(0, NULL, m_nsAddr, nameservice_context));//m_conf.get<std::string>(NS_CONTEXT_NAME)
                if (cc.get() != NULL)
                {
                    break;
                }
            }
            catch (NameService::NOT_RUNNING)
            {
                LOG(ERROR_LOG, "regular_procedure(): resolve attempt %d of %d catching NOT_RUNNING", idx + 1, resolve_tries);
            }
            catch (NameService::BAD_CONTEXT)
            {
                LOG(ERROR_LOG, "regular_procedure(): resolve attempt %d of %d catching BAD_CONTEXT", idx + 1, resolve_tries);
            }
        }
        MailerManager mailMan(cc->getNS());

        std::unique_ptr<Fred::Zone::Manager> zoneMan(Fred::Zone::Manager::create());

        Fred::Messages::ManagerPtr msgMan = Fred::Messages::create_manager();


        std::unique_ptr<Fred::Domain::Manager> domMan(Fred::Domain::Manager::create(m_db, zoneMan.get()));

        std::unique_ptr<Fred::Contact::Manager> conMan(
                Fred::Contact::Manager::create(m_db, restricted_handles));//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
        std::unique_ptr<Fred::Nsset::Manager> nssMan(
                Fred::Nsset::Manager::create(m_db, zoneMan.get(), restricted_handles));//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
        std::unique_ptr<Fred::Keyset::Manager> keyMan(
                Fred::Keyset::Manager::create(m_db, restricted_handles));//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
        std::unique_ptr<Fred::Manager> registryMan(
                Fred::Manager::create(m_db, restricted_handles));//m_conf.get<bool>(REG_RESTRICTED_HANDLES_NAME)
        std::unique_ptr<Fred::Registrar::Manager> regMan(Fred::Registrar::Manager::create(m_db));
        std::unique_ptr<Fred::Notify::Manager> notifyMan(
                Fred::Notify::Manager::create(
                        m_db,
                        &mailMan,
                        conMan.get(),
                        nssMan.get(),
                        keyMan.get(),
                        domMan.get(),
                        docMan.get(),
                        regMan.get(),
                        msgMan));

        registryMan->updateObjectStates();
        registryMan->updateObjectStates();
        std::set<Fred::Poll::MessageType::Enum> poll_except;
        if (object_regular_procedure_params.poll_except_types.is_value_set()) //m_conf.hasOpt(OBJECT_POLL_EXCEPT_TYPES_NAME)
        {
            poll_except = Conversion::Enums::Sets::from_config_string<Fred::Poll::MessageType>(
                    object_regular_procedure_params.poll_except_types.get_value());
        }
        {
            Fred::OperationContextCreator ctx;
            Fred::Poll::CreateStateMessages(poll_except, boost::optional<int>()).exec(ctx);
            ctx.commit_transaction();
        }

        const std::string deleteTypes = delete_objects_params.object_delete_types.is_value_set()
            ? delete_objects_params.object_delete_types.get_value()
            : std::string();
        if (this->deleteObjects(deleteTypes) != DeleteObjectsResult::success)
        {
            LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): Error has occurred in deleteObject");
            return;
        }
        const std::string notifyExcept = object_regular_procedure_params.notify_except_types.is_value_set()
            ? object_regular_procedure_params.notify_except_types.get_value()
            : std::string();
        notifyMan->notifyStateChanges(notifyExcept, 0, NULL);

        {
            Fred::OperationContextCreator ctx;
            Fred::Poll::CreateLowCreditMessages().exec(ctx);
            ctx.commit_transaction();
        }
        notifyMan->generateLetters(docgen_domain_count_limit); //m_conf.get<unsigned>(REG_DOCGEN_DOMAIN_COUNT_LIMIT)
    }
    catch (const ccReg::Admin::SQL_ERROR&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): SQL_ERROR caught");
    }
    catch (const NameService::NOT_RUNNING&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): NOT_RUNNING caught");
    }
    catch (const NameService::BAD_CONTEXT&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): BAD_CONTEXT caught");
    }
    catch (const CORBA::Exception&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): CORBA exception caught");
    }
    catch (...)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): unknown exception caught");
    }
}

void
ObjectClient::delete_candidates()
{
    try
    {
        const std::string deleteTypes = delete_objects_params.object_delete_types.is_value_set()
            ? delete_objects_params.object_delete_types.get_value()
            : std::string();
        if (this->deleteObjects(deleteTypes) != DeleteObjectsResult::success)
        {
            LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): Error has occurred in deleteObject");
            return;
        }

    }
    catch (const ccReg::Admin::SQL_ERROR&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): SQL_ERROR caught");
    }
    catch (const NameService::NOT_RUNNING&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): NOT_RUNNING caught");
    }
    catch (const NameService::BAD_CONTEXT&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): BAD_CONTEXT caught");
    }
    catch (const CORBA::Exception&)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): CORBA exception caught");
    }
    catch (...)
    {
        LOG(ERROR_LOG, "Admin::ObjectClient::regular_procedure(): unknown exception caught");
    }
}

}//namespace Admin
