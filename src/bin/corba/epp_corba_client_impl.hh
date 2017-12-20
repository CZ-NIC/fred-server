#ifndef _EPP_CORBA_CLIENT_IMPL_H_
#define _EPP_CORBA_CLIENT_IMPL_H_

#include <boost/thread/thread.hpp>
#include <src/bin/corba/EPP.hh>
#include "src/util/corba_wrapper_decl.hh"
#include "src/libfred/epp_corba_client.hh"

class EppCorbaClientImpl : public EppCorbaClient
{
public:
    EppCorbaClientImpl();

    void callDestroyAllRegistrarSessions(Database::ID reg_id) const;

private:
    ccReg::EPP_var epp_ref;

    mutable boost::mutex ref_mutex;
};

#endif // _EPP_CORBA_CLIENT_IMPL_H_
