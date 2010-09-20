#ifndef MOJEID_IMPL_H_
#define MOJEID_IMPL_H_

#include <string>
#include "corba/MojeID.hh"

namespace Registry {

class MojeIDImpl : public POA_Registry::MojeID,
                   public PortableServer::RefCountServantBase
{
    private:
        const std::string server_name_;

        virtual ~MojeIDImpl();


    public:
        MojeIDImpl(const std::string &_server_name);

        CORBA::ULongLong contactCreate(const Contact &_contact);

        CORBA::ULongLong contactUpdatePrepare(const Contact &_contact,
                                              const char* _trans_id);

        Registry::Contact* contactInfo(const char* _handle);

        void commitPreparedTransaction(const char* _trans_id);

        void rollbackPreparedTransaction(const char* _trans_id);

};

}

#endif /*MOJEID_IMPL_H_*/
