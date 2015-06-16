/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/fredlib/public_request/create_public_request_auth.h"
#include "src/fredlib/public_request/update_public_request.h"
#include "src/fredlib/contact/create_contact.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/util.h"

#include <map>
#include <boost/test/unit_test.hpp>

const std::string server_name = "test-update-public-request";

struct update_public_request_fixture : virtual Test::Fixture::instantiate_db_template,
                                       private Fred::PublicRequestTypeIface
{
    update_public_request_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        public_request_type(*this),
        bad_enum_status(static_cast< Fred::PublicRequestStatus >(NUMBER_OF_STATES))
    {
        Fred::OperationContextCreator ctx;
        Database::Result dbres = ctx.get_conn().exec(
            "SELECT id,handle FROM registrar WHERE system ORDER BY id LIMIT 1");
        BOOST_CHECK(dbres.size() == 1);//expecting existing system registrar
        registrar_id = static_cast< Fred::RegistrarId >(dbres[0][0]);
        const std::string registrar_handle = static_cast< std::string >(dbres[0][1]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar
        const std::string contact_handle = "TEST-UPDATE-PUBLIC-REQUEST-CONTACT-HANDLE" + xmark;
        const std::string contact_name = "Testupdatepublicrequest Contactname" + xmark;
        Fred::Contact::PlaceAddress place;
        place.street1 = "Veřejná 5";
        place.city = "Žádostín u Měnína";
        place.postalcode = "32100";
        place.country = "CZ";
        Fred::CreateContact(contact_handle, registrar_handle)
            .set_place(place)
            .set_name(contact_name)
            .exec(ctx);
        dbres = ctx.get_conn().exec_params(
            "SELECT obr.id "
            "FROM object_registry obr "
            "JOIN enum_object_type eot ON eot.id=obr.type AND eot.name='contact' "
            "WHERE obr.name=$1::TEXT AND obr.erdate IS NULL",
            Database::query_param_list(contact_handle));
        BOOST_CHECK(dbres.size() == 1);//expecting existing system registrar
        contact_id = static_cast< Fred::ObjectId >(dbres[0][0]);
        Fred::PublicRequestObjectLockGuard locked_contact(ctx, contact_id);

        dbres = ctx.get_conn().exec(
            "SELECT name FROM enum_public_request_type "
            "ORDER BY id OFFSET (SELECT COUNT(*)/2 FROM enum_public_request_type) LIMIT 1");
        BOOST_CHECK(dbres.size() == 1);//expecting existing public request type
        type_name_ = static_cast< std::string >(dbres[0][0]);
        create_result = Fred::CreatePublicRequestAuth(public_request_type, "h*vno kleslo")
            .exec(ctx, locked_contact);
        ctx.commit_transaction();
    }

    virtual ~update_public_request_fixture() { }

protected:
    const std::string xmark;
    Fred::RegistrarId registrar_id;
    Fred::ObjectId contact_id;
    Fred::CreatePublicRequestAuth::Result create_result;
    const Fred::PublicRequestTypeIface &public_request_type;
    enum { NUMBER_OF_STATES = 3 };
    const Fred::PublicRequestStatus bad_enum_status;
private:
    std::string get_public_request_type()const { return type_name_; }
    std::string type_name_;
};

BOOST_FIXTURE_TEST_SUITE(TestUpdatePublicRequest, update_public_request_fixture)

/**
 * test public_request_status conversion functions
 */
BOOST_AUTO_TEST_CASE(public_request_status_conversions)
{
    Fred::OperationContextCreator ctx;

    typedef std::set< std::string > StatusName;
    StatusName status_names;
    {
        const Database::Result res = ctx.get_conn().exec("SELECT name FROM enum_public_request_status ORDER BY id");
        BOOST_CHECK(res.size() == NUMBER_OF_STATES);
        for (::size_t idx = 0; idx < res.size(); ++idx) {
            status_names.insert(static_cast< std::string >(res[idx][0]));
        }
        BOOST_CHECK(status_names.size() == res.size());
    }
    static const std::string known_status[NUMBER_OF_STATES] = {"new", "answered", "invalidated"};
    for (int idx = 0; idx < NUMBER_OF_STATES; ++idx) {
        BOOST_CHECK(status_names.count(known_status[idx]) == 1);
        const Fred::PublicRequestStatus enum_status = Fred::str2public_request_status(known_status[idx]);
        BOOST_CHECK(Fred::public_request_status2str(enum_status) == known_status[idx]);
    }

    BOOST_CHECK_EXCEPTION(
    try {
        const std::string bad_status = "newx";
        BOOST_CHECK(status_names.count(bad_status) == 0);
        Fred::str2public_request_status(bad_status);
    }
    catch(const Fred::PublicRequestStatusBadConversion &e) {
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);

    BOOST_CHECK_EXCEPTION(
    try {
        BOOST_CHECK(Fred::PRS_NEW < bad_enum_status);
        BOOST_CHECK(Fred::PRS_ANSWERED < bad_enum_status);
        BOOST_CHECK(Fred::PRS_INVALIDATED < bad_enum_status);
        Fred::public_request_status2str(bad_enum_status);
    }
    catch(const Fred::PublicRequestStatusBadConversion &e) {
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

/**
 * test PublicRequestLockGuardById
 */
BOOST_AUTO_TEST_CASE(public_request_lock_guard_by_id)
{
    Fred::OperationContextCreator ctx;

    const Fred::PublicRequestId bad_id = static_cast< Fred::PublicRequestId >(ctx.get_conn().exec(
        "SELECT 100+2*MAX(id) FROM public_request")[0][0]);
    BOOST_CHECK_EXCEPTION(
    const Fred::PublicRequestLockGuardById locked(ctx, create_result.public_request_id);
    BOOST_CHECK(locked.get_public_request_id() == create_result.public_request_id);
    try {
        Fred::PublicRequestLockGuardById(ctx, bad_id);
    }
    catch(const Fred::PublicRequestLockGuardById::Exception &e) {
        BOOST_CHECK(e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(e.get_public_request_doesnt_exist() == bad_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

/**
 * test PublicRequestLockGuardByIdentification
 */
BOOST_AUTO_TEST_CASE(public_request_lock_guard_by_identification)
{
    Fred::OperationContextCreator ctx;

    const std::string bad_identification = create_result.identification + " ";
    BOOST_CHECK_EXCEPTION(
    const Fred::PublicRequestLockGuardByIdentification locked(ctx, create_result.identification);
    BOOST_CHECK(locked.get_public_request_id() == create_result.public_request_id);
    try {
        Fred::PublicRequestLockGuardByIdentification(ctx, bad_identification);
    }
    catch(const Fred::PublicRequestLockGuardByIdentification::Exception &e) {
        BOOST_CHECK(e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(e.get_public_request_doesnt_exist() == bad_identification);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

/**
 * test UpdatePublicRequest without changes
 */
BOOST_AUTO_TEST_CASE(update_public_request_without_changes)
{
    Fred::OperationContextCreator ctx;

    BOOST_CHECK_EXCEPTION(
    try {
        Fred::UpdatePublicRequest()
            .exec(ctx, Fred::PublicRequestLockGuardById(ctx, create_result.public_request_id));
    }
    catch(const Fred::UpdatePublicRequest::Exception &e) {
        BOOST_CHECK(e.is_set_nothing_to_do());
        BOOST_CHECK(!e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(!e.is_set_unknown_email_id());
        BOOST_CHECK(!e.is_set_unknown_registrar_id());
        BOOST_CHECK(!e.is_set_bad_public_request_status());
        BOOST_CHECK(e.get_nothing_to_do() == create_result.public_request_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

class PublicRequestLockGuardFake:public Fred::PublicRequestLockGuard
{
public:
    PublicRequestLockGuardFake(Fred::PublicRequestId _public_request_id):PublicRequestLockGuard(_public_request_id) { }
};

/**
 * test UpdatePublicRequest with wrong public request id
 */
BOOST_AUTO_TEST_CASE(update_public_request_wrong_public_request_id)
{
    Fred::OperationContextCreator ctx;

    const Fred::PublicRequestId bad_public_request_id = static_cast< Fred::RegistrarId >(ctx.get_conn().exec(
        "SELECT 100+2*MAX(id) FROM public_request")[0][0]);
    BOOST_CHECK_EXCEPTION(
    try {
        Fred::UpdatePublicRequest()
            .set_status(Fred::PRS_NEW)
            .exec(ctx, PublicRequestLockGuardFake(bad_public_request_id));
    }
    catch(const Fred::UpdatePublicRequest::Exception &e) {
        BOOST_CHECK(!e.is_set_nothing_to_do());
        BOOST_CHECK(e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(!e.is_set_unknown_email_id());
        BOOST_CHECK(!e.is_set_unknown_registrar_id());
        BOOST_CHECK(!e.is_set_bad_public_request_status());
        BOOST_CHECK(e.get_public_request_doesnt_exist() == bad_public_request_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

/**
 * test UpdatePublicRequest with wrong email or registrar
 */
BOOST_AUTO_TEST_CASE(update_public_request_wrong_email_or_registrar)
{
    Fred::OperationContextCreator ctx;

    const Database::Result dbres = ctx.get_conn().exec(
        "SELECT (SELECT 100+2*MAX(id) FROM mail_archive),100+2*MAX(id) FROM registrar");
    const Fred::UpdatePublicRequest::EmailId bad_email_id =
        static_cast< Fred::UpdatePublicRequest::EmailId >(dbres[0][0]);
    const Fred::RegistrarId bad_registrar_id = static_cast< Fred::RegistrarId >(dbres[0][1]);
    BOOST_CHECK_EXCEPTION(
    try {
        Fred::UpdatePublicRequest()
            .set_answer_email_id(bad_email_id)
            .exec(ctx, Fred::PublicRequestLockGuardById(ctx, create_result.public_request_id));
    }
    catch(const Fred::UpdatePublicRequest::Exception &e) {
        BOOST_CHECK(!e.is_set_nothing_to_do());
        BOOST_CHECK(!e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(e.is_set_unknown_email_id());
        BOOST_CHECK(!e.is_set_unknown_registrar_id());
        BOOST_CHECK(!e.is_set_bad_public_request_status());
        BOOST_CHECK(e.get_unknown_email_id() == bad_email_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);

    BOOST_CHECK_EXCEPTION(
    try {
        Fred::UpdatePublicRequest()
            .set_registrar_id(bad_registrar_id)
            .exec(ctx, Fred::PublicRequestLockGuardById(ctx, create_result.public_request_id));
    }
    catch(const Fred::UpdatePublicRequest::Exception &e) {
        BOOST_CHECK(!e.is_set_nothing_to_do());
        BOOST_CHECK(!e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(!e.is_set_unknown_email_id());
        BOOST_CHECK(e.is_set_unknown_registrar_id());
        BOOST_CHECK(!e.is_set_bad_public_request_status());
        BOOST_CHECK(e.get_unknown_registrar_id() == bad_registrar_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);

    BOOST_CHECK_EXCEPTION(
    try {
        Fred::UpdatePublicRequest()
            .set_answer_email_id(bad_email_id)
            .set_registrar_id(bad_registrar_id)
            .exec(ctx, Fred::PublicRequestLockGuardById(ctx, create_result.public_request_id));
    }
    catch(const Fred::UpdatePublicRequest::Exception &e) {
        BOOST_CHECK(!e.is_set_nothing_to_do());
        BOOST_CHECK(!e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(e.is_set_unknown_email_id());
        BOOST_CHECK(e.is_set_unknown_registrar_id());
        BOOST_CHECK(!e.is_set_bad_public_request_status());
        BOOST_CHECK(e.get_unknown_email_id() == bad_email_id);
        BOOST_CHECK(e.get_unknown_registrar_id() == bad_registrar_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

/**
 * test UpdatePublicRequest with wrong public request status
 */
BOOST_AUTO_TEST_CASE(update_public_request_wrong_public_request_status)
{
    Fred::OperationContextCreator ctx;

    BOOST_CHECK_EXCEPTION(
    try {
        Fred::UpdatePublicRequest()
            .set_status(bad_enum_status)
            .exec(ctx, Fred::PublicRequestLockGuardById(ctx, create_result.public_request_id));
    }
    catch(const Fred::UpdatePublicRequest::Exception &e) {
        BOOST_CHECK(!e.is_set_nothing_to_do());
        BOOST_CHECK(!e.is_set_public_request_doesnt_exist());
        BOOST_CHECK(!e.is_set_unknown_email_id());
        BOOST_CHECK(!e.is_set_unknown_registrar_id());
        BOOST_CHECK(e.is_set_bad_public_request_status());
        BOOST_CHECK(e.get_bad_public_request_status() == bad_enum_status);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e) {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...) {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

/**
 * test UpdatePublicRequest
 */
BOOST_AUTO_TEST_CASE(update_public_request_ok)
{
    Fred::OperationContextCreator ctx;
    Fred::PublicRequestLockGuardById locked_request(ctx, create_result.public_request_id);
    const Fred::PublicRequestStatus enum_status = Fred::PRS_ANSWERED;
    const std::string str_status = Fred::public_request_status2str(enum_status);
    const std::string reason = "Prostě proto.";
    const std::string email = "noreply@nic.cz";
    const Fred::UpdatePublicRequest::EmailId email_id =
        static_cast< Fred::UpdatePublicRequest::EmailId >(ctx.get_conn().exec(
        "SELECT MAX(id) FROM mail_archive")[0][0]);
    const Fred::UpdatePublicRequest::RequestId create_request_id = 10;
    const Fred::UpdatePublicRequest::RequestId resolve_request_id = 2 * create_request_id + 10;
    Fred::UpdatePublicRequest::Result result = Fred::UpdatePublicRequest()
        .set_status(enum_status)
        .set_resolve_time_to_now()
        .set_reason(reason)
        .set_email_to_answer(email)
        .set_answer_email_id(email_id)
        .set_registrar_id(registrar_id)
        .set_create_request_id(create_request_id)
        .set_resolve_request_id(resolve_request_id)
        .exec(ctx, locked_request);
    BOOST_CHECK(result.public_request_id == create_result.public_request_id);
    BOOST_CHECK(result.public_request_type == public_request_type.get_public_request_type());
    BOOST_CHECK(result.object_id == contact_id);
    Database::Result res = ctx.get_conn().exec_params(
        "SELECT "
            "id,"
            "(SELECT name=$2::TEXT FROM enum_public_request_type WHERE id=pr.request_type),"
            "create_time<NOW(),"
            "(SELECT name=$3::TEXT FROM enum_public_request_status WHERE id=pr.status),"
            "resolve_time=NOW(),"
            "reason=$4::TEXT,"
            "email_to_answer=$5::TEXT,"
            "answer_email_id=$6::BIGINT,"
            "registrar_id=$7::BIGINT,"
            "create_request_id=$8::BIGINT,"
            "resolve_request_id=$9::BIGINT "
        "FROM public_request pr "
        "WHERE id=$1::BIGINT",
        Database::query_param_list
            (locked_request.get_public_request_id())
            (public_request_type.get_public_request_type())
            (str_status)
            (reason)
            (email)
            (email_id)
            (registrar_id)
            (create_request_id)
            (resolve_request_id));
    BOOST_CHECK(res.size() == 1);
    BOOST_CHECK(static_cast< Fred::PublicRequestId >(res[0][0]) == create_result.public_request_id);
    BOOST_CHECK(!res[0][1].isnull() && static_cast< bool >(res[0][1]));
    BOOST_CHECK(static_cast< bool >(res[0][2]));
    BOOST_CHECK(!res[0][3].isnull() && static_cast< bool >(res[0][3]));
    BOOST_CHECK(!res[0][4].isnull() && static_cast< bool >(res[0][4]));
    BOOST_CHECK(!res[0][5].isnull() && static_cast< bool >(res[0][5]));
    BOOST_CHECK(!res[0][6].isnull() && static_cast< bool >(res[0][6]));
    BOOST_CHECK(!res[0][7].isnull() && static_cast< bool >(res[0][7]));
    BOOST_CHECK(!res[0][8].isnull() && static_cast< bool >(res[0][8]));
    BOOST_CHECK(!res[0][9].isnull() && static_cast< bool >(res[0][9]));
    BOOST_CHECK(!res[0][10].isnull() && static_cast< bool >(res[0][10]));

    const std::string resolve_time = "2015-05-14 10:11:12";
    result = Fred::UpdatePublicRequest()
        .set_resolve_time(boost::posix_time::time_from_string(resolve_time))
        .set_reason(Nullable< std::string >())
        .set_email_to_answer(Nullable< std::string >())
        .set_answer_email_id(Nullable< Fred::UpdatePublicRequest::EmailId >())
        .set_registrar_id(Nullable< Fred::RegistrarId >())
        .set_create_request_id(Nullable< Fred::UpdatePublicRequest::RequestId >())
        .set_resolve_request_id(Nullable< Fred::UpdatePublicRequest::RequestId >())
        .exec(ctx, locked_request);
    BOOST_CHECK(result.public_request_id == create_result.public_request_id);
    BOOST_CHECK(result.public_request_type == public_request_type.get_public_request_type());
    BOOST_CHECK(result.object_id == contact_id);
    res = ctx.get_conn().exec_params(
        "SELECT "
            "id,"
            "(SELECT name=$2::TEXT FROM enum_public_request_type WHERE id=pr.request_type),"
            "create_time<NOW(),"
            "(SELECT name=$3::TEXT FROM enum_public_request_status WHERE id=pr.status),"
            "resolve_time=$4::TIMESTAMP WITHOUT TIME ZONE,"
            "reason IS NULL,"
            "email_to_answer IS NULL,"
            "answer_email_id IS NULL,"
            "registrar_id IS NULL,"
            "create_request_id IS NULL,"
            "resolve_request_id IS NULL "
        "FROM public_request pr "
        "WHERE id=$1::BIGINT",
        Database::query_param_list
            (locked_request.get_public_request_id())
            (public_request_type.get_public_request_type())
            (str_status)
            (resolve_time));
    BOOST_CHECK(res.size() == 1);
    BOOST_CHECK(static_cast< Fred::PublicRequestId >(res[0][0]) == create_result.public_request_id);
    BOOST_CHECK(!res[0][1].isnull() && static_cast< bool >(res[0][1]));
    BOOST_CHECK(static_cast< bool >(res[0][2]));
    BOOST_CHECK(!res[0][3].isnull() && static_cast< bool >(res[0][3]));
    BOOST_CHECK(!res[0][4].isnull() && static_cast< bool >(res[0][4]));
    BOOST_CHECK(static_cast< bool >(res[0][5]));
    BOOST_CHECK(static_cast< bool >(res[0][6]));
    BOOST_CHECK(static_cast< bool >(res[0][7]));
    BOOST_CHECK(static_cast< bool >(res[0][8]));
    BOOST_CHECK(static_cast< bool >(res[0][9]));
    BOOST_CHECK(static_cast< bool >(res[0][10]));

    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END();//TestUpdatePublicRequest
