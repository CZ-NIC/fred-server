/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "setup_server_decl.h"
#include "time_clock.h"

#include "cfg/handle_general_args.h"
#include "cfg/handle_server_args.h"
#include "cfg/handle_logging_args.h"
#include "cfg/handle_database_args.h"
#include "cfg/handle_threadgroup_args.h"
#include "cfg/handle_corbanameservice_args.h"

//not using UTF defined main
#define BOOST_TEST_NO_MAIN

#include "cfg/config_handler_decl.h"
#include <boost/test/unit_test.hpp>

#include "decimal/mpdecimal-2.2/mpdecimal.h"

BOOST_AUTO_TEST_SUITE(TestDecimal)

const std::string server_name = "test-decimal";

BOOST_AUTO_TEST_CASE( test_decimal )
{

    mpd_context_t ctx;
    mpd_t *a, *b;
    mpd_t *result;
    char *rstring;

    char status_str[MPD_MAX_FLAG_STRING];
    clock_t start_clock, end_clock;


    mpd_init(&ctx, 38);
    ctx.traps = 0;

    result = mpd_new(&ctx);
    a = mpd_new(&ctx);
    b = mpd_new(&ctx);


    mpd_set_string(a, "11.111", &ctx);
    mpd_set_string(b, "12.222", &ctx);

    start_clock = clock();

    mpd_add(result,a,b,&ctx);

    end_clock = clock();
    //fprintf(stderr, "time: %f\n\n", (double)(end_clock-start_clock)/(double)CLOCKS_PER_SEC);

    rstring = mpd_to_sci(result, 1);
    mpd_snprint_flags(status_str, MPD_MAX_FLAG_STRING, ctx.status);

    //printf("%s  %s\n", rstring, status_str);

    BOOST_CHECK_EQUAL(std::string("23.333").compare(std::string(rstring)),0);

    mpd_del(a);
    mpd_del(b);
    mpd_del(result);
    mpd_free(rstring);
}

BOOST_AUTO_TEST_SUITE_END();//TestDecimal
