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

#include "src/libfred/public_request/create_public_request_auth.hh"
#include "src/libfred/public_request/public_request_status.hh"
#include "src/libfred/public_request/public_request_auth_type_iface.hh"
#include "src/libfred/registrable_object/contact/create_contact.hh"

#include "src/util/random_data_generator.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "test/libfred/util.hh"
#include "test/libfred/enum_to_db_handle_conversion.hh"

#include <boost/test/unit_test.hpp>

const std::string server_name = "test-create-public-request-auth";

struct create_public_request_auth_fixture : public virtual Test::instantiate_db_template
{
    create_public_request_auth_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6)),
        password("Km92bm8ga2xlc2xv")
    {
        ::LibFred::OperationContextCreator ctx;
        Database::Result dbres = ctx.get_conn().exec(
            "SELECT id,handle FROM registrar WHERE system ORDER BY id LIMIT 1");
        BOOST_CHECK(dbres.size() == 1);//expecting existing system registrar
        registrar_id = static_cast< ::LibFred::RegistrarId >(dbres[0][0]);
        const std::string registrar_handle = static_cast< std::string >(dbres[0][1]);
        BOOST_CHECK(!registrar_handle.empty());//expecting existing system registrar
        const std::string contact_handle = "TEST-CREATE-PUBLIC-REQUEST-AUTH-CONTACT-HANDLE" + xmark;
        const std::string contact_name = "Testcreatepublicrequestauth Contactname" + xmark;
        ::LibFred::Contact::PlaceAddress place;
        place.street1 = "Veřejně autentizační 5";
        place.city = "Žádostín u Stvořenova";
        place.postalcode = "32100";
        place.country = "CZ";
        ::LibFred::CreateContact(contact_handle, registrar_handle)
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
        contact_id = static_cast< ::LibFred::ObjectId >(dbres[0][0]);
        ctx.commit_transaction();
    }
    ~create_public_request_auth_fixture()
    {}

    const std::string xmark;
    const std::string password;
    ::LibFred::RegistrarId registrar_id;
    ::LibFred::ObjectId contact_id;
};

namespace {

class PublicRequestAuthTypeFake:public ::LibFred::PublicRequestAuthTypeIface
{
public:
    PublicRequestAuthTypeFake(const std::string &_type,
                              const std::string &_password)
    :   type_(_type),
        password_(_password) { }
    virtual std::string get_public_request_type()const { return type_; }
    virtual std::string generate_passwords(const ::LibFred::LockedPublicRequestsOfObjectForUpdate&)const { return password_; }
    virtual ~PublicRequestAuthTypeFake() { }
private:
    PublicRequestTypes get_public_request_types_to_cancel_on_create()const
    {
        PublicRequestTypes result;
        result.insert(IfacePtr(new PublicRequestAuthTypeFake(type_, password_)));
        return result;
    }
    PublicRequestTypes get_public_request_types_to_cancel_on_update(
        ::LibFred::PublicRequest::Status::Enum _old_status, ::LibFred::PublicRequest::Status::Enum _new_status)const
    {
        PublicRequestTypes result;
        if ((_old_status == ::LibFred::PublicRequest::Status::opened) &&
            (_new_status == ::LibFred::PublicRequest::Status::answered)) {
        }
        return result;
    }
    const std::string type_;
    const std::string password_;
};

}

BOOST_FIXTURE_TEST_SUITE(TestCreatePublicRequestAuth, create_public_request_auth_fixture)

/**
 * test public_request_status conversion functions
 */
BOOST_AUTO_TEST_CASE(public_request_status_conversions)
{
    ::LibFred::OperationContextCreator ctx;
    static const char *const sql = "SELECT name FROM enum_public_request_status";
    enum_to_db_handle_conversion_test< ::LibFred::PublicRequest::Status, 3 >(ctx, sql);
}

/**
 * test CreatePublicRequestAuth with wrong registrar
 */
BOOST_AUTO_TEST_CASE(create_public_request_auth_wrong_registrar)
{
    ::LibFred::OperationContextCreator ctx;

    const ::LibFred::RegistrarId bad_registrar_id = static_cast< ::LibFred::RegistrarId >(ctx.get_conn().exec(
        "SELECT 100+2*MAX(id) FROM registrar")[0][0]);
    BOOST_CHECK_EXCEPTION(
    try {
        ::LibFred::CreatePublicRequestAuth()
            .set_registrar_id(bad_registrar_id)
            .exec(::LibFred::PublicRequestsOfObjectLockGuardByObjectId(ctx, contact_id),
                  PublicRequestAuthTypeFake("mojeid_contact_identification", password));
    }
    catch(const ::LibFred::CreatePublicRequestAuth::Exception &e) {
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
 * test CreatePublicRequestAuth with wrong public request type
 */
BOOST_AUTO_TEST_CASE(create_public_request_auth_wrong_type)
{
    ::LibFred::OperationContextCreator ctx;
    const std::string bad_type = "absolutely_wrong_prt";

    BOOST_CHECK_EXCEPTION(
    try {
        ::LibFred::CreatePublicRequestAuth()
            .exec(::LibFred::PublicRequestsOfObjectLockGuardByObjectId(ctx, contact_id),
                  PublicRequestAuthTypeFake(bad_type, password));
    }
    catch(const ::LibFred::CreatePublicRequestAuth::Exception &e) {
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
 * test CreatePublicRequestAuth
 */
BOOST_AUTO_TEST_CASE(create_public_request_auth_ok)
{
    ::LibFred::OperationContextCreator ctx;
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

    ::LibFred::PublicRequestsOfObjectLockGuardByObjectId locked_contact(ctx, contact_id);
    for (TypeName::const_iterator name_ptr = type_names.begin(); name_ptr != type_names.end(); ++name_ptr) {
        const PublicRequestAuthTypeFake public_request_type(*name_ptr, password);
        const ::LibFred::CreatePublicRequestAuth::Result result = ::LibFred::CreatePublicRequestAuth()
            .exec(locked_contact, public_request_type);
        BOOST_CHECK(result.identification != result.password);
        BOOST_CHECK(result.identification.length() == ::LibFred::PUBLIC_REQUEST_AUTH_IDENTIFICATION_LENGTH);
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
        BOOST_CHECK(static_cast< ::LibFred::PublicRequestId >(res[0][0]) == result.public_request_id);
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
    const PublicRequestAuthTypeFake public_request_type(*type_names.begin(), password);
    ::LibFred::CreatePublicRequestAuth::Result result[2];
    result[0] = ::LibFred::CreatePublicRequestAuth(reason, email_to_answer, registrar_id)
        .exec(locked_contact, public_request_type);
    result[1] = ::LibFred::CreatePublicRequestAuth()
        .set_reason(reason)
        .set_email_to_answer(email_to_answer)
        .set_registrar_id(registrar_id)
        .exec(locked_contact, public_request_type);
    const Database::Result res = ctx.get_conn().exec_params(
        "SELECT "
            "pr.id,"
            "(SELECT name=$7::TEXT FROM enum_public_request_type WHERE id=pr.request_type),"
            "pr.create_time=NOW(),"
            "(SELECT name FROM enum_public_request_status WHERE id=pr.status),"
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
        BOOST_CHECK(static_cast< ::LibFred::PublicRequestId >(res[idx][0]) == result[idx].public_request_id);
        BOOST_CHECK(!res[idx][1].isnull() && static_cast< bool >(res[idx][1]));
        BOOST_CHECK(!res[idx][2].isnull() && static_cast< bool >(res[idx][2]));
        const std::string status = Conversion::Enums::to_db_handle(idx == 0
                                                                   ? ::LibFred::PublicRequest::Status::invalidated
                                                                   : ::LibFred::PublicRequest::Status::opened);
        BOOST_CHECK(!res[idx][3].isnull() && (static_cast< std::string >(res[idx][3]) == status));
        BOOST_CHECK(static_cast< bool >(res[idx][4]) == (idx != 0));
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

BOOST_AUTO_TEST_SUITE_END();//TestCreatePublicRequestAuth
