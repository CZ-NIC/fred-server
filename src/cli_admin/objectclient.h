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

#ifndef _OBJECTCLIENT_H_
#define _OBJECTCLIENT_H_

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

#include "corba/admin/admin_impl.h"
#include "old_utils/dbsql.h"
#include "fredlib/registry.h"
#include "baseclient.h"
#include "commonclient.h"

#include "object_params.h"

namespace Admin {

class ObjectClient : public BaseClient {
private:
    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;

    std::string nameservice_context;
    optional_string docgen_path;
    optional_string docgen_template_path;
    optional_string fileclient_path;
    bool restricted_handles;
    unsigned long long docgen_domain_count_limit;
    bool object_new_state_request;
    ObjectNewStateRequestArgs object_new_state_request_params;
    bool object_new_state_request_name;
    ObjectNewStateRequestNameArgs object_new_state_request_name_params;

    bool object_update_states;
    ObjectUpdateStatesArgs object_update_states_params;
    bool object_regular_procedure;
    ObjectRegularProcedureArgs object_regular_procedure_params;
    bool object_delete_candidates;
    DeleteObjectsArgs delete_objects_params;

    std::string disable_epp_notifier_cltrid_prefix;


    int createObjectStateRequest(Fred::TID object, unsigned state);
    int deleteObjects(const std::string &typeList, CorbaClient &cc);

    static const struct options m_opts[];
public:
    ObjectClient()
    : restricted_handles(false)
    , docgen_domain_count_limit(0)
    , object_new_state_request(false)
    , object_new_state_request_name(false)
    , object_update_states(false)
    , object_regular_procedure(false)
    , object_delete_candidates(false)
    { }
    ObjectClient(
            const std::string &connstring
            , const std::string &nsAddr
            , const std::string& _nameservice_context
            , const optional_string& _docgen_path
            , const optional_string& _docgen_template_path
            , const optional_string& _fileclient_path
            , bool _restricted_handles
            , const unsigned long long _docgen_domain_count_limit
            , const bool _object_new_state_request
            , const ObjectNewStateRequestArgs& _object_new_state_request_params
            , const bool _object_new_state_request_name
            , const ObjectNewStateRequestNameArgs& _object_new_state_request_name_params
            , const bool _object_update_states
            , const ObjectUpdateStatesArgs& _object_update_states_params
            , const bool _object_regular_procedure
            , const ObjectRegularProcedureArgs& _object_regular_procedure_params
            , const bool _object_delete_candidates
            , const DeleteObjectsArgs& _delete_objects_params
            , const std::string disable_epp_notifier_cltrid_prefix
            )
    : BaseClient(connstring, nsAddr)
    , nameservice_context(_nameservice_context)
    , docgen_path(_docgen_path)
    , docgen_template_path(_docgen_template_path)
    , fileclient_path(_fileclient_path)
    , restricted_handles(_restricted_handles)
    , docgen_domain_count_limit(_docgen_domain_count_limit)
    , object_new_state_request(_object_new_state_request)
    , object_new_state_request_params(_object_new_state_request_params)
    , object_new_state_request_name(_object_new_state_request_name)
    , object_new_state_request_name_params(_object_new_state_request_name_params)
    , object_update_states(_object_update_states)
    , object_update_states_params(_object_update_states_params)
    , object_regular_procedure(_object_regular_procedure)
    , object_regular_procedure_params(_object_regular_procedure_params)
    , object_delete_candidates(_object_delete_candidates)
    , delete_objects_params(_delete_objects_params)
    , disable_epp_notifier_cltrid_prefix(disable_epp_notifier_cltrid_prefix)
    {
        m_db = connect_DB(connstring
                , std::runtime_error("ObjectClient db connection failed"));
    }
    ~ObjectClient()
    { }

    static const struct options *getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void new_state_request();
    void new_state_request_name();
    void list();
    void update_states();
    void delete_candidates();
    void regular_procedure();
}; // class Object

} // namespace Admin;

#endif // _OBJECTCLIENT_H_

