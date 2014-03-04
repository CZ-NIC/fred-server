#include <boost/algorithm/string/join.hpp>
#include <boost/uuid/uuid.hpp>            // uuid class
#include <boost/uuid/uuid_generators.hpp> // generators
#include <boost/uuid/uuid_io.hpp>         // streaming operators etc.

#include "tests/fredlib/contact/verification/setup_utils.h"

#include "src/fredlib/contact/verification/create_check.h"
#include "src/fredlib/contact/verification/create_test.h"
#include "src/fredlib/contact/verification/enum_testsuite_handle.h"
#include "src/fredlib/contact/verification/enum_check_status.h"
#include "src/fredlib/contact/verification/enum_test_status.h"
#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/contact/delete_contact.h"
#include "util/random.h"
#include "util/random_data_generator.h"

setup_get_registrar_handle::setup_get_registrar_handle( ) {
    Fred::OperationContext ctx;

    registrar_handle = static_cast<std::string>(
        ctx.get_conn().exec("SELECT handle FROM registrar LIMIT 1;")[0][0] );

    ctx.commit_transaction();

    if(registrar_handle.empty()) {
        throw std::runtime_error("no registrar found");
    }
}

setup_contact::setup_contact() {
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            contact_handle = "CONTACT_" + RandomDataGenerator().xnumstring(15);
            Fred::CreateContact create(contact_handle, registrar.registrar_handle);
            create.exec(ctx);

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        } catch (Fred::InternalError& ) {
            continue;
        }
        break;
    }

    Fred::OperationContext ctx_check;
    data_ = Fred::InfoContactByHandle(contact_handle).exec(ctx_check);

    contact_id_ = static_cast<unsigned long long>(
        ctx_check.get_conn().exec(
            "SELECT id "
            "   FROM contact "
            "   JOIN object_registry AS o_r USING(id) "
            "   WHERE o_r.name='" + contact_handle + "' "
        )[0][0]);
}

setup_nonexistent_contact_handle::setup_nonexistent_contact_handle() {
    Database::Result res;
    // guarantee non-existence
    do {
        Fred::OperationContext ctx;
        contact_handle = "NONEX_CONTACT_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT name "
            "   FROM object_registry "
            "   WHERE name='"+contact_handle+"'"
            "       AND type=1;" );
    } while(res.size() != 0);
}

setup_nonexistent_contact_id::setup_nonexistent_contact_id() {
    Database::Result res;
    // guarantee non-existence
    do {
        Fred::OperationContext ctx;
        contact_id_ = Random::integer(0, 2147000000);
        res = ctx.get_conn().exec(
            "SELECT id "
            "   FROM contact "
            "   WHERE id='" + boost::lexical_cast<std::string>(contact_id_) + "'" );
    } while(res.size() != 0);
}

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
        contact_.contact_id_,
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
    Fred::CreateContactTest create_test(_check_handle, testdef_handle_, logd_request_);

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

autoclean_contact_verification_db::T_foreign_keys autoclean_contact_verification_db::foreign_keys;

autoclean_contact_verification_db::autoclean_contact_verification_db() {
    Fred::OperationContext ctx;

    /*
    Database::Result pre_existing_res = ctx.get_conn().exec(
        "SELECT o_r.id AS object_id_ "
        "   FROM object_registry AS o_r ");

    for(Database::Result::Iterator it = pre_existing_res.begin(); it != pre_existing_res.end(); ++it) {
        object_ids_to_preserve_.push_back( static_cast<unsigned long long>( (*it)["object_id_"] ) );
    }
    */
    object_ids_to_preserve_.push_back(1);
    object_ids_to_preserve_.push_back(2);
    object_ids_to_preserve_.push_back(3);
    object_ids_to_preserve_.push_back(4);

    if(foreign_keys.empty()) {
        foreign_keys.push_back( boost::make_tuple(
            "object_id_fkey",
            "object", "id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "object_history_id_fkey",
            "object_history", "id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "contact_history_id_fkey",
            "contact_history", "id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "domain_history_id_fkey",
            "domain_history", "id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "nsset_history_id_fkey",
            "nsset_history", "id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "domain_contact_map_history_contactid_fkey",
            "domain_contact_map_history", "domainid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "nsset_contact_map_history_contactid_fkey",
            "nsset_contact_map_history", "contactid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "nsset_contact_map_history_nssetid_fkey",
            "nsset_contact_map_history", "nssetid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "nsset_contact_map_contactid_fkey",
            "nsset_contact_map", "contactid",
            "contact", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "keyset_contact_map_history_contactid_fkey",
            "keyset_contact_map_history", "keysetid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "object_state_object_id_fkey",
            "object_state", "object_id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "object_state_request_object_id_fkey",
            "object_state_request", "object_id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "contact_id_fkey",
            "contact", "id",
            "object", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "nsset_id_fkey",
            "nsset", "id",
            "object", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "domain_id_fkey",
            "domain", "id",
            "object", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "invoice_operation_object_id_fkey",
            "invoice_operation", "object_id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "public_request_objects_map_object_id_fkey",
            "public_request_objects_map", "object_id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "host_history_nssetid_fkey",
            "host_history", "nssetid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "host_ipaddr_map_history_nssetid_fkey",
            "host_ipaddr_map_history", "nssetid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "domain_contact_map_history_domainid_fkey",
            "domain_contact_map_history", "domainid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "enumval_history_domainid_fkey",
            "enumval_history", "domainid",
            "object_registry", "id"
        ));
/*
        foreign_keys.push_back( boost::make_tuple(
            "genzone_domain_history_domain_id_fkey",
            "genzone_domain_history", "domain_id",
            "object_registry", "id"
        ));
*/
        foreign_keys.push_back( boost::make_tuple(
            "keyset_history_id_fkey",
            "keyset_history", "id",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "keyset_contact_map_history_keysetid_fkey",
            "keyset_contact_map_history", "keysetid",
            "object_registry", "id"
        ));

        foreign_keys.push_back( boost::make_tuple(
            "keyset_contact_map_history_keysetid_fkey",
            "keyset_contact_map_history", "keysetid",
            "object_registry", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "reminder_contact_message_map_contact_id_fkey",
            "reminder_contact_message_map", "contact_id",
            "object_registry", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "invoice_operation_charge_map_invoice_operation_id_fkey",
            "invoice_operation_charge_map", "invoice_operation_id",
            "invoice_operation", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "public_request_state_request_map_state_request_id_fkey",
            "public_request_state_request_map", "state_request_id",
            "object_state_request", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "check_nsset_nsset_hid_fkey",
            "check_nsset", "nsset_hid",
            "nsset_history", "historyid"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "check_result_checkid_fkey",
            "check_result", "checkid",
            "check_nsset", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "poll_techcheck_cnid_fkey",
            "poll_techcheck", "cnid",
            "check_nsset", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "keyset_id_fkey",
            "keyset", "id",
            "object", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "nsset_id_fkey",
            "nsset", "id",
            "object", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "contact_id_fkey",
            "contact", "id",
            "object", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "domain_id_fkey",
            "domain", "id",
            "object", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "dnskey_keysetid_fkey",
            "dnskey", "keysetid",
            "keyset", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "domain_keyset_fkey",
            "domain", "keyset",
            "keyset", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "dsrecord_keysetid_fkey",
            "dsrecord", "keysetid",
            "keyset", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "keyset_contact_map_keysetid_fkey",
            "keyset_contact_map", "keysetid",
            "keyset", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "host_nssetid_fkey",
            "host", "nssetid",
            "nsset", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "nsset_contact_map_nssetid_fkey",
            "nsset_contact_map", "nssetid",
            "nsset", "id"
        ));
/*
        foreign_keys.push_back(boost::make_tuple(
            "dnssec_domainid_fkey",
            "dnssec", "domainid",
            "domain", "id"
        ));
*/
        foreign_keys.push_back(boost::make_tuple(
            "domain_contact_map_domainid_fkey",
            "domain_contact_map", "domainid",
            "domain", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "enumval_domainid_fkey",
            "enumval", "domainid",
            "domain", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "object_state_ohid_from_fkey",
            "object_state", "ohid_from",
            "object_history", "historyid"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "object_state_ohid_to_fkey",
            "object_state", "ohid_to",
            "object_history", "historyid"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "poll_eppaction_objid_fkey",
            "poll_eppaction", "objid",
            "object_history", "historyid"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "notify_letters_state_id_fkey",
            "notify_letters", "state_id",
            "object_state", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "notify_statechange_state_id_fkey",
            "notify_statechange", "state_id",
            "object_state", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "poll_statechange_stateid_fkey",
            "poll_statechange", "stateid",
            "object_state", "id"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "poll_eppaction_objid_fkey",
            "poll_eppaction", "objid",
            "object_history", "historyid"
        ));

        foreign_keys.push_back(boost::make_tuple(
            "domain_contact_map_contactid_fkey",
            "domain_contact_map", "contactid",
            "contact", "id"
        ));
    }

    clean(ctx);
    ctx.commit_transaction();

    std::vector<setup_test_status> test_statuses;
    test_statuses.push_back(setup_test_status(Fred::ContactTestStatus::ENQUEUED));
    test_statuses.push_back(setup_test_status(Fred::ContactTestStatus::RUNNING));
    test_statuses.push_back(setup_test_status(Fred::ContactTestStatus::OK));
    test_statuses.push_back(setup_test_status(Fred::ContactTestStatus::SKIPPED));
    test_statuses.push_back(setup_test_status(Fred::ContactTestStatus::FAIL));
    test_statuses.push_back(setup_test_status(Fred::ContactTestStatus::MANUAL));
    test_statuses.push_back(setup_test_status(Fred::ContactTestStatus::ERROR));

    std::vector<std::string> check_status_handles = Fred::ContactCheckStatus::get_all();
    std::vector<setup_check_status> check_statuses;
    for(std::vector<std::string>::const_iterator it = check_status_handles.begin();
        it != check_status_handles.end();
        ++it
    ) {
        check_statuses.push_back(setup_check_status(*it));
    }

    setup_empty_testsuite testsuite_automatic(Fred::TestsuiteHandle::AUTOMATIC);
    setup_empty_testsuite testsuite_manual(Fred::TestsuiteHandle::MANUAL);
}

void autoclean_contact_verification_db::set_cascading_fkeys(Fred::OperationContext& _ctx) {
    for(T_foreign_keys::const_iterator it = foreign_keys.begin(); it != foreign_keys.end(); ++it) {
        _ctx.get_conn()
            .exec(
                "ALTER TABLE " + it->get<1>() + " "
                "DROP CONSTRAINT " + it->get<0>()
        );

        _ctx.get_conn()
            .exec(
                "ALTER TABLE "+ it->get<1>() + " "
                "ADD CONSTRAINT " + it->get<0>() + " "
                "FOREIGN KEY ("+ it->get<2>() + ") "
                "REFERENCES "+ it->get<3>() + " (" + it->get<4>() + ") "
                "ON DELETE CASCADE;"
        );
    }
}

void autoclean_contact_verification_db::restore_fkeys(Fred::OperationContext& _ctx) {
    for(T_foreign_keys::const_iterator it = foreign_keys.begin(); it != foreign_keys.end(); ++it) {
        _ctx.get_conn()
            .exec(
                "ALTER TABLE " + it->get<1>() + " "
                "DROP CONSTRAINT " + it->get<0>()
        );

        _ctx.get_conn()
            .exec(
                "ALTER TABLE "+ it->get<1>() + " "
                "ADD CONSTRAINT " + it->get<0>() + " "
                "FOREIGN KEY ("+ it->get<2>() + ") "
                "REFERENCES "+ it->get<3>() + " (" + it->get<4>() + ") "
        );
    }
}

void autoclean_contact_verification_db::clean(Fred::OperationContext& _ctx) {
    _ctx.get_conn()
        .exec(
            "TRUNCATE "
            "   contact_test_result_history, "
            "   contact_test_result, "
            "   contact_check_history, "
            "   contact_check , "
            "   contact_testsuite_map, "
            "   enum_contact_test, "
            "   enum_contact_testsuite, "
            "   enum_contact_test_status, "
            "   enum_contact_check_status, "
            "   enum_contact_test_localization, "
            "   enum_contact_testsuite_localization, "
            "   enum_contact_test_status_localization, "
            "   enum_contact_check_status_localization, "
            "   contact_check_message_map,"
            "   contact_check_object_state_request_map, "
            "   contact_check_poll_message_map");

    set_cascading_fkeys(_ctx);

    std::vector<std::string> string_ids_to_preserve;
    for(std::vector<unsigned long long>::const_iterator it = object_ids_to_preserve_.begin();
        it != object_ids_to_preserve_.end();
        ++it
    ) {
        string_ids_to_preserve.push_back(
            boost::lexical_cast<std::string>(*it)
        );
    }


    _ctx.get_conn()
        .exec(
            "DELETE "
            "   FROM object_registry "
            "   WHERE id NOT IN ("
            + boost::join(
                string_ids_to_preserve,
                ", ")
            +")");

    restore_fkeys(_ctx);
}

autoclean_contact_verification_db::~autoclean_contact_verification_db() {
    Fred::OperationContext ctx;
    clean(ctx);
    ctx.commit_transaction();
}
