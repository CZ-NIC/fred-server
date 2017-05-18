/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#include "src/fredlib/poll/create_request_fee_info_message.h"
#include "src/fredlib/poll/get_request_fee_message.h"
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"
#include "src/fredlib/opcontext.h"

#include <boost/test/unit_test.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time_adjustor.hpp>

BOOST_AUTO_TEST_SUITE(TestPoll)
BOOST_AUTO_TEST_SUITE(TestPollRequestFeeInfoMessages)

namespace {

struct PollRequestFeeInfoMessages : Test::instantiate_db_template
{
    Fred::OperationContextCreator ctx;
    unsigned long long message_id;
    unsigned long long registrar_id;
    Fred::Poll::RequestFeeInfoEvent golden_output;

    PollRequestFeeInfoMessages()
    {
        const Test::registrar registrar(ctx);
        registrar_id = registrar.info_data.id;
        golden_output.from = boost::posix_time::time_from_string("2017-03-30 14:16:02.713506");
        golden_output.to = boost::posix_time::time_from_string("2017-03-30 14:17:07.113492");

        golden_output.free_count = 20;
        golden_output.used_count = 10;
        golden_output.price = "1024.42";
    }

    void test()
    {
        // sadly the timestamp is converted from local time to utc when stored, but not converted back when fetched
        typedef boost::date_time::local_adjustor<boost::posix_time::ptime, +2, boost::posix_time::no_dst> cest;

        const Fred::Poll::CreateRequestFeeInfoMessage message_creator(
            registrar_id,
            cest::utc_to_local(golden_output.from),
            cest::utc_to_local(golden_output.to),
            golden_output.free_count,
            golden_output.used_count,
            golden_output.price,
            2);
        BOOST_CHECK_NO_THROW(message_id = message_creator.exec(ctx));
        Fred::Poll::RequestFeeInfoEvent output;
        BOOST_CHECK_NO_THROW(output = Fred::Poll::get_last_request_fee_info_message(ctx, registrar_id));

        BOOST_CHECK_EQUAL(golden_output.from, output.from);
        BOOST_CHECK_EQUAL(golden_output.to, output.to);
        BOOST_CHECK_EQUAL(golden_output.free_count, output.free_count);
        BOOST_CHECK_EQUAL(golden_output.used_count, output.used_count);
        BOOST_CHECK_EQUAL(golden_output.price, output.price);

        Fred::Poll::RequestFeeInfoEvent output2;
        BOOST_CHECK_NO_THROW(output2 =
                             Fred::Poll::get_request_fee_info_message(ctx,
                                                                      registrar_id,
                                                                      cest::utc_to_local(golden_output.to)));
        BOOST_CHECK_EQUAL(golden_output.from, output2.from);
        BOOST_CHECK_EQUAL(golden_output.to, output2.to);
        BOOST_CHECK_EQUAL(golden_output.free_count, output2.free_count);
        BOOST_CHECK_EQUAL(golden_output.used_count, output2.used_count);
        BOOST_CHECK_EQUAL(golden_output.price, output2.price);

    }
};

} // namespace {anonymous}

BOOST_FIXTURE_TEST_CASE(fredlib_request_fee_info_message, PollRequestFeeInfoMessages)
{
    test();
}

BOOST_AUTO_TEST_SUITE_END();
BOOST_AUTO_TEST_SUITE_END();
