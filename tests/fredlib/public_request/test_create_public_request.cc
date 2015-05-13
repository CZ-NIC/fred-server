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
#include "src/fredlib/contact/create_contact.h"

#include "util/random_data_generator.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "tests/fredlib/util.h"

#include <boost/test/unit_test.hpp>

const std::string server_name = "test-create-public-request";

struct create_public_request_fixture : public virtual Test::Fixture::instantiate_db_template
{
    create_public_request_fixture()
    :   xmark(RandomDataGenerator().xnumstring(6))
    {
        Fred::OperationContext ctx;
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

DECLARE_EXCEPTION_DATA(unknown_registrar_handle, std::string);

/**
 * test CreatePublicRequest with wrong registrar
 */
BOOST_AUTO_TEST_CASE(create_public_request_wrong_registrar)
{
    Fred::OperationContext ctx;

    const Fred::RegistrarId bad_registrar_id = static_cast< Fred::RegistrarId >(ctx.get_conn().exec(
        "SELECT 100+2*MAX(id) FROM registrar")[0][0]);
    BOOST_CHECK_EXCEPTION(
    try
    {
        Fred::CreatePublicRequest(PublicRequestTypeFake("mojeid_contact_identification"))
            .set_registrar_id(bad_registrar_id)
            .exec(ctx, Fred::PublicRequestObjectLockGuard(ctx, contact_id));
    }
    catch(const Fred::CreatePublicRequest::Exception &e)
    {
        BOOST_CHECK(e.is_set_unknown_registrar());
        BOOST_CHECK(e.get_unknown_registrar() == bad_registrar_id);
        BOOST_TEST_MESSAGE(boost::diagnostic_information(e));
        throw;
    }
    catch(const std::exception &e)
    {
        BOOST_ERROR(boost::diagnostic_information(e));
        throw;
    }
    catch(...)
    {
        BOOST_ERROR("unexpected exception occurs");
        throw;
    },
    std::exception,
    check_std_exception);
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateContact
