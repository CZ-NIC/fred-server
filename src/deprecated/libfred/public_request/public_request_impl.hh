/*
 * Copyright (C) 2011-2022  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef PUBLIC_REQUEST_IMPL_HH_21AC7B02F9C5497DA07D708E2DDAA8B5
#define PUBLIC_REQUEST_IMPL_HH_21AC7B02F9C5497DA07D708E2DDAA8B5

#include "src/deprecated/libfred/public_request/public_request.hh"
#include "src/deprecated/libfred/common_impl.hh"


namespace LibFred {
namespace PublicRequest {

std::string Status2Str(Status_PR _status);

std::string ObjectType2Str(ObjectType type);

void insertNewStateRequest(
        Database::ID blockRequestID,
        Database::ID objectId,
        const std::string& state_name);

bool queryBlockRequest(
        Database::ID objectId,
        Database::ID blockRequestID,
        const std::vector<std::string>& states_vect,
        bool unblock);

unsigned long long check_public_request(
        unsigned long long _object_id,
        const Type& _type);

void cancel_public_request(
        unsigned long long _object_id,
        const Type& _type,
        unsigned long long _request_id);

bool object_was_changed_since_request_create(const unsigned long long _request_id);

template <typename T>
std::unique_ptr<PublicRequestProducer> make_public_request_producer()
{
    class Producer : public PublicRequestProducer
    {
    private:
        std::unique_ptr<PublicRequest> get() const override
        {
            return std::make_unique<T>();
        }
    };
    return std::make_unique<Producer>();
}

class PublicRequestImpl
    : public LibFred::CommonObjectImpl,
      virtual public PublicRequest
{
public:
    Database::ID& get_answer_email_id() { return answer_email_id_; }
    Manager* get_manager_ptr() { return man_; }
    PublicRequestImpl();

    explicit PublicRequestImpl(
            Database::ID _id,
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
            std::string _registrar_url);

    void setManager(Manager* _man);

    void init(Database::Row::Iterator& _it) override;
    void save() override;
    LibFred::PublicRequest::Type getType() const override;
    void setType(LibFred::PublicRequest::Type _type) override;
    LibFred::PublicRequest::Status_PR getStatus() const override;
    void setStatus(LibFred::PublicRequest::Status_PR _status) override;
    ptime getCreateTime() const override;
    ptime getResolveTime() const override;
    const std::string& getReason() const override;
    void setReason(const std::string& _reason) override;
    const std::string& getEmailToAnswer() const override;
    void setEmailToAnswer(const std::string& _email) override;
    const Database::ID getAnswerEmailId() const override;
    const Database::ID getRequestId() const override;
    const Database::ID getResolveRequestId() const override;
    void setRequestId(const Database::ID& _create_request_id) override;
    void setRegistrarId(const Database::ID& _registrar_id) override;
    void addObject(const OID& _oid) override;
    const OID& getObject(unsigned _idx) const override;
    unsigned getObjectSize() const override;
    const Database::ID getRegistrarId() const override;
    const std::string getRegistrarHandle() const override;
    const std::string getRegistrarName() const override;
    const std::string getRegistrarUrl() const override;
    std::string getEmails() const override;
    TID sendEmail() const override;
    void processAction(bool check) override;
    virtual void invalidateAction();
    void process(bool invalidated, bool check, unsigned long long _request_id) override;
    unsigned getPDFType() const override;
    virtual void postCreate();

    Manager* getPublicRequestManager() const;
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

    Manager* man_;
};

class PublicRequestAuthImpl
    : virtual public PublicRequestAuth,
      public PublicRequestImpl
{
public:
    PublicRequestAuthImpl();

    ~PublicRequestAuthImpl() override { }

    void init(Database::Row::Iterator& _it) override;
    virtual std::string getIdentification() const;
    virtual std::string getPassword() const;
    bool authenticate(const std::string &_password) override;
    void save() override;
    void process(bool _invalidated, bool _check, unsigned long long _request_id) override;

    /* just to be sure of empty impl (if someone would change base impl) */
    void postCreate() override;

    /* don't use this methods for constucting email so far */
    std::string getTemplateName() const;

    void fillTemplateParams(Mailer::Parameters& params) const;

    virtual std::string generatePasswords() = 0;

    bool check() const;
protected:
    bool authenticated_;
    std::string identification_;
    std::string password_;
};

COMPARE_CLASS_IMPL(PublicRequestImpl, CreateTime)
COMPARE_CLASS_IMPL(PublicRequestImpl, ResolveTime)
COMPARE_CLASS_IMPL(PublicRequestImpl, Type)
COMPARE_CLASS_IMPL(PublicRequestImpl, Status)

}//namespace LibFred::PublicRequest
}//namespace LibFred

#endif//PUBLIC_REQUEST_IMPL_HH_21AC7B02F9C5497DA07D708E2DDAA8B5

