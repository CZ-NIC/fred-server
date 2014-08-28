#include <boost/algorithm/string/join.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include "tests/fredlib/contact/verification/setup_utils.h"
#include "tests/setup/fixtures_utils.h"

#include "src/fredlib/contact/verification/create_check.h"
#include "src/fredlib/contact/verification/create_test.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/verification/enum_test_status.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "util/random.h"
#include "util/random_data_generator.h"

setup_testdef::setup_testdef() {
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            testdef_handle_ = "TEST_" + RandomDataGenerator().xnumstring(15);
            testdef_id_ = static_cast<long>(
                ctx.get_conn().exec(
                    "INSERT INTO enum_contact_test "
                    "   (id, handle) "
                    "   VALUES ("+RandomDataGenerator().xnumstring(9)+", '"+testdef_handle_+"') "
                    "   RETURNING id;"
                )[0][0]);

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }
}

setup_nonexistent_testdef_handle::setup_nonexistent_testdef_handle() {
    Database::Result res;
    // prevent handle collisions
    do {
        Fred::OperationContext ctx;

        testdef_handle = "NONEX_TEST_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT handle FROM enum_contact_testsuite WHERE handle='"+testdef_handle+"';" );
    } while(res.size() != 0);
}

setup_testdef_in_testsuite::setup_testdef_in_testsuite(const std::string& testdef_handle, const std::string& testsuite_handle) {
    Fred::OperationContext ctx;

    Database::Result res = ctx.get_conn().exec(
        "INSERT INTO contact_testsuite_map "
        "   (enum_contact_test_id, enum_contact_testsuite_id) "
        "   VALUES ("
        "       (SELECT id FROM enum_contact_test WHERE handle='"+testdef_handle+"' ), "
        "       (SELECT id FROM enum_contact_testsuite WHERE handle='"+testsuite_handle+"') "
        "   ) "
        "   RETURNING enum_contact_test_id;");

    ctx.commit_transaction();

    if(res.size() != 1) {
        throw std::runtime_error("inserting testdef to testsuite");
    }

}

setup_testdef_in_testsuite_of_check::setup_testdef_in_testsuite_of_check(const std::string testdef_handle, const std::string check_handle) {
    Fred::OperationContext ctx;

    Database::Result res =
        ctx.get_conn().exec(
            "INSERT INTO contact_testsuite_map "
            "   (enum_contact_test_id, enum_contact_testsuite_id) "
            "   VALUES ("
            "       (SELECT id FROM enum_contact_test WHERE handle='"+testdef_handle+"' ), "
            "       (SELECT enum_contact_testsuite_id FROM contact_check WHERE handle='"+check_handle+"') "
            "   ) "
            "   RETURNING enum_contact_test_id;"
        );

    ctx.commit_transaction();

    if(res.size() != 1) {
        throw std::runtime_error("inserting testdef to testsuite");
    }
}

setup_empty_testsuite::setup_empty_testsuite() {
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            testsuite_handle = "TESTSUITE_" + RandomDataGenerator().xnumstring(15) ;
            testsuite_id = static_cast<long>(
                ctx.get_conn().exec(
                    "INSERT INTO enum_contact_testsuite "
                    "   (id, handle)"
                    "   VALUES ("+RandomDataGenerator().xnumstring(9)+", '"+testsuite_handle+"')"
                    "   RETURNING id;"
                )[0][0]);

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }
}

setup_empty_testsuite::setup_empty_testsuite(const std::string& _testsuite_handle)
    : testsuite_handle(_testsuite_handle)
{
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            testsuite_id = static_cast<long>(
                ctx.get_conn().exec(
                    "INSERT INTO enum_contact_testsuite "
                    "   (id, handle)"
                    "   VALUES ("+RandomDataGenerator().xnumstring(9)+", '"+testsuite_handle+"')"
                    "   RETURNING id;"
                )[0][0]);

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }
}

setup_testsuite::setup_testsuite() {
    testdefs.push_back(setup_testdef());
    setup_testdef_in_testsuite(testdefs.front().testdef_handle_, testsuite_handle);
}

setup_nonexistent_testsuite_handle::setup_nonexistent_testsuite_handle() {
    Database::Result res;
    // prevent handle collisions
    do {
        Fred::OperationContext ctx;

        testsuite_handle = "NONEX_TESTSUITE_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT handle FROM enum_contact_testsuite WHERE handle='"+testsuite_handle+"';" );
    } while(res.size() != 0);

}

setup_logd_request_id::setup_logd_request_id() {
    logd_request_id = RandomDataGenerator().xuint();
}

setup_check_status::setup_check_status() {
    Database::Result res;
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            status_handle = "STATUS_" + RandomDataGenerator().xnumstring(15);
            res = ctx.get_conn().exec(
                "INSERT "
                "   INTO enum_contact_check_status "
                "   (id, handle) "
                "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '"+status_handle+"') "
                "   RETURNING id;" );

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }

    if(res.size()!=1) {
        throw std::runtime_error("creating check status failed");
    }
}

setup_check_status::setup_check_status(const std::string& _handle)
    : status_handle(_handle)
{
    Database::Result res;

    while(true) {
        try {
            Fred::OperationContext ctx;

            res = ctx.get_conn().exec(
                "INSERT "
                "   INTO enum_contact_check_status "
                "   (id, handle ) "
                "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '"+status_handle+"') "
                "   RETURNING id;" );

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
        continue;
    }
    break;
            }
    if(res.size()!=1) {
        throw std::runtime_error("creating check status failed");
    }
}

setup_test_status::setup_test_status() {
    Database::Result res;
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            status_handle_ = "STATUS_" + RandomDataGenerator().xnumstring(15);
            res = ctx.get_conn().exec(
                "INSERT "
                "   INTO enum_contact_test_status "
                "   (id, handle ) "
                "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '"+status_handle_+"') "
                "   RETURNING id;" );

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }
    if(res.size()!=1) {
        throw std::runtime_error("creating test status failed");
    }
}

setup_test_status::setup_test_status(const std::string& _handle)
    : status_handle_(_handle)
{
    Database::Result res;

    while(true) {
        try {
            Fred::OperationContext ctx;

            res = ctx.get_conn().exec(
                "INSERT "
                "   INTO enum_contact_test_status "
                "   (id, handle ) "
                "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '" + status_handle_ + "') "
                "   RETURNING id;" );

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }
    if(res.size()!=1) {
        throw std::runtime_error("creating test status failed");
    }
}

setup_error_msg::setup_error_msg() {
    error_msg = "ERROR_MSG_" + RandomDataGenerator().xnumstring(20);
}

setup_check::setup_check(const std::string& _testsuite_handle, Optional<unsigned long long> _logd_request)
    : logd_request_(_logd_request)
{
    // check
    Fred::CreateContactCheck create_check(
        contact_.info_data.id,
        _testsuite_handle,
        logd_request_
    );

    Fred::OperationContext ctx;
    check_handle_ = create_check.exec(ctx);
    ctx.commit_transaction();
}

setup_nonexistent_check_handle::setup_nonexistent_check_handle() {

    Fred::OperationContext ctx;

    Database::Result res;
    do {
        check_handle = boost::lexical_cast<std::string>(boost::uuids::random_generator()());
        res = ctx.get_conn().exec(
            "SELECT handle "
            "   FROM contact_check "
            "   WHERE handle='"+check_handle+"';"
        );
    } while(res.size() != 0);
}

setup_nonexistent_check_status_handle::setup_nonexistent_check_status_handle() {
    Fred::OperationContext ctx;
    // prevent handle collisions
    Database::Result res;
    do {
        status_handle = "STATUS_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT handle "
            "   FROM enum_contact_check_status "
            "   WHERE handle='"+status_handle+"';" );
    } while(res.size() != 0);
}

setup_test::setup_test(
    const std::string& _check_handle,
    const std::string& _testdef_handle,
    Optional<unsigned long long> _logd_request
) :
    testdef_handle_(_testdef_handle),
    logd_request_(_logd_request)
{
    Fred::CreateContactTest create_test(uuid::from_string(_check_handle), testdef_handle_, logd_request_);

    Fred::OperationContext ctx;
    create_test.exec(ctx);
    ctx.commit_transaction();
}

setup_nonexistent_test_status_handle::setup_nonexistent_test_status_handle() {
    Database::Result res;
    // prevent handle collisions
    do {
        Fred::OperationContext ctx;
        status_handle = "STATUS_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT handle "
            "   FROM enum_contact_test_status "
            "   WHERE handle='"+status_handle+"';" );
    } while(res.size() != 0);
}
