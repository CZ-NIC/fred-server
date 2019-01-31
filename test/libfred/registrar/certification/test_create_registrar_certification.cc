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

#include "libfred/registrar/certification/create_registrar_certification.hh"
#include "libfred/registrar/certification/exceptions.hh"

#include "libfred/opcontext.hh"
#include "test/setup/fixtures.hh"
#include "test/setup/fixtures_utils.hh"

#include <boost/test/unit_test.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

struct test_create_certification_fixture : virtual public Test::instantiate_db_template
{
    LibFred::InfoRegistrarData test_registrar;
    int file_id;
    boost::gregorian::date valid_from;

    test_create_certification_fixture()
            : valid_from(boost::gregorian::day_clock::local_day())
    {
        LibFred::OperationContextCreator ctx;
        test_registrar = Test::registrar::make(ctx);
        file_id = ctx.get_conn().exec(
                "INSERT INTO files (name, path, filesize, filetype) "
                "VALUES ('update_file', "
                "CONCAT(TO_CHAR(current_timestamp, 'YYYY/fmMM/fmD'), '/', CURRVAL('files_id_seq'::regclass))::text, "
                "0, 6) RETURNING id;")[0][0];
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_SUITE(TestCreateRegistrarCertification, test_create_certification_fixture)

BOOST_AUTO_TEST_CASE(create_registrar_certification)
{
    LibFred::OperationContextCreator ctx;
    const boost::gregorian::date valid_until(valid_from);
    constexpr unsigned score = 5;
    const unsigned long long id = LibFred::Registrar::CreateRegistrarCertification(test_registrar.id,
                valid_from, score, file_id)
            .set_valid_until(valid_until)
            .exec(ctx);
    Database::Result result = ctx.get_conn().exec_params(
            "SELECT * FROM registrar_certification "
            "WHERE id = $1::bigint",
            Database::query_param_list(id));
    BOOST_CHECK(test_registrar.id == static_cast<unsigned long long>(result[0][1]));
    BOOST_CHECK(to_iso_extended_string(valid_from) == static_cast<std::string>(result[0][2]));
    BOOST_CHECK(to_iso_extended_string(valid_until) == static_cast<std::string>(result[0][3]));
    BOOST_CHECK(score == static_cast<int>(result[0][4]));
    BOOST_CHECK(file_id == static_cast<int>(result[0][5]));
}

BOOST_AUTO_TEST_CASE(create_registrar_certification_without_expiration)
{
    LibFred::OperationContextCreator ctx;
    const boost::gregorian::date valid_until(boost::gregorian::not_a_date_time);
    constexpr unsigned score = 4;
    const unsigned long long id = LibFred::Registrar::CreateRegistrarCertification(test_registrar.id,
                valid_from, score, file_id)
            .set_valid_until(valid_until)
            .exec(ctx);
    Database::Result result = ctx.get_conn().exec_params(
            "SELECT registrar_id, valid_from, classification, eval_file_id FROM registrar_certification "
            "WHERE id = $1::bigint "
            "AND valid_until IS NULL ",
            Database::query_param_list(id));
    BOOST_CHECK(test_registrar.id == static_cast<unsigned long long>(result[0][0]));
    BOOST_CHECK(to_iso_extended_string(valid_from) == static_cast<std::string>(result[0][1]));
    BOOST_CHECK(score == static_cast<int>(result[0][2]));
    BOOST_CHECK(file_id == static_cast<int>(result[0][3]));
}

BOOST_AUTO_TEST_CASE(terminate_last_registrar_certification)
{
    LibFred::OperationContextCreator ctx;
    const boost::gregorian::date new_valid_from(valid_from + boost::gregorian::date_duration(1));
    unsigned score = 3;

    LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id).exec(ctx);

    score++;
    const unsigned long long id = LibFred::Registrar::CreateRegistrarCertification(test_registrar.id,
                new_valid_from, score, file_id).exec(ctx);

    Database::Result result = ctx.get_conn().exec_params(
            "SELECT id, valid_from, classification FROM registrar_certification "
            "WHERE registrar_id = $1::bigint "
            "AND valid_until IS NULL ",
            Database::query_param_list(test_registrar.id));
    BOOST_CHECK(result.size() == 1);
    BOOST_CHECK(id == static_cast<unsigned long long>(result[0][0]));
    BOOST_CHECK(to_iso_extended_string(new_valid_from) == static_cast<std::string>(result[0][1]));
    BOOST_CHECK(score == static_cast<unsigned>(result[0][2]));
}

BOOST_AUTO_TEST_CASE(wrong_interval_order)
{
    LibFred::OperationContextCreator ctx;
    const boost::gregorian::date valid_until(valid_from - boost::gregorian::date_duration(1));
    constexpr unsigned score = 0;
    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id)
            .set_valid_until(valid_until)
            .exec(ctx),
        WrongIntervalOrder);
}

BOOST_AUTO_TEST_CASE(invalid_date_from)
{
    LibFred::OperationContextCreator ctx;
    valid_from = boost::gregorian::date(not_a_date_time);
    constexpr unsigned score = 0;
    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id)
            .exec(ctx),
        InvalidDateFrom);
}

BOOST_AUTO_TEST_CASE(certification_in_past)
{
    LibFred::OperationContextCreator ctx;
    valid_from -= boost::gregorian::date_duration(1);
    const boost::gregorian::date valid_until(valid_from);
    constexpr unsigned score = 1;
    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id)
            .set_valid_until(valid_until)
            .exec(ctx),
        CertificationInPast);
}

BOOST_AUTO_TEST_CASE(overlapping_range)
{
    LibFred::OperationContextCreator ctx;
    boost::gregorian::date valid_until(valid_from);
    constexpr unsigned score = 2;
    LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id)
            .set_valid_until(valid_until)
            .exec(ctx);

    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id)
            .exec(ctx),
        OverlappingRange);

    valid_from -= boost::gregorian::date_duration(3);
    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id)
            .set_valid_until(valid_until)
            .exec(ctx),
        OverlappingRange);
}

BOOST_AUTO_TEST_CASE(score_overcome)
{
    LibFred::OperationContextCreator ctx;
    constexpr unsigned score = 6;
    BOOST_CHECK_THROW(
        LibFred::Registrar::CreateRegistrarCertification(test_registrar.id, valid_from, score, file_id)
            .exec(ctx),
        ScoreOutOfRange);
}

BOOST_AUTO_TEST_SUITE_END();//TestCreateRegistrarCertification
