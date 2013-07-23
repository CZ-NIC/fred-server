/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @domain_name_validation_init.h
 *  initialization of domain name validation
 */

#ifndef DOMAIN_NAME_VALIDATION_IMPL_H_
#define DOMAIN_NAME_VALIDATION_IMPL_H_
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "cli_admin/handle_adminclientselection_args.h"

#include "cli_admin/domain_name_validation_params.h"
#include "log/context.h"
#include "util/factory.h"
#include "util/factory_check.h"

#include "fredlib/domain/domain_name.h"


/**
 * \class file_list_impl
 * \brief admin client implementation of file_list
 */
struct init_domain_name_validation_impl
{
    void operator()() const
    {
        Logging::Context logctx("init_domain_name_validation_impl");

        Fred::OperationContext ctx;

        DomainNameValidationCheckersInitArgs cfg_params
            = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleInitDomainNameValidationCheckersArgsGrp>()->params;

        if(cfg_params.serch_checkers_by_prefix)
        {
            std::vector<std::string> checkers_with_prefix
                = get_names_begining_with_prefix_from_factory<Fred::Domain::DomainNameCheckerFactory>(
                    cfg_params.checker_name_prefix.get_value());

            for(std::vector<std::string>::const_iterator i = checkers_with_prefix.begin()
                    ; i != checkers_with_prefix.end() ; ++i)
            {
                try
                {
                    ctx.get_conn().exec("SAVEPOINT init_domain_name_validation");
                    Fred::Domain::insert_domain_name_checker_name_into_database(
                        ctx, *i, std::string("autogenerated by init_domain_name_validation search with prefix: ")
                            + cfg_params.checker_name_prefix.get_value());
                }
                catch(const std::exception& ex)
                {
                    std::string what_string(ex.what());
                    if(what_string.find("enum_domain_name_validation_checker_name_key") != std::string::npos)
                    {
                        std::cout << "prefix searched domain name checker: " << (*i) << " is already in enum_domain_name_validation_checker table" << std::endl;
                        ctx.get_conn().exec("ROLLBACK TO SAVEPOINT init_domain_name_validation");
                    }
                    else
                        throw;
                }
            }
        }

        for(std::vector<std::string>::const_iterator i = cfg_params.checker_names.begin()
                ; i != cfg_params.checker_names.end() ; ++i)
        {
            try
            {
                ctx.get_conn().exec("SAVEPOINT init_domain_name_validation");
                Fred::Domain::insert_domain_name_checker_name_into_database(
                    ctx, *i, std::string("explicit specification of checker name"));
            }
            catch(const std::exception& ex)
            {
                std::string what_string(ex.what());
                if(what_string.find("enum_domain_name_validation_checker_name_key") != std::string::npos)
                {
                    std::cout << "specified domain name checker: " << (*i) << " is already in enum_domain_name_validation_checker table" << std::endl;
                    ctx.get_conn().exec("ROLLBACK TO SAVEPOINT init_domain_name_validation");
                }
                else
                    throw;
            }
        }
        ctx.commit_transaction();
  }
};

#endif // DOMAIN_NAME_VALIDATION_INIT_H_
