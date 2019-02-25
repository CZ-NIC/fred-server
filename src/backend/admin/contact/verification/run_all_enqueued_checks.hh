/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  run all contact verification checks in queue
 */

#ifndef RUN_ALL_ENQUEUED_CHECKS_HH_98FF71CFFEEF434FAD74FEBACFD1DAE5
#define RUN_ALL_ENQUEUED_CHECKS_HH_98FF71CFFEEF434FAD74FEBACFD1DAE5

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "src/backend/admin/contact/verification/test_impl/test_interface.hh"

namespace Fred {
namespace Backend {
namespace Admin {
namespace Contact {
namespace Verification {

/**
 * Randomly (by happenstance, not even pseudo-random) runs all enqueued checks one by one
 *
 * @param _tests map of test objects denoted by their name
 * @return handles of executed (finalized) check ordered by execution (first in vector - first executed)
 */
std::vector<std::string> run_all_enqueued_checks(
        const std::map<std::string, std::shared_ptr<Test>>& _tests,
        Optional<unsigned long long> _logd_request_id = Optional<unsigned long long>());


} // namespace Fred::Backend::Admin::Contact::Verification
} // namespace Fred::Backend::Admin::Contact
} // namespace Fred::Backend::Admin
} // namespace Fred::Backend
} // namespace Fred

#endif
