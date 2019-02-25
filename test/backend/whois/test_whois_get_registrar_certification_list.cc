/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
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
#include "test/backend/whois/fixture_common.hh"

#include "libfred/db_settings.hh"
#include "util/random_data_generator.hh"
#include "src/backend/whois/registrar_certification.hh"

BOOST_AUTO_TEST_SUITE(TestWhois)
BOOST_AUTO_TEST_SUITE(get_registrar_certification_list)

struct certification_list_fixture
: whois_impl_instance_fixture
{
    std::vector<Fred::Backend::Whois::RegistrarCertificationData> fixture_certs;

    certification_list_fixture()
    {
        LibFred::OperationContextCreator ctx;
        fixture_certs = Fred::Backend::Whois::get_registrar_certifications(ctx);
        for (int i = 0; i < 5; ++i) //XXX
        {
            std::string name("test-certification");
            RandomDataGenerator rdg;
            name += rdg.xnumstring(6);
            const LibFred::InfoRegistrarData& data = Test::registrar::make(ctx);
            Fred::Backend::Whois::RegistrarCertificationData rcd(data.handle, rdg.xuint() % 4 + 1, 1); // XXX
            ctx.get_conn().exec_params(
                    "INSERT INTO registrar_certification (registrar_id, valid_from, valid_until, classification, eval_file_id) "
                    "VALUES ($1::bigint, now()::date, now()::date, $2::int, 1) ", // XXX
                    Database::query_param_list(data.id)(rcd.get_registrar_score()));
            fixture_certs.push_back(rcd);
        }
        ctx.commit_transaction();
    }
};

BOOST_FIXTURE_TEST_CASE(get_registrar_certification_list, certification_list_fixture)
{
    typedef Fred::Backend::Whois::RegistrarCertification Certification;
    std::vector<Certification> cert_vec = impl.get_registrar_certification_list();
    BOOST_CHECK(cert_vec.size() == fixture_certs.size());
    BOOST_FOREACH(const Certification& it, cert_vec)
    {
        bool is_in_other = false;
        BOOST_FOREACH(const Fred::Backend::Whois::RegistrarCertificationData& fc_it, fixture_certs)
        {
            if (it.score == fc_it.get_registrar_score() &&
                it.evaluation_file_id == fc_it.get_registrar_evaluation_file_id() && 
                it.registrar == fc_it.get_registrar_handle())
            {
                is_in_other = true;
            }
        }
        BOOST_CHECK(is_in_other);
    }
}

BOOST_AUTO_TEST_SUITE_END()//get_registrar_certification_list
BOOST_AUTO_TEST_SUITE_END()//TestWhois
