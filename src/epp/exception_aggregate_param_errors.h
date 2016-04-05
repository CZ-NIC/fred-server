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

#ifndef EPP_EXCEPTION_AGGREGATE_PARAM_ERRORS_854853143541
#define EPP_EXCEPTION_AGGREGATE_PARAM_ERRORS_854853143541

#include "src/epp/error.h"

#include <set>

namespace Epp {

    class AggregatedParamErrors {
        private:
            std::set<Error> param_errors_;

        public:
            void add(const Error& _new_error) {
                param_errors_.insert(_new_error);
            }

            std::set<Error> get() const {
                return param_errors_;
            }

            bool is_empty() const { return param_errors_.empty(); }
    };
}

#endif
