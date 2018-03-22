/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#include "src/libfred/registrar/certification/create_registrar_certification.hh"
#include "src/libfred/registrar/certification/get_registrar_certifications.hh"
#include "src/libfred/registrar/certification/exceptions.hh"
#include "src/libfred/registrar/certification/registrar_certification_type.hh"

#include "src/libfred/db_settings.hh"
#include "src/libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/unit_test.hpp>
#include <boost/date_time/gregorian/greg_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include <algorithm>

struct test_get_certifications_fixture : virtual public Test::instantiate_db_template
{
    LibFred::InfoRegistrarData test_registrar;
    unsigned int certifications_amount;
    std::vector<LibFred::Registrar::RegistrarCertification> reg_certs;

    test_get_certifications_fixture()
    : certifications_amount(3)
    {
        LibFred::OperationContextCreator ctx;
        test_registrar = Test::registrar::make(ctx);
        int file_id = ctx.get_conn().exec(
                "INSERT INTO files (name, path, filesize, filetype) "
                "VALUES ('update_file', "
                "CONCAT(TO_CHAR(current_timestamp, 'YYYY/fmMM/fmD'), '/', CURRVAL('files_id_seq'::regclass))::text, "
                "0, 6) RETURNING id;")[0][0];
        int score = 1;
        LibFred::Registrar::RegistrarCertification rc;
        rc.valid_until = boost::gregorian::day_clock::local_day();
        for (int i = 0; i < certifications_amount; ++i)
        {
            rc.valid_from = rc.valid_until + boost::gregorian::date_duration(1);
            rc.valid_until = rc.valid_from + boost::gregorian::date_duration(1);
            rc.id = LibFred::Registrar::CreateRegistrarCertification(
                    test_registrar.id, rc.valid_from, rc.valid_until, score, file_id)
                .exec(ctx);
            rc.classification = score++;
            rc.eval_file_id = file_id;
            reg_certs.push_back(rc);
        }
        ctx.commit_transaction();
        std::reverse(reg_certs.begin(), reg_certs.end());
    }
};

BOOST_FIXTURE_TEST_SUITE(TestGetRegistrarCertifications, test_get_certifications_fixture)

BOOST_AUTO_TEST_CASE(get_registrar_certifications)
{
    LibFred::OperationContextCreator ctx;
    std::vector<LibFred::Registrar::RegistrarCertification> result = LibFred::Registrar::GetRegistrarCertifications(test_registrar.id).exec(ctx);

    BOOST_CHECK(result.size() == certifications_amount);
    for (int i = 0; i < certifications_amount; ++i)
    {
        BOOST_CHECK(reg_certs.at(i) == result.at(i));
    }
}

BOOST_AUTO_TEST_CASE(registrar_not_found)
{
    LibFred::OperationContextCreator ctx;
    Database::Result reg_id = ctx.get_conn().exec(
            "select nextval('registrar_id_seq'::regclass)");
    BOOST_CHECK_THROW(
        LibFred::Registrar::GetRegistrarCertifications(static_cast<int>(reg_id[0][0]) + 1).exec(ctx),
        RegistrarNotFound);
}

BOOST_AUTO_TEST_SUITE_END(); // TestGetRegistrarCertifications
