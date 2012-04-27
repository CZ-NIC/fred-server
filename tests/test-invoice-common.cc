#include "test-invoice-common.h"

#include "cfg/handle_corbanameservice_args.h"
#include "corba_wrapper_decl.h"


void init_corba_container() {

    //corba config
    FakedArgs fa = CfgArgs::instance()->fa;


    HandleCorbaNameServiceArgs* ns_args_ptr=CfgArgs::instance()->
            get_handler_ptr_by_type<HandleCorbaNameServiceArgs>();
    CorbaContainer::set_instance(fa.get_argc(), fa.get_argv()
            , ns_args_ptr->nameservice_host
            , ns_args_ptr->nameservice_port
            , ns_args_ptr->nameservice_context);

}

void handle_epp_exception(ccReg::EPP::EppError &ex)
{
    std::ostringstream msg;
    msg << " EPP Exception: " << ex.errCode << ": " << ex.errMsg << ", Reason: ";

    for (unsigned i=0;i<ex.errorList.length();i++) {
        msg << ex.errorList[i].reason;
    }
    throw CreateDomainFailed(msg.str());
}

void EPP_backend_init(ccReg_EPP_i *epp_i, HandleRifdArgs *rifd_args_ptr)
{
    // load all country code from table enum_country
    if (epp_i->LoadCountryCode() <= 0) {
      throw std::runtime_error("EPP backend init: database error: load country code");
    }

    // load error messages to memory
    if (epp_i->LoadErrorMessages() <= 0) {
      throw std::runtime_error("EPP backend init: database error: load error messages");
    }

    // load reason messages to memory
    if (epp_i->LoadReasonMessages() <= 0) {
      throw std::runtime_error("EPP backend init: database error: load reason messages" );
    }
}

std::auto_ptr<ccReg_EPP_i> create_epp_backend_object()
{
    init_corba_container();

    //conf pointers
    HandleDatabaseArgs* db_args_ptr = CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleDatabaseArgs>();
    HandleRegistryArgs* registry_args_ptr = CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleRegistryArgs>();
    HandleRifdArgs* rifd_args_ptr = CfgArgs::instance()
        ->get_handler_ptr_by_type<HandleRifdArgs>();

    // TODO this will be leaked
    MailerManager * mailMan(new MailerManager(CorbaContainer::get_instance()->getNS()));

    std::auto_ptr<ccReg_EPP_i> ret_epp ( new ccReg_EPP_i(
                db_args_ptr->get_conn_info()
                , mailMan, CorbaContainer::get_instance()->getNS()
                , registry_args_ptr->restricted_handles
                , registry_args_ptr->disable_epp_notifier
                , registry_args_ptr->lock_epp_commands
                , registry_args_ptr->nsset_level
                , registry_args_ptr->docgen_path
                , registry_args_ptr->docgen_template_path
                , registry_args_ptr->fileclient_path
                , registry_args_ptr->disable_epp_notifier_cltrid_prefix
                , rifd_args_ptr->rifd_session_max
                , rifd_args_ptr->rifd_session_timeout
                , rifd_args_ptr->rifd_session_registrar_max
                , rifd_args_ptr->rifd_epp_update_domain_keyset_clear
        ));

    EPP_backend_init( ret_epp.get(), rifd_args_ptr);

    return ret_epp;
}    

CORBA::Long epp_backend_login(ccReg_EPP_i *epp, std::string registrar_handle)
{
    std::string passwd_var("");
    std::string new_passwd_var("");
    std::string cltrid_var("omg");
    std::string xml_var("<omg/>");
    std::string cert_var("");

    CORBA::ULongLong clientId = 0;

    ccReg::Response *r = epp->ClientLogin(
        registrar_handle.c_str(),passwd_var.c_str(),new_passwd_var.c_str(),cltrid_var.c_str(),xml_var.c_str(),clientId, 0,
        cert_var.c_str(),ccReg::EN);
    
    if (r->code != 1000 || !clientId) {
        boost::format msg = boost::format("Error code: %1% - %2% ") % r->code % r->msg;
        std::cerr << msg.str() << std::endl;
        throw std::runtime_error(msg.str());
    }

    return clientId;
}
