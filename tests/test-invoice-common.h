
#include "cfg/config_handler_decl.h"
#include "cfg/handle_database_args.h"

#include "cfg/handle_registry_args.h"
#include "cfg/handle_rifd_args.h"
#include "cfg/handle_corbanameservice_args.h"
#include "mailer_manager.h"
#include "corba/EPP.hh"
#include "corba/epp/epp_impl.h"

void init_corba_container();

void EPP_backend_init(ccReg_EPP_i *epp_i, HandleRifdArgs *rifd_args_ptr);

std::auto_ptr<ccReg_EPP_i> create_epp_backend_object();

CORBA::Long epp_backend_login(ccReg_EPP_i *epp, std::string registrar_handle);
