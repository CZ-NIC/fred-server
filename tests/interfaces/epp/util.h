/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 */

#ifndef TEST_INTERFACE_EPP_UTIL_3563545411254
#define TEST_INTERFACE_EPP_UTIL_3563545411254

#include <vector>
#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <boost/test/unit_test.hpp>
#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

#include "src/epp/exception.h"
#include "src/epp/exception_aggregate_param_errors.h"
#include "src/epp/error.h"

namespace Test {

    struct autocommitting_context : virtual Fixture::instantiate_db_template {
        Fred::OperationContextCreator ctx;

        virtual ~autocommitting_context() {
            ctx.commit_transaction();
        }
    };

    inline void check_correct_aggregated_exception_was_thrown(const Epp::Error& _error) {

        bool correct_exception_type_thrown = false;

        try {
            throw;

        } catch(const Epp::AggregatedParamErrors& e) {

            correct_exception_type_thrown = true;

            const std::set<Epp::Error> errors = e.get();

            BOOST_CHECK(
                errors.find(_error)
                !=
                errors.end()
            );
        }

        BOOST_CHECK(correct_exception_type_thrown);
    }
}

#endif
