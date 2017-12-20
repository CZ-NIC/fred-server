
#include "src/bin/corba/epp_corba_client_impl.hh"

EppCorbaClientImpl::EppCorbaClientImpl()
{
    boost::mutex::scoped_lock lock(ref_mutex);

    epp_ref = ccReg::EPP::_narrow(CorbaContainer::get_instance()->nsresolve("EPP"));
}

void EppCorbaClientImpl::callDestroyAllRegistrarSessions(Database::ID reg_id) const
{
    boost::mutex::scoped_lock lock(ref_mutex);

    epp_ref->destroyAllRegistrarSessions(reg_id);
}
