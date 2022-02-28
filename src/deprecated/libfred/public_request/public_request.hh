/*
 * Copyright (C) 2008-2022  CZ.NIC, z. s. p. o.
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

#ifndef PUBLIC_REQUEST_HH_2EEC5CA83E364803BF617BF5F3166AEC
#define PUBLIC_REQUEST_HH_2EEC5CA83E364803BF617BF5F3166AEC

#include "src/deprecated/libfred/common_object.hh"
#include "src/deprecated/libfred/registrar.hh"
#include "src/deprecated/libfred/object.hh"
#include "src/deprecated/libfred/registrable_object/domain.hh"
#include "src/deprecated/libfred/registrable_object/contact.hh"
#include "src/deprecated/libfred/registrable_object/nsset.hh"
#include "src/deprecated/libfred/registrable_object/keyset.hh"
#include "src/deprecated/libfred/documents.hh"
#include "src/deprecated/libfred/messages/messages_impl.hh"
#include "src/deprecated/model/model_filters.hh"

#include "libfred/db_settings.hh"
#include "libfred/mailer.hh"

#include "util/factory.hh"

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/format.hpp>
#include <boost/utility.hpp>

#include <exception>
#include <string>
#include <vector>


namespace LibFred {
namespace PublicRequest {

struct NotApplicable : std::runtime_error
{
    explicit NotApplicable(const std::string &_str) : std::runtime_error{"not_applicable: " + _str} { }
};

struct AlreadyProcessed : std::runtime_error
{
    explicit AlreadyProcessed(unsigned long long _rid, bool _success)
        : std::runtime_error{
                str(boost::format("public_request [id=%1%]: already_processed") % _rid)},
          success{_success}
    { }
    bool success;
};

struct ObjectChanged : std::runtime_error
{
    ObjectChanged() : std::runtime_error{"object_changed (since request create)"} { }
};

/// Member identification (i.e. for sorting)
enum MemberType {
  MT_CRDATE, ///< create date
  MT_RDATE,  ///< resolve date
  MT_TYPE,   ///< type of request
  MT_STATUS  ///< request status
};

using Type = std::string;

/// Request status
enum Status_PR {  // suffix to fix name clash with public_request_status.h status struct
  PRS_OPENED,     ///< Request was created and waiting for autorization
  PRS_RESOLVED,   ///< Email with answer was sent
  PRS_INVALIDATED ///< Time passed without authorization
};
std::string Status2Str(Status_PR _status); 

/// Object types
enum ObjectType {
  OT_UNKNOWN = 0,
  OT_CONTACT = 1,
  OT_NSSET   = 2,
  OT_DOMAIN  = 3,
  OT_KEYSET  = 4
};
std::string ObjectType2Str(ObjectType type);

/*
 * Object info
 */
struct OID
{
    explicit OID(Database::ID _id) : id(_id), handle(), type() { }
    explicit OID(Database::ID _id, std::string _handle, ObjectType _type)
        : id(_id),
          handle(_handle),
          type(_type)
    { }
    Database::ID id;
    std::string handle;
    ObjectType type;
};

struct REQUEST_BLOCKED : std::exception {};

struct RequestExists : std::runtime_error
{
    explicit RequestExists(const Type &_type, unsigned long long _object_id)
        : std::runtime_error{str(boost::format(
                "public_request [type=%1% object_id=%2%]: already exists")
                % _type % _object_id)}
    { }
};

/*
 * Request interface
 */
class PublicRequest
    : virtual public LibFred::CommonObject,
      private boost::noncopyable
{
public:
  virtual ~PublicRequest() { }

  virtual void init(Database::Row::Iterator& _it) = 0;
  virtual void save() = 0;

  virtual LibFred::PublicRequest::Type getType() const = 0;
  virtual void setType(LibFred::PublicRequest::Type _type) = 0;
  virtual LibFred::PublicRequest::Status_PR getStatus() const = 0;
  virtual void setStatus(LibFred::PublicRequest::Status_PR _status) = 0;
  virtual ptime getCreateTime() const = 0;
  virtual ptime getResolveTime() const = 0;
  virtual const std::string& getReason() const = 0;
  virtual void setReason(const std::string& _reason) = 0;
  virtual const std::string& getEmailToAnswer() const = 0;
  virtual void setEmailToAnswer(const std::string& _email) = 0;
  virtual const Database::ID getAnswerEmailId() const = 0;
  virtual void setRegistrarId(const Database::ID& _registrar_id) = 0;
  virtual void setRequestId(const Database::ID& _request_id) = 0;
  virtual void addObject(const OID& _oid) = 0;
  virtual const OID& getObject(unsigned _idx) const = 0;
  virtual unsigned getObjectSize() const = 0;
  virtual const Database::ID getRegistrarId() const = 0;
  virtual const std::string getRegistrarHandle() const = 0;
  virtual const std::string getRegistrarName() const = 0;
  virtual const std::string getRegistrarUrl() const = 0;
  virtual const Database::ID getRequestId() const = 0;
  virtual const Database::ID getResolveRequestId() const = 0;
  /// check if request can be handles
  virtual bool check() const = 0;
  /// return name of mailtemplate for answer email
  virtual std::string getTemplateName() const = 0;
  /// fill all mailtemplate parameters
  virtual void fillTemplateParams(Mailer::Parameters& params) const = 0;
  /// return list of destination email addresses for answer email
  virtual std::string getEmails() const = 0;
  /// send email with answer
  virtual TID sendEmail() const = 0;
  /// process request (or just close in case of invalid flag)
  virtual void process(bool invalid, bool check,
                       unsigned long long _request_id = 0) = 0;
  /// concrete action taken during request processing
  virtual void processAction(bool check) = 0;
  /// return proper type for PDF template generation
  virtual unsigned getPDFType() const = 0;
};

using PublicRequestPtr = std::shared_ptr<PublicRequest>;

class PublicRequestAuth : virtual public PublicRequest
{
public:
    struct NotAuthenticated : std::runtime_error
    {
        NotAuthenticated() : std::runtime_error{"not authenticated"} { }
    };

    virtual ~PublicRequestAuth() { }

    /* try to authenticate public request by comparing _password with the one
     * stored in database. it should save it to internal state and processAction(...) */
    virtual bool authenticate(const std::string& _password) = 0;
    virtual void sendPasswords() = 0;
};

using PublicRequestAuthPtr = std::shared_ptr<PublicRequestAuth>;

class List : virtual public LibFred::CommonList
{
public:
    virtual ~List() { }
    virtual const char* getTempTableName() const = 0;
    virtual PublicRequest* get(unsigned _idx) const = 0;
    virtual void reload(Database::Filters::Union& _filter) = 0;

    /// from CommonList; propably will be removed in future
    virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
    virtual void reload() = 0;
    virtual void sort(MemberType _member, bool _asc) = 0;
};

class Manager
{
public:
    virtual ~Manager() { }

    static Manager* create(
            Domain::Manager* _domain_manager,
            Contact::Manager* _contact_manager,
            Nsset::Manager* _nsset_manager,
            Keyset::Manager* _keyset_manager,
            Mailer::Manager* _mailer_manager,
            Document::Manager* _doc_manager,
            Messages::ManagerPtr _messages_manager);

    virtual Mailer::Manager* getMailerManager() const = 0;
    virtual Document::Manager* getDocumentManager() const = 0;
    virtual Messages::ManagerPtr getMessagesManager() const = 0;

    virtual List* createList() const = 0;
    virtual List* loadRequest(Database::ID id) const = 0;
    virtual void getPdf(Database::ID _id,
                        const std::string& _lang,
                        std::ostream& _output) const = 0;

    virtual PublicRequest* createRequest(Type _type) const = 0;

    virtual void processRequest(Database::ID _id,
                                bool _invalidate,
                                bool _check,
                                unsigned long long _request_id = 0) const = 0;

    virtual unsigned long long processAuthRequest(
            const std::string &_identification,
            const std::string &_password,
            unsigned long long _request_id) = 0;

    virtual bool checkAlreadyProcessedPublicRequest(
            unsigned long long _contact_id,
            const std::vector<Type>& _request_type_list) = 0;

    virtual std::string getPublicRequestAuthIdentification(
            unsigned long long _contact_id,
            const std::vector<Type> &_request_type_list) = 0;

    /* config - this should be in contructor */
    virtual const std::string& getIdentificationMailAuthHostname() const = 0;
    virtual bool getDemoMode() const = 0;
    virtual void setIdentificationMailAuthHostname(const std::string &_hostname) = 0;
    virtual void setDemoMode(bool _demo_mode) = 0;
};

using Factory = Util::Factory<PublicRequest>;

const Factory& get_default_factory();

std::vector<std::string> get_enum_public_request_type();

void lock_public_request_by_object(unsigned long long object_id);
void lock_public_request_lock(const std::string& identification);
void lock_public_request_id(unsigned long long public_request_id);

}//namespace LibFred::PublicRequest
}//namespace LibFred

#endif//PUBLIC_REQUEST_HH_2EEC5CA83E364803BF617BF5F3166AEC
