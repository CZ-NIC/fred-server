#ifndef MOJEID_IMPL_H_
#define MOJEID_IMPL_H_

#include <string>
#include <boost/thread/mutex.hpp>

#include "cfg/handle_registry_args.h"
#include "cfg/handle_mojeid_args.h"
#include "corba/MojeID.hh"

namespace Registry {
namespace MojeID {

class ServerImpl : public POA_Registry::MojeID::Server,
                   public PortableServer::RefCountServantBase
{
    private:
        const HandleRegistryArgs *registry_conf_;
        const HandleMojeIDArgs *server_conf_;
        const std::string server_name_;
        unsigned long long mojeid_registrar_id_;

        std::map<std::string, unsigned long long> transaction_contact;
        boost::mutex tc_mutex;

        virtual ~ServerImpl();


    public:
        ServerImpl(const std::string &_server_name);

        CORBA::ULongLong contactCreate(const Contact &_contact,
                                       IdentificationMethod _method,
                                       const CORBA::ULongLong _request_id);

        CORBA::ULongLong contactTransfer(const char* _handle,
                                         IdentificationMethod _method,
                                         const CORBA::ULongLong _request_id);

        void contactUnidentifyPrepare(const CORBA::ULongLong _contact_id,
                                const char * _trans_id,
                               const CORBA::ULongLong _request_id);

        void contactUpdatePrepare(const Contact &_contact,
                                  const char* _trans_id,
                                  const CORBA::ULongLong _request_id);

        Contact* contactInfo(const CORBA::ULongLong _id);

        CORBA::ULongLong processIdentification(const char* _ident_request_id,
                                               const char* _password,
                                               const CORBA::ULongLong _request_id);

        void commitPreparedTransaction(const char* _trans_id);

        void rollbackPreparedTransaction(const char* _trans_id);

        char* getIdentificationInfo(CORBA::ULongLong _contact_id);

        Buffer* getValidationPdf(const CORBA::ULongLong _contact_id);
        
        void createValidationRequest(const CORBA::ULongLong _contact_id,
                                     const CORBA::ULongLong _request_id);

        ContactStateInfoList* getContactsStates(const CORBA::ULong _last_hours);

        ContactStateInfo getContactState(const CORBA::ULongLong _contact_id);

        CORBA::ULongLong getContactId(const char* _handle);
};

}
}

#endif /*MOJEID_IMPL_H_*/

