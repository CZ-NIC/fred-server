#ifndef _EPP_CORBA_CLIENT_IMPL_H_
#define _EPP_CORBA_CLIENT_IMPL_H_

#include <boost/thread/thread.hpp>
#include <src/corba/EPP.hh>
#include "corba_wrapper_decl.h"
#include "fredlib/epp_corba_client.h"

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
