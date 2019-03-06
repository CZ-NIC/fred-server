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
#ifndef SETUP_UTILS_HH_4F0FB7CE3105470AB918373331EBBE5E
#define SETUP_UTILS_HH_4F0FB7CE3105470AB918373331EBBE5E

#include <string>
#include <vector>

#include "test/setup/fixtures_utils.hh"

struct setup_testdef {
    long testdef_id_;
    std::string testdef_handle_;

    setup_testdef();
};

struct setup_nonexistent_testdef_handle {
    std::string testdef_handle;

    setup_nonexistent_testdef_handle();
};

struct setup_testdef_in_testsuite {
    setup_testdef_in_testsuite(const std::string& testdef_handle, const std::string& testsuite_handle);
};

struct setup_testdef_in_testsuite_of_check {
    setup_testdef_in_testsuite_of_check(const std::string testdef_handle, const std::string check_handle);
};

struct setup_empty_testsuite {
    long testsuite_id;
    std::string testsuite_handle;

    setup_empty_testsuite();

    setup_empty_testsuite(const std::string& _testsuite_handle);
};

struct setup_testsuite : public setup_empty_testsuite {
    std::vector<setup_testdef> testdefs;
    setup_testsuite();

};

struct setup_nonexistent_testsuite_handle {
    std::string testsuite_handle;

    setup_nonexistent_testsuite_handle();
};

struct setup_logd_request_id {
    unsigned long long logd_request_id;

    setup_logd_request_id();
};

struct setup_check_status {
    std::string status_handle;

    setup_check_status();
    setup_check_status(const std::string& _handle);
};

struct setup_test_status {
    std::string status_handle_;

    setup_test_status();
    setup_test_status(const std::string& _handle);
};

struct setup_error_msg {
    std::string error_msg;

    setup_error_msg();
};

struct setup_check {
    std::string check_handle_;
    Optional<unsigned long long> logd_request_;
    Test::contact contact_;

    setup_check(const std::string& _testsuite_handle, Optional<unsigned long long> _logd_request = Optional<unsigned long long>());
};

struct setup_nonexistent_check_handle {
    std::string check_handle;

    setup_nonexistent_check_handle();
};

struct setup_nonexistent_check_status_handle {
    std::string status_handle;

    setup_nonexistent_check_status_handle();
};

struct setup_test {
    std::string testdef_handle_;
    Optional<unsigned long long> logd_request_;

    setup_test(
        const std::string& _check_handle,
        const std::string& _testdef_handle,
        Optional<unsigned long long> _logd_request = Optional<unsigned long long>()
    );
};

struct setup_nonexistent_test_status_handle {
    std::string status_handle;

    setup_nonexistent_test_status_handle();
};

#endif
