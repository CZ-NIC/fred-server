#ifndef MOJEID_IMPL_H_
#define MOJEID_IMPL_H_

#include <string>

#include "cfg/handle_registry_args.h"
#include "corba/MojeID.hh"

namespace Registry {
namespace MojeID {

class ServerImpl : public POA_Registry::MojeID::Server,
                   public PortableServer::RefCountServantBase
{
    private:
        const HandleRegistryArgs *server_conf_;
        const std::string server_name_;
        unsigned long long mojeid_registrar_id_;

        virtual ~ServerImpl();


    public:
        ServerImpl(const HandleRegistryArgs *_server_conf,
                   const std::string &_server_name);

        CORBA::ULongLong contactCreate(const Contact &_contact,
                                       IdentificationMethod _method,
                                       const CORBA::ULongLong _request_id);

        CORBA::ULongLong transferContact(const char* _handle,
                                         IdentificationMethod _method,
                                         const CORBA::ULongLong _request_id);

        CORBA::ULongLong processIdentification(const char* _ident_request_id,
                                               const char* _password,
                                               const CORBA::ULongLong _request_id);

        void contactUpdatePrepare(const Contact &_contact,
                                  const char* _trans_id,
                                  const CORBA::ULongLong _request_id);

        Contact* contactInfo(const CORBA::ULongLong _id);

        void commitPreparedTransaction(const char* _trans_id);

        void rollbackPreparedTransaction(const char* _trans_id);

        char* getIdentificationInfo(CORBA::ULongLong);

        ContactStateChangeList* getContactStateChanges(const Date& since);
};

}
}

#endif /*MOJEID_IMPL_H_*/

