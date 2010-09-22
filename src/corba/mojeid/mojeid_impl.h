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
        unsigned long long mojeid_registrar_id_;

        virtual ~MojeIDImpl();


    public:
        MojeIDImpl(const std::string &_server_name);

        CORBA::ULongLong contactCreate(const Contact &_contact,
                                       IdentificationMethod _method,
                                       const CORBA::ULongLong _request_id);

        void processIdentification(const char* _ident_request_id,
                                   const char* _password,
                                   const CORBA::ULongLong _request_id);

        void contactUpdatePrepare(const Contact &_contact,
                                  const char* _trans_id,
                                  const CORBA::ULongLong _request_id);

        Registry::Contact* contactInfo(const CORBA::ULongLong _id);

        void commitPreparedTransaction(const char* _trans_id);

        void rollbackPreparedTransaction(const char* _trans_id);

};

}

#endif /*MOJEID_IMPL_H_*/

