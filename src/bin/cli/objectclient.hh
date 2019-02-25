/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef OBJECTCLIENT_HH_673CBCC4D75746C6919827FD6E1CDC06
#define OBJECTCLIENT_HH_673CBCC4D75746C6919827FD6E1CDC06

#include "src/bin/cli/baseclient.hh"
#include "src/bin/cli/commonclient.hh"
#include "src/bin/cli/object_params.hh"

#include "src/bin/corba/admin/admin_impl.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/libfred/registry.hh"

#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>

namespace Admin {

class ObjectClient : public BaseClient
{
public:
    ObjectClient();
    ObjectClient(
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
            const DeleteObjectsArgs& _delete_objects_params);
    ~ObjectClient()
    { }

    static const struct options* getOpts();
    static int getOptsCount();
    void runMethod();

    void show_opts();
    void new_state_request_name();
    void list();
    void update_states();
    void delete_candidates();
    void regular_procedure();
private:
    enum class DeleteObjectsResult
    {
        success,
        failure
    };
    DeleteObjectsResult deleteObjects(const std::string& _type_list);

    DBSharedPtr m_db;
    ccReg::EPP_var m_epp;

    std::string nameservice_context;
    optional_string docgen_path;
    optional_string docgen_template_path;
    optional_string fileclient_path;
    bool restricted_handles;
    unsigned long long docgen_domain_count_limit;
    bool object_new_state_request_name;
    ObjectNewStateRequestNameArgs object_new_state_request_name_params;

    bool object_update_states;
    ObjectUpdateStatesArgs object_update_states_params;
    bool object_regular_procedure;
    ObjectRegularProcedureArgs object_regular_procedure_params;
    bool object_delete_candidates;
    DeleteObjectsArgs delete_objects_params;

    static const struct options m_opts[];
};

} // namespace Admin

#endif
