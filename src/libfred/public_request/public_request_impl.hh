#ifndef PUBLIC_REQUEST_IMPL_HH_21AC7B02F9C5497DA07D708E2DDAA8B5
#define PUBLIC_REQUEST_IMPL_HH_21AC7B02F9C5497DA07D708E2DDAA8B5

#include "src/libfred/public_request/public_request.hh"
#include "src/libfred/common_impl.hh"

namespace LibFred {
namespace PublicRequest {



std::string Status2Str(Status_PR _status);

std::string ObjectType2Str(ObjectType type);

void insertNewStateRequest(
        Database::ID blockRequestID,
        Database::ID objectId,
        const std::string & state_name);

bool queryBlockRequest(
        Database::ID objectId,
        Database::ID blockRequestID,
        const std::vector<std::string>& states_vect,
        bool unblock);

unsigned long long check_public_request(
        const unsigned long long &_object_id,
        const Type &_type);

void cancel_public_request(
        const unsigned long long &_object_id,
        const Type &_type,
        const unsigned long long &_request_id);

bool object_was_changed_since_request_create(const unsigned long long _request_id);



class PublicRequestImpl
    : public LibFred::CommonObjectImpl,
      virtual public PublicRequest
{
protected:
    LibFred::PublicRequest::Type type_;
    Database::ID create_request_id_;
    Database::ID resolve_request_id_;
    Database::DateTime create_time_;
    LibFred::PublicRequest::Status_PR status_;
    Database::DateTime resolve_time_;
    std::string reason_;
    std::string email_to_answer_;
    Database::ID answer_email_id_;

    Database::ID registrar_id_;
    std::string registrar_handle_;
    std::string registrar_name_;
    std::string registrar_url_;

    std::vector<OID> objects_;

protected:
    Manager* man_;

public:
    Database::ID& get_answer_email_id(){return answer_email_id_;}
    Manager* get_manager_ptr(){return man_;}
    PublicRequestImpl();

    PublicRequestImpl(Database::ID _id,
              LibFred::PublicRequest::Type _type,
              Database::ID _create_request_id,
              Database::DateTime _create_time,
              LibFred::PublicRequest::Status_PR _status,
              Database::DateTime _resolve_time,
              std::string _reason,
              std::string _email_to_answer,
              Database::ID _answer_email_id,
              Database::ID _registrar_id,
              std::string _registrar_handle,
              std::string _registrar_name,
              std::string _registrar_url
              );

    void setManager(Manager* _man);

    virtual void init(Database::Row::Iterator& _it);

    virtual void save();

    virtual LibFred::PublicRequest::Type getType() const;

    virtual void setType(LibFred::PublicRequest::Type _type);

    virtual LibFred::PublicRequest::Status_PR getStatus() const;

    virtual void setStatus(LibFred::PublicRequest::Status_PR _status);

    virtual ptime getCreateTime() const;

    virtual ptime getResolveTime() const;

    virtual const std::string& getReason() const;

    virtual void setReason(const std::string& _reason);

    virtual const std::string& getEmailToAnswer() const;

    virtual void setEmailToAnswer(const std::string& _email);

    virtual const Database::ID getAnswerEmailId() const;

    virtual const Database::ID getRequestId() const;

    virtual const Database::ID getResolveRequestId() const;

    virtual void setRequestId(const Database::ID& _create_request_id);

    virtual void setRegistrarId(const Database::ID& _registrar_id);

    virtual void addObject(const OID& _oid);

    virtual const OID& getObject(unsigned _idx) const;

    virtual unsigned getObjectSize() const;

    virtual const Database::ID getRegistrarId() const;

    virtual const std::string getRegistrarHandle() const;

    virtual const std::string getRegistrarName() const;

    virtual const std::string getRegistrarUrl() const;

    virtual std::string getEmails() const;

    virtual TID sendEmail() const;

    virtual void processAction(bool check);

    virtual void invalidateAction();

    virtual void process(bool invalidated, bool check, const unsigned long long &_request_id);

    virtual unsigned getPDFType() const;

    virtual void postCreate();

    Manager* getPublicRequestManager() const;
};



class PublicRequestAuthImpl
    : virtual public PublicRequestAuth,
      public PublicRequestImpl
{
protected:
    bool authenticated_;
    std::string identification_;
    std::string password_;


public:
    PublicRequestAuthImpl();

    virtual ~PublicRequestAuthImpl() { }

    virtual void init(Database::Row::Iterator& _it);

    virtual std::string getIdentification() const;

    virtual std::string getPassword() const;

    virtual bool authenticate(const std::string &_password);

    virtual void save();

    virtual void process(bool _invalidated, bool _check, const unsigned long long &_request_id);

    /* just to be sure of empty impl (if someone would change base impl) */
    virtual void postCreate();

    /* don't use this methods for constucting email so far */
    std::string getTemplateName() const;

    void fillTemplateParams(Mailer::Parameters& params) const;

    virtual std::string generatePasswords() = 0;

    bool check() const;
};


COMPARE_CLASS_IMPL(PublicRequestImpl, CreateTime)
COMPARE_CLASS_IMPL(PublicRequestImpl, ResolveTime)
COMPARE_CLASS_IMPL(PublicRequestImpl, Type)
COMPARE_CLASS_IMPL(PublicRequestImpl, Status)

}
}

#endif /* PUBLIC_REQUEST_IMPL_H_ */

