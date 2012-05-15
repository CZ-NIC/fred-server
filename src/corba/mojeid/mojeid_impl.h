#ifndef MOJEID_IMPL_H_
#define MOJEID_IMPL_H_

#include <string>
#include <boost/thread/mutex.hpp>

#include "cfg/handle_registry_args.h"
#include "cfg/handle_mojeid_args.h"
#include "corba/mailer_manager.h"
#include "corba/MojeID.hh"
#include "mojeid/request.h"
#include "mojeid_identification.h"


namespace Registry {
namespace MojeID {

enum mojeid_operation_type {
    MOJEID_NOP = 0,
    MOJEID_CONTACT_CREATE = 1,
    MOJEID_CONTACT_UPDATE,
    MOJEID_CONTACT_UNIDENTIFY,
    MOJEID_CONTACT_TRANSFER
};

struct trans_data {

    explicit trans_data(const mojeid_operation_type &operation) : op(operation), cid(0), prid(0), request_id(0)
    { }

    mojeid_operation_type op;
    unsigned long long cid;
    unsigned long long prid;
    unsigned long long request_id;
};

class ServerImpl : public POA_Registry::MojeID::Server,
                   public PortableServer::RefCountServantBase
{
    private:
        const HandleRegistryArgs *registry_conf_;
        const HandleMojeIDArgs *server_conf_;
        const std::string server_name_;
        unsigned long long mojeid_registrar_id_;

        typedef std::map<std::string, trans_data> transaction_data_map_type;
        transaction_data_map_type transaction_data;
        boost::mutex td_mutex; /// for transaction data

        virtual ~ServerImpl();
        boost::shared_ptr<MailerManager> mailer_;

        IdentificationRequestPtr contactCreateWorker(unsigned long long &cid, unsigned long long &hid,
            const Contact &_contact, IdentificationMethod _method, ::MojeID::Request &_request);

    public:
        ServerImpl(const std::string &_server_name);

        CORBA::ULongLong contactCreate(const Contact &_contact,
                                                   IdentificationMethod _method,
                                                   const CORBA::ULongLong _request_id);

        CORBA::ULongLong contactCreatePrepare(const Contact &_contact,
                                                   IdentificationMethod _method,
                                                   const char * _trans_id,
                                                   const CORBA::ULongLong _request_id,
                                                   CORBA::String_out _identification);

        CORBA::ULongLong contactTransfer(const char* _handle,
                                         IdentificationMethod _method,
                                         const CORBA::ULongLong _request_id);

        CORBA::ULongLong contactTransferPrepare(const char *_handle,
                                             IdentificationMethod _method,
                                             const char * _trans_id,
                                             const CORBA::ULongLong _request_id,
                                             CORBA::String_out _identification);

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

        ContactHandleList* getUnregistrableHandles();

        char *contactAuthInfo(const CORBA::ULongLong _contact_id);
};

void sendAuthPasswords(unsigned long long cid, unsigned long long prid);
void updateObjectStates(unsigned long long cid) throw();
void finishEppAction(unsigned long long eppaction_id) throw();

}
}

#endif /*MOJEID_IMPL_H_*/

