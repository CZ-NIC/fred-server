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

#ifndef DOMAIN_NAME_VALIDATION_INIT_HH_4E51E41BE2744BCD8C3A3E8EBEACBF58
#define DOMAIN_NAME_VALIDATION_INIT_HH_4E51E41BE2744BCD8C3A3E8EBEACBF58
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"

#include "src/bin/cli/domain_name_validation_params.hh"
#include "src/util/log/context.hh"
#include "src/util/factory.hh"
#include "src/util/factory_check.hh"

#include "src/libfred/registrable_object/domain/domain_name.hh"


/**
 * \class init_domain_name_validation_impl
 * \brief implementation of domain name validation initialization
 */
struct init_domain_name_validation_impl
{
    void operator()() const
    {
        Logging::Context logctx("init_domain_name_validation_impl");

        LibFred::OperationContextCreator ctx;

        DomainNameValidationCheckersInitArgs cfg_params
            = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleInitDomainNameValidationCheckersArgsGrp>()->params;

        if(cfg_params.serch_checkers_by_prefix)
        {
            std::vector<std::string> checkers_with_prefix
                = get_names_begining_with_prefix_from_factory<LibFred::Domain::DomainNameCheckerFactory>(
                    cfg_params.checker_name_prefix.get_value());

            for(std::vector<std::string>::const_iterator i = checkers_with_prefix.begin()
                    ; i != checkers_with_prefix.end() ; ++i)
            {
                try
                {
                    ctx.get_conn().exec("SAVEPOINT init_domain_name_validation");
                    LibFred::Domain::insert_domain_name_checker_name_into_database(
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
                LibFred::Domain::insert_domain_name_checker_name_into_database(
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

/**
 * \class set_zone_domain_name_validation_impl
 * \brief set domain name validation config per zone
 */
struct set_zone_domain_name_validation_impl
{
    void operator()() const
    {
        Logging::Context logctx("set_zone_domain_name_validation_impl");

        LibFred::OperationContextCreator ctx;

        ZoneDomainNameValidationCheckersArgs cfg_params
            = CfgArgGroups::instance()->get_handler_ptr_by_type<HandleDomainNameValidationByZoneArgsGrp>()->params;

        LibFred::Domain::set_domain_name_validation_config_into_database(ctx,cfg_params.zone_name, cfg_params.checker_names);

        ctx.commit_transaction();
  }
};


#endif
