
#include "src/util/cfg/config_handler_decl.hh"
#include "src/util/cfg/handle_database_args.hh"

#include "src/util/cfg/handle_registry_args.hh"
#include "src/util/cfg/handle_rifd_args.hh"
#include "src/util/cfg/handle_corbanameservice_args.hh"
#include "src/bin/corba/mailer_manager.hh"
#include "src/bin/corba/EPP.hh"
#include "src/bin/corba/epp/epp_impl.hh"


// thrown in case test has failed (something than CAN go wrong, not an error in test)
class CreateDomainFailed : public std::runtime_error {
public:
    CreateDomainFailed(const std::string &msg) : std::runtime_error(msg) 
    { }
};

void init_corba_container();

void handle_epp_exception(ccReg::EPP::EppError &ex);
void EPP_backend_init(ccReg_EPP_i *epp_i, HandleRifdArgs *rifd_args_ptr);
std::unique_ptr<ccReg_EPP_i> create_epp_backend_object();
CORBA::Long epp_backend_login(ccReg_EPP_i *epp, std::string registrar_handle);