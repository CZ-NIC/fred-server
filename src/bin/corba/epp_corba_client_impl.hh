#ifndef EPP_CORBA_CLIENT_IMPL_HH_E0FFD7FBE9284C4593DCD9E28A896CCF
#define EPP_CORBA_CLIENT_IMPL_HH_E0FFD7FBE9284C4593DCD9E28A896CCF

#include <boost/thread/thread.hpp>
#include <src/bin/corba/EPP.hh>
#include "src/util/corba_wrapper_decl.hh"
#include "src/deprecated/libfred/epp_corba_client.hh"

class EppCorbaClientImpl : public EppCorbaClient
{
public:
    EppCorbaClientImpl();

    void callDestroyAllRegistrarSessions(Database::ID reg_id) const;

private:
    ccReg::EPP_var epp_ref;

    mutable boost::mutex ref_mutex;
};

#endif
