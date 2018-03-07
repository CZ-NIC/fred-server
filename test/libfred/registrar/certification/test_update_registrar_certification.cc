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
#include "src/libfred/registrar/certification/update_registrar_certification.hh"
#include "src/libfred/registrar/certification/exceptions.hh"

#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"
#include "src/libfred/opcontext.hh"

#include <boost/date_time/gregorian/greg_duration.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/test/unit_test.hpp>

struct test_update_certification_fixture : virtual public Test::instantiate_db_template
{
    LibFred::InfoRegistrarData test_registrar;
    unsigned long long certification_id;
    boost::gregorian::date valid_from, valid_until;
    const int score;
    int file_id;

    test_update_certification_fixture()
    : score(1)
    {
        LibFred::OperationContextCreator ctx;
        test_registrar = Test::registrar::make(ctx);
        valid_from = boost::gregorian::day_clock::local_day() + boost::gregorian::date_duration(1);
        valid_until = valid_from + boost::gregorian::date_duration(1);
        file_id = ctx.get_conn().exec(
                "insert into files (name, path, filesize, filetype) "
                "values ('update_file', "
                "concat(to_char(current_timestamp, 'YYYY/fmMM/fmD'), '/', currval('files_id_seq'::regclass))::text, "
                "0, 6) returning id;")[0][0];
        certification_id = LibFred::CreateRegistrarCertification(test_registrar.id,
                valid_from, valid_until, score, file_id)
            .exec(ctx);
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestUpdateRegistrarCertification, test_update_certification_fixture)
    
BOOST_AUTO_TEST_CASE(update_registrar_certification_date)
{
    LibFred::OperationContextCreator ctx;
    valid_until -= boost::gregorian::date_duration(1);
    LibFred::UpdateRegistrarCertification(certification_id, valid_until).exec(ctx);
    Database::Result result = ctx.get_conn().exec_params(
            "select * from registrar_certification "
            "where id = $1::bigint",
            Database::query_param_list(certification_id));
    BOOST_CHECK(test_registrar.id == static_cast<unsigned long long>(result[0][1]));
    BOOST_CHECK(to_iso_extended_string(valid_from) == static_cast<std::string>(result[0][2]));
    BOOST_CHECK(to_iso_extended_string(valid_until) == static_cast<std::string>(result[0][3]));
    BOOST_CHECK(score == static_cast<int>(result[0][4]));
    BOOST_CHECK(file_id == static_cast<int>(result[0][5]));
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_CASE(update_registrar_certification_score_file)
{
    LibFred::OperationContextCreator ctx;
    const int new_score = 2;
    LibFred::UpdateRegistrarCertification(certification_id, new_score, file_id).exec(ctx);
    Database::Result result = ctx.get_conn().exec_params(
            "select * from registrar_certification "
            "where id = $1::bigint",
            Database::query_param_list(certification_id));
    BOOST_CHECK(test_registrar.id == static_cast<unsigned long long>(result[0][1]));
    BOOST_CHECK(to_iso_extended_string(valid_from) == static_cast<std::string>(result[0][2]));
    BOOST_CHECK(to_iso_extended_string(valid_until) == static_cast<std::string>(result[0][3]));
    BOOST_CHECK(new_score == static_cast<int>(result[0][4]));
    BOOST_CHECK(file_id == static_cast<int>(result[0][5]));
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_CASE(certification_in_past)
{
    LibFred::OperationContextCreator ctx;
    valid_until -= boost::gregorian::date_duration(3);
    BOOST_CHECK_THROW(
        LibFred::UpdateRegistrarCertification(certification_id, valid_until).exec(ctx),
        CertificationInPast);
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_CASE(certification_extension)
{
    LibFred::OperationContextCreator ctx;
    valid_until += boost::gregorian::date_duration(1);
    BOOST_CHECK_THROW(
        LibFred::UpdateRegistrarCertification(certification_id, valid_until).exec(ctx),
        CertificationExtension);
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_CASE(wrong_interval_order)
{
    LibFred::OperationContextCreator ctx;
    valid_until -= boost::gregorian::date_duration(2);
    BOOST_CHECK_THROW(
        LibFred::UpdateRegistrarCertification(certification_id, valid_until).exec(ctx),
        WrongIntervalOrder);
    ctx.commit_transaction();
}

BOOST_AUTO_TEST_CASE(score_overcome)
{
    LibFred::OperationContextCreator ctx;
    const int score = 6;
    boost::gregorian::date valid_from(boost::gregorian::day_clock::local_day());
    boost::gregorian::date valid_until(valid_from + boost::gregorian::date_duration(1));
    BOOST_CHECK_THROW(
        LibFred::UpdateRegistrarCertification(certification_id, score, file_id)
            .exec(ctx),
        ScoreOutOfRange);
}

BOOST_AUTO_TEST_SUITE_END(); //TestCreateRegistrarCertification
