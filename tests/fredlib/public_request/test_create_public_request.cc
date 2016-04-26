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

#include "src/fredlib/public_request/create_public_request.h"
#include "src/fredlib/public_request/public_request_status.h"
#include "src/fredlib/contact/create_contact.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/util.h"
#include "tests/fredlib/enum_to_db_handle_conversion.h"

#include <boost/test/unit_test.hpp>

const std::string server_name = "test-create-public-request";

struct create_public_request_fixture : public virtual Test::Fixture::instantiate_db_template
{
    create_public_request_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6))
    {
        Fred::OperationContextCreator ctx;
        Database::Result dbres = ctx.get_conn().exec(
            "SELECT id,handle FROM registrar WHERE system ORDER BY id LIMIT 1");
        BOOST_CHECK(dbres.size() == 1);//expecting existing system registrar
        registrar_id = static_cast< Fred::RegistrarId >(dbres[0][0]);
        const std::string registrar_handle = static_cast< std::string >(dbres[0][1]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar
        const std::string contact_handle = "TEST-CREATE-PUBLIC-REQUEST-CONTACT-HANDLE" + xmark;
        const std::string contact_name = "Testcreatepublicrequest Contactname" + xmark;
        Fred::Contact::PlaceAddress place;
        place.street1 = "Veřejná 5";
        place.city = "Žádostín u Stvořenova";
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
        ctx.commit_transaction();
    }
    ~create_public_request_fixture()
    {}

    const std::string xmark;
    Fred::RegistrarId registrar_id;
    Fred::ObjectId contact_id;
};

class PublicRequestTypeFake:public Fred::PublicRequestTypeIface
{
public:
    PublicRequestTypeFake(const std::string &_type):type_(_type) { }
    std::string get_public_request_type()const { return type_; }
    ~PublicRequestTypeFake() { }
private:
    const std::string type_;
};

BOOST_FIXTURE_TEST_SUITE(TestCreatePublicRequest, create_public_request_fixture)

/**
 * test public_request_status conversion functions
 */
BOOST_AUTO_TEST_CASE(public_request_status_conversions)
{
    Fred::OperationContextCreator ctx;
    static const char *const sql = "SELECT name FROM enum_public_request_status";
    enum_to_db_handle_conversion_test< Fred::PublicRequest::Status, 3 >(ctx, sql);
}

/**
 * test PublicRequestObjectLockGuard with wrong object_id
 */
BOOST_AUTO_TEST_CASE(public_request_object_lock_guard_wrong_id)
{
    Fred::OperationContextCreator ctx;

    const Fred::ObjectId bad_object_id = static_cast< Fred::RegistrarId >(ctx.get_conn().exec(
        "SELECT 100+2*MAX(id) FROM object")[0][0]);
    BOOST_CHECK_EXCEPTION(
    try {
        Fred::PublicRequestsOfObjectLockGuardByObjectId(ctx, bad_object_id);
    }
    catch(const Fred::PublicRequestsOfObjectLockGuardByObjectId::Exception &e) {
        BOOST_CHECK(e.is_set_object_doesnt_exist());
        BOOST_CHECK(e.get_object_doesnt_exist() == bad_object_id);
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
 * test CreatePublicRequest with wrong registrar
 */
BOOST_AUTO_TEST_CASE(create_public_request_wrong_registrar)
{
    Fred::OperationContextCreator ctx;

    const Fred::RegistrarId bad_registrar_id = static_cast< Fred::RegistrarId >(ctx.get_conn().exec(
        "SELECT 100+2*MAX(id) FROM registrar")[0][0]);
    BOOST_CHECK_EXCEPTION(
    try {
        Fred::CreatePublicRequest(PublicRequestTypeFake("mojeid_contact_identification"))
            .set_registrar_id(bad_registrar_id)
            .exec(Fred::PublicRequestsOfObjectLockGuardByObjectId(ctx, contact_id));
    }
    catch(const Fred::CreatePublicRequest::Exception &e) {
        BOOST_CHECK(!e.is_set_unknown_type());
        BOOST_CHECK(e.is_set_unknown_registrar_id());
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
 * test CreatePublicRequest with wrong public request type
 */
BOOST_AUTO_TEST_CASE(create_public_request_wrong_type)
{
    Fred::OperationContextCreator ctx;
    const std::string bad_type = "absolutely_wrong_prt";

    BOOST_CHECK_EXCEPTION(
    try {
        Fred::CreatePublicRequest(PublicRequestTypeFake(bad_type))
            .exec(Fred::PublicRequestsOfObjectLockGuardByObjectId(ctx, contact_id));
    }
    catch(const Fred::CreatePublicRequest::Exception &e) {
        BOOST_CHECK(e.is_set_unknown_type());
        BOOST_CHECK(!e.is_set_unknown_registrar_id());
        BOOST_CHECK(e.get_unknown_type() == bad_type);
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
 * test CreatePublicRequest
 */
BOOST_AUTO_TEST_CASE(create_public_request_ok)
{
    Fred::OperationContextCreator ctx;
    typedef std::vector< std::string > TypeName;
    TypeName type_names;
    {
        const Database::Result res = ctx.get_conn().exec("SELECT name FROM enum_public_request_type ORDER BY id");
        BOOST_CHECK(0 < res.size());
        for (::size_t idx = 0; idx < res.size(); ++idx) {
            type_names.push_back(static_cast< std::string >(res[idx][0]));
        }
        BOOST_CHECK(type_names.size() == res.size());
    }

    Fred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, contact_id);
    for (TypeName::const_iterator name_ptr = type_names.begin(); name_ptr != type_names.end(); ++name_ptr) {
        const PublicRequestTypeFake public_request_type(*name_ptr);
        const Fred::PublicRequestId public_request_id = Fred::CreatePublicRequest(public_request_type)
            .exec(locked_contact);
        const Database::Result res = ctx.get_conn().exec_params(
            "SELECT "
                "id,"
                "(SELECT name=$2::TEXT FROM enum_public_request_type WHERE id=pr.request_type),"
                "create_time=NOW(),"
                "(SELECT name='new' FROM enum_public_request_status WHERE id=pr.status),"
                "resolve_time IS NULL,"
                "reason IS NULL,"
                "email_to_answer IS NULL,"
                "answer_email_id IS NULL,"
                "registrar_id IS NULL,"
                "create_request_id IS NULL,"
                "resolve_request_id IS NULL "
            "FROM public_request pr "
            "WHERE id=$1::BIGINT",
            Database::query_param_list(public_request_id)(*name_ptr));
        BOOST_CHECK(res.size() == 1);
        BOOST_CHECK(static_cast< Fred::PublicRequestId >(res[0][0]) == public_request_id);
        BOOST_CHECK(!res[0][1].isnull() && static_cast< bool >(res[0][1]));
        BOOST_CHECK(!res[0][2].isnull() && static_cast< bool >(res[0][2]));
        BOOST_CHECK(!res[0][3].isnull() && static_cast< bool >(res[0][3]));
        BOOST_CHECK(static_cast< bool >(res[0][4]));
        BOOST_CHECK(static_cast< bool >(res[0][5]));
        BOOST_CHECK(static_cast< bool >(res[0][6]));
        BOOST_CHECK(static_cast< bool >(res[0][7]));
        BOOST_CHECK(static_cast< bool >(res[0][8]));
        BOOST_CHECK(static_cast< bool >(res[0][9]));
        BOOST_CHECK(static_cast< bool >(res[0][10]));
    }

    const std::string reason = "Naprosto bezdůvodně!";
    const std::string email_to_answer = "noreply@nic.cz";
    BOOST_CHECK(reason != email_to_answer);
    const PublicRequestTypeFake public_request_type(*type_names.begin());
    Fred::PublicRequestId public_request_id[2];
    public_request_id[0] = Fred::CreatePublicRequest(
        public_request_type, reason, email_to_answer, registrar_id)
        .exec(locked_contact);
    public_request_id[1] = Fred::CreatePublicRequest(public_request_type)
        .set_reason(reason)
        .set_email_to_answer(email_to_answer)
        .set_registrar_id(registrar_id)
        .exec(locked_contact);
    const Database::Result res = ctx.get_conn().exec_params(
        "SELECT "
            "id,"
            "(SELECT name=$3::TEXT FROM enum_public_request_type WHERE id=pr.request_type),"
            "create_time=NOW(),"
            "(SELECT name FROM enum_public_request_status WHERE id=pr.status),"
            "resolve_time IS NULL,"
            "reason=$4::TEXT,"
            "email_to_answer=$5::TEXT,"
            "answer_email_id IS NULL,"
            "registrar_id=$6::BIGINT,"
            "create_request_id IS NULL,"
            "resolve_request_id IS NULL "
        "FROM public_request pr "
        "WHERE id IN ($1::BIGINT,$2::BIGINT) ORDER BY id",
        Database::query_param_list(public_request_id[0])(public_request_id[1])
                                  (*type_names.begin())(reason)(email_to_answer)(registrar_id));
    BOOST_CHECK(res.size() == 2);
    for (int idx = 0; idx < 2; ++idx) {
        BOOST_CHECK(static_cast< Fred::PublicRequestId >(res[idx][0]) == public_request_id[idx]);
        BOOST_CHECK(!res[idx][1].isnull() && static_cast< bool >(res[idx][1]));
        BOOST_CHECK(!res[idx][2].isnull() && static_cast< bool >(res[idx][2]));
        const std::string status = Conversion::Enums::to_db_handle(idx == 0
                                                                   ? Fred::PublicRequest::Status::invalidated
                                                                   : Fred::PublicRequest::Status::active);
        BOOST_CHECK(!res[idx][3].isnull() && (static_cast< std::string >(res[idx][3]) == status));
        BOOST_CHECK(static_cast< bool >(res[idx][4]) == (idx != 0));
        BOOST_CHECK(!res[idx][5].isnull() && static_cast< bool >(res[idx][5]));
        BOOST_CHECK(!res[idx][6].isnull() && static_cast< bool >(res[idx][6]));
        BOOST_CHECK(static_cast< bool >(res[idx][7]));
        BOOST_CHECK(!res[idx][8].isnull() && static_cast< bool >(res[idx][8]));
        BOOST_CHECK(static_cast< bool >(res[idx][9]));
        BOOST_CHECK(static_cast< bool >(res[idx][10]));
    }

    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END();//TestCreatePublicRequest
