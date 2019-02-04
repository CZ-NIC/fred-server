#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/cli/handle_adminclientselection_args.hh"
#include "util/log/context.hh"
#include "src/bin/corba/epp_corba_client_impl.hh"
#include "src/bin/cli/regblock_client_impl.hh"

struct block_registrar_id_impl
{
    void operator ()() const
    {
        Logging::Context ctx("block_registrar_id");

        Admin::RegBlockClient cli(
                CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientBlockRegistrarIdArgsGrp>()->params
        );
        cli.runMethod();
        return ;
    }
};

struct unblock_registrar_id_impl
{
    void operator ()() const
    {
        Logging::Context ctx("unblock_registrar_id");

        Admin::RegBlockClient cli(
                CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientUnblockRegistrarIdArgsGrp>()->params
        );
        cli.runMethod();
        return ;
    }
};

struct list_blocked_regs_impl
{
    void operator ()() const
    {
        Logging::Context ctx("list_blocked_regs");

        Admin::RegBlockClient cli(
                CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientListBlockedRegsArgsGrp>()->params
        );
        cli.runMethod();
        return ;
    }
};

struct block_registrars_over_limit_impl
{
    void operator ()() const
    {
        Logging::Context ctx("block_registrars_over_limit");

        Admin::RegBlockClient cli(
                CfgArgGroups::instance()->get_handler_ptr_by_type<HandleAdminClientBlockRegistrarsOverLimitArgsGrp>()->params
        );
        cli.runMethod();
        return ;
    }
};



