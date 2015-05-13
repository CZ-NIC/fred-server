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
#include "src/fredlib/contact/create_contact.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/util.h"

#include <boost/test/unit_test.hpp>

const std::string server_name = "test-create-public-request-auth";

struct create_public_request_auth_fixture : public virtual Test::Fixture::instantiate_db_template
{
    create_public_request_auth_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        password("*ovno kleslo")
    {
        Fred::OperationContext ctx;
        Database::Result dbres = ctx.get_conn().exec(
            "SELECT id,handle FROM registrar WHERE system ORDER BY id LIMIT 1");
        BOOST_CHECK(dbres.size() == 1);//expecting existing system registrar
        registrar_id = static_cast< Fred::RegistrarId >(dbres[0][0]);
        const std::string registrar_handle = static_cast< std::string >(dbres[0][1]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar
        const std::string contact_handle = "TEST-CREATE-PUBLIC-REQUEST-AUTH-CONTACT-HANDLE" + xmark;
        const std::string contact_name = "Testcreatepublicrequestauth Contactname" + xmark;
        Fred::Contact::PlaceAddress place;
        place.street1 = "Veřejně autentizační 5";
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
    ~create_public_request_auth_fixture()
    {}

    const std::string xmark;
    const std::string password;
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

BOOST_FIXTURE_TEST_SUITE(TestCreatePublicRequestAuth, create_public_request_auth_fixture)

/**
 * test CreatePublicRequestAuth with wrong registrar
 */
BOOST_AUTO_TEST_CASE(create_public_request_auth_wrong_registrar)
{
    Fred::OperationContext ctx;

    const Fred::RegistrarId bad_registrar_id = static_cast< Fred::RegistrarId >(ctx.get_conn().exec(
        "SELECT 100+2*MAX(id) FROM registrar")[0][0]);
    BOOST_CHECK_EXCEPTION(
    try {
        Fred::CreatePublicRequestAuth(PublicRequestTypeFake("mojeid_contact_identification"), password)
            .set_registrar_id(bad_registrar_id)
            .exec(ctx, Fred::PublicRequestObjectLockGuard(ctx, contact_id));
    }
    catch(const Fred::CreatePublicRequestAuth::Exception &e) {
        BOOST_CHECK(e.is_set_unknown_registrar());
        BOOST_CHECK(e.get_unknown_registrar() == bad_registrar_id);
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
 * test CreatePublicRequestAuth with wrong public request type
 */
BOOST_AUTO_TEST_CASE(create_public_request_auth_wrong_type)
{
    Fred::OperationContext ctx;
    const std::string bad_type = "absolutely_wrong_prt";

    BOOST_CHECK_EXCEPTION(
    try {
        Fred::CreatePublicRequestAuth(PublicRequestTypeFake(bad_type), password)
            .exec(ctx, Fred::PublicRequestObjectLockGuard(ctx, contact_id));
    }
    catch(const Fred::CreatePublicRequestAuth::Exception &e) {
        BOOST_CHECK(e.is_set_unknown_type());
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
 * test CreatePublicRequestAuth
 */
BOOST_AUTO_TEST_CASE(create_public_request_auth_ok)
{
    Fred::OperationContext ctx;
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

    Fred::PublicRequestObjectLockGuard locked_contact(ctx, contact_id);
    for (TypeName::const_iterator name_ptr = type_names.begin(); name_ptr != type_names.end(); ++name_ptr) {
        const PublicRequestTypeFake public_request_type(*name_ptr);
        const Fred::CreatePublicRequestAuth::Result result = Fred::CreatePublicRequestAuth(public_request_type, password)
            .exec(ctx, locked_contact);
        BOOST_CHECK(result.identification != result.password);
        BOOST_CHECK(result.identification.length() == Fred::PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH);
        const Database::Result res = ctx.get_conn().exec_params(
            "SELECT "
                "pr.id,"
                "(SELECT name=$2::TEXT FROM enum_public_request_type WHERE id=pr.request_type),"
                "pr.create_time=NOW(),"
                "(SELECT name='new' FROM enum_public_request_status WHERE id=pr.status),"
                "pr.resolve_time IS NULL,"
                "pr.reason IS NULL,"
                "pr.email_to_answer IS NULL,"
                "pr.answer_email_id IS NULL,"
                "pr.registrar_id IS NULL,"
                "pr.create_request_id IS NULL,"
                "pr.resolve_request_id IS NULL,"
                "pra.id IS NOT NULL,"
                "pra.identification=$3::TEXT,"
                "pra.password=$4::TEXT "
            "FROM public_request pr "
            "LEFT JOIN public_request_auth pra ON pra.id=pr.id "
            "WHERE pr.id=$1::BIGINT",
            Database::query_param_list(result.public_request_id)(*name_ptr)(result.identification)(result.password));
        BOOST_CHECK(res.size() == 1);
        BOOST_CHECK(static_cast< Fred::PublicRequestId >(res[0][0]) == result.public_request_id);
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
        BOOST_CHECK(static_cast< bool >(res[0][11]));
        BOOST_CHECK(!res[0][12].isnull() && static_cast< bool >(res[0][12]));
        BOOST_CHECK(!res[0][13].isnull() && static_cast< bool >(res[0][13]));
    }

    const std::string reason = "Naprosto bezdůvodně!";
    const std::string email_to_answer = "noreply@nic.cz";
    BOOST_CHECK(reason != email_to_answer);
    const PublicRequestTypeFake public_request_type(*type_names.begin());
    Fred::CreatePublicRequestAuth::Result result[2];
    result[0] = Fred::CreatePublicRequestAuth(
        public_request_type, password, reason, email_to_answer, registrar_id)
        .exec(ctx, locked_contact);
    result[1] = Fred::CreatePublicRequestAuth(public_request_type, password)
        .set_reason(reason)
        .set_email_to_answer(email_to_answer)
        .set_registrar_id(registrar_id)
        .exec(ctx, locked_contact);
    const Database::Result res = ctx.get_conn().exec_params(
        "SELECT "
            "pr.id,"
            "(SELECT name=$7::TEXT FROM enum_public_request_type WHERE id=pr.request_type),"
            "pr.create_time=NOW(),"
            "(SELECT name='new' FROM enum_public_request_status WHERE id=pr.status),"
            "pr.resolve_time IS NULL,"
            "pr.reason=$8::TEXT,"
            "pr.email_to_answer=$9::TEXT,"
            "pr.answer_email_id IS NULL,"
            "pr.registrar_id=$10::BIGINT,"
            "pr.create_request_id IS NULL,"
            "pr.resolve_request_id IS NULL,"
            "pra.id IS NOT NULL,"
            "pr.id=$1::BIGINT AND pra.identification=$2::TEXT OR "
            "pr.id=$4::BIGINT AND pra.identification=$5::TEXT,"
            "pr.id=$1::BIGINT AND pra.password=$3::TEXT OR "
            "pr.id=$4::BIGINT AND pra.password=$6::TEXT "
        "FROM public_request pr "
        "LEFT JOIN public_request_auth pra ON pra.id=pr.id "
        "WHERE pr.id IN ($1::BIGINT,$4::BIGINT) ORDER BY pr.id",
        Database::query_param_list(result[0].public_request_id)(result[0].identification)(result[0].password)
                                  (result[1].public_request_id)(result[1].identification)(result[1].password)
                                  (*type_names.begin())(reason)(email_to_answer)(registrar_id));
    BOOST_CHECK(res.size() == 2);
    for (int idx = 0; idx < 2; ++idx) {
        BOOST_CHECK(static_cast< Fred::PublicRequestId >(res[idx][0]) == result[idx].public_request_id);
        BOOST_CHECK(!res[idx][1].isnull() && static_cast< bool >(res[idx][1]));
        BOOST_CHECK(!res[idx][2].isnull() && static_cast< bool >(res[idx][2]));
        BOOST_CHECK(!res[idx][3].isnull() && static_cast< bool >(res[idx][3]));
        BOOST_CHECK(static_cast< bool >(res[idx][4]));
        BOOST_CHECK(!res[idx][5].isnull() && static_cast< bool >(res[idx][5]));
        BOOST_CHECK(!res[idx][6].isnull() && static_cast< bool >(res[idx][6]));
        BOOST_CHECK(static_cast< bool >(res[idx][7]));
        BOOST_CHECK(!res[idx][8].isnull() && static_cast< bool >(res[idx][8]));
        BOOST_CHECK(static_cast< bool >(res[idx][9]));
        BOOST_CHECK(static_cast< bool >(res[idx][10]));
        BOOST_CHECK(static_cast< bool >(res[idx][11]));
        BOOST_CHECK(!res[idx][12].isnull() && static_cast< bool >(res[idx][12]));
        BOOST_CHECK(!res[idx][13].isnull() && static_cast< bool >(res[idx][13]));
    }

    ctx.commit_transaction();
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateContactAuth
