#include "tests/fredlib/contact/verification/setup_utils.h"

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/enum_testsuite_name.h"
#include "fredlib/contact/verification/enum_check_status.h"
#include "fredlib/contact/verification/enum_test_status.h"
#include "fredlib/contact/create_contact.h"
#include "fredlib/contact/delete_contact.h"
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
    // prevent name collisions
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
    data_ = Fred::InfoContact(contact_handle, registrar.registrar_handle).exec(ctx_check);

    contact_id_ = static_cast<long long>(
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
    // prevent name collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            testdef_name_ = "TEST_" + RandomDataGenerator().xnumstring(15);
            testdef_description_ = testdef_name_ + "_DESCRIPTION";
            testdef_id_ = static_cast<long>(
                ctx.get_conn().exec(
                    "INSERT INTO enum_contact_test "
                    "   (name, description) "
                    "   VALUES ('"+testdef_name_+"', '"+testdef_description_+"') "
                    "   RETURNING id;"
                )[0][0]);

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }
}

setup_nonexistent_testdef_name::setup_nonexistent_testdef_name() {
    Database::Result res;
    // prevent name collisions
    do {
        Fred::OperationContext ctx;

        testdef_name = "NONEX_TEST_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT name FROM enum_contact_testsuite WHERE name='"+testdef_name+"';" );
    } while(res.size() != 0);
}

setup_testdef_in_testsuite::setup_testdef_in_testsuite(const std::string& testdef_name, const std::string& testsuite_name) {
    Fred::OperationContext ctx;

    Database::Result res = ctx.get_conn().exec(
        "INSERT INTO contact_testsuite_map "
        "   (enum_contact_test_id, enum_contact_testsuite_id) "
        "   VALUES ("
        "       (SELECT id FROM enum_contact_test WHERE name='"+testdef_name+"' ), "
        "       (SELECT id FROM enum_contact_testsuite WHERE name='"+testsuite_name+"') "
        "   ) "
        "   RETURNING enum_contact_test_id;");

    ctx.commit_transaction();

    if(res.size() != 1) {
        throw std::runtime_error("inserting testdef to testsuite");
    }

}

setup_testdef_in_testsuite_of_check::setup_testdef_in_testsuite_of_check(const std::string testdef_name, const std::string check_handle) {
    Fred::OperationContext ctx;

    Database::Result res =
        ctx.get_conn().exec(
            "INSERT INTO contact_testsuite_map "
            "   (enum_contact_test_id, enum_contact_testsuite_id) "
            "   VALUES ("
            "       (SELECT id FROM enum_contact_test WHERE name='"+testdef_name+"' ), "
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
    // prevent name collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            testsuite_name = "TESTSUITE_" + RandomDataGenerator().xnumstring(15) ;
            testsuite_description = testsuite_name + "_DESCRIPTION";
            testsuite_id = static_cast<long>(
                ctx.get_conn().exec(
                    "INSERT INTO enum_contact_testsuite "
                    "   (name, description)"
                    "   VALUES ('"+testsuite_name+"', '"+testsuite_description+"')"
                    "   RETURNING id;"
                )[0][0]);

            ctx.commit_transaction();
        } catch (Database::ResultFailed& ) {
            continue;
        }
        break;
    }
}

setup_empty_testsuite::setup_empty_testsuite(const std::string& _testsuite_name)
    : testsuite_name(_testsuite_name)
{
    Fred::OperationContext ctx;

    testsuite_description = testsuite_name + "_DESCRIPTION";
    testsuite_id = static_cast<long>(
        ctx.get_conn().exec(
            "INSERT INTO enum_contact_testsuite "
            "   (name, description)"
            "   VALUES ('"+testsuite_name+"', '"+testsuite_description+"')"
            "   RETURNING id;"
        )[0][0]);

    ctx.commit_transaction();
}

setup_testsuite::setup_testsuite() {
    testdefs.push_back(setup_testdef());
    setup_testdef_in_testsuite(testdefs.front().testdef_name_, testsuite_name);
}

setup_nonexistent_testsuite_name::setup_nonexistent_testsuite_name() {
    Database::Result res;
    // prevent name collisions
    do {
        Fred::OperationContext ctx;

        testsuite_name = "NONEX_TESTSUITE_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT name FROM enum_contact_testsuite WHERE name='"+testsuite_name+"';" );
    } while(res.size() != 0);

}

setup_logd_request_id::setup_logd_request_id() {
    logd_request_id = RandomDataGenerator().xuint();
}

setup_check_status::setup_check_status() {
    Database::Result res;
    // prevent name collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            status_name_ = "STATUS_" + RandomDataGenerator().xnumstring(15);
            res = ctx.get_conn().exec(
                "INSERT "
                "   INTO enum_contact_check_status "
                "   (id, name, description ) "
                "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '"+status_name_+"', '"+status_name_+"_desc') "
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

setup_check_status::setup_check_status(const std::string& _name)
    : status_name_(_name)
{
    Database::Result res;
    Fred::OperationContext ctx;

    res = ctx.get_conn().exec(
        "INSERT "
        "   INTO enum_contact_check_status "
        "   (id, name, description ) "
        "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '"+status_name_+"', '"+status_name_+"_desc') "
        "   RETURNING id;" );

    ctx.commit_transaction();

    if(res.size()!=1) {
        throw std::runtime_error("creating check status failed");
    }
}

setup_test_status::setup_test_status() {
    Database::Result res;
    // prevent name collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            status_name_ = "STATUS_" + RandomDataGenerator().xnumstring(15);
            res = ctx.get_conn().exec(
                "INSERT "
                "   INTO enum_contact_test_status "
                "   (id, name, description ) "
                "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '"+status_name_+"', '"+status_name_+"_desc') "
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

setup_test_status::setup_test_status(const std::string& _name)
    : status_name_(_name)
{
    Database::Result res;

    Fred::OperationContext ctx;

    res = ctx.get_conn().exec(
        "INSERT "
        "   INTO enum_contact_test_status "
        "   (id, name, description ) "
        "   VALUES (" + RandomDataGenerator().xnumstring(9) + ", '" + status_name_ + "', '" + status_name_ + "_desc') "
        "   RETURNING id;" );

    ctx.commit_transaction();

    if(res.size()!=1) {
        throw std::runtime_error("creating test status failed");
    }
}

setup_error_msg::setup_error_msg() {
    error_msg = "ERROR_MSG_" + RandomDataGenerator().xnumstring(20);
}

setup_check::setup_check(const std::string& _testsuite_name, Optional<long long> _logd_request)
    : logd_request_(_logd_request)
{
    // check
    Fred::CreateContactCheck create_check(
        contact_.contact_id_,
        _testsuite_name,
        logd_request_
    );

    Fred::OperationContext ctx;
    check_handle_ = create_check.exec(ctx);
    ctx.commit_transaction();
}

setup_nonexistent_check_handle::setup_nonexistent_check_handle() {
    struct BOOST { struct UUIDS { struct RANDOM_GENERATOR {
        static std::string generate() {
            srand(time(NULL));
            std::vector<unsigned char> bytes;

            // generate random 128bits = 16 bytes
            for (int i = 0; i < 16; ++i) {
                bytes.push_back( RandomDataGenerator().xletter()%256 );
            }
            /* some specific uuid rules
             * http://www.cryptosys.net/pki/Uuid.c.html
             */
            bytes.at(6) = static_cast<char>(0x40 | (bytes.at(6) & 0xf));
            bytes.at(8) = static_cast<char>(0x80 | (bytes.at(8) & 0x3f));

            // buffer for hex representation of one byte + terminating zero
            char hex_rep[3];

            // converting raw bytes to hex string representation
            std::string result;
            for (std::vector<unsigned char>::iterator it = bytes.begin(); it != bytes.end(); ++it) {
                sprintf(hex_rep,"%02x",*it);
                // conversion target is hhhh - so in case it gets wrong just cut off the tail
                hex_rep[2] = 0;
                result += hex_rep;
            }

            // hyphens for canonical form
            result.insert(8, "-");
            result.insert(13, "-");
            result.insert(18, "-");
            result.insert(23, "-");

            return result;
        }
    }; }; };

    /* end of temporary ugliness - please cut and replace between ASAP*/
    Fred::OperationContext ctx;

    Database::Result res;
    do {
        check_handle = boost::lexical_cast<std::string>(BOOST::UUIDS::RANDOM_GENERATOR::generate());
        res = ctx.get_conn().exec(
            "SELECT handle "
            "   FROM contact_check "
            "   WHERE handle='"+check_handle+"';"
        );
    } while(res.size() != 0);
}

setup_nonexistent_check_status_name::setup_nonexistent_check_status_name() {
    Fred::OperationContext ctx;
    // prevent name collisions
    Database::Result res;
    do {
        status_name_ = "STATUS_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT name "
            "   FROM enum_contact_check_status "
            "   WHERE name='"+status_name_+"';" );
    } while(res.size() != 0);
}

setup_test::setup_test(
    const std::string& _check_handle,
    const std::string& _testdef_name,
    Optional<long long> _logd_request
) :
    testdef_name_(_testdef_name),
    logd_request_(_logd_request)
{
    Fred::CreateContactTest create_test(_check_handle, testdef_name_, logd_request_);

    Fred::OperationContext ctx;
    create_test.exec(ctx);
    ctx.commit_transaction();
}

setup_nonexistent_test_status_name::setup_nonexistent_test_status_name() {
    Database::Result res;
    // prevent name collisions
    do {
        Fred::OperationContext ctx;
        status_name_ = "STATUS_" + RandomDataGenerator().xnumstring(15);
        res = ctx.get_conn().exec(
            "SELECT name "
            "   FROM enum_contact_test_status "
            "   WHERE name='"+status_name_+"';" );
    } while(res.size() != 0);
}

autoclean_contact_verification_db::T_foreign_keys autoclean_contact_verification_db::foreign_keys;

autoclean_contact_verification_db::autoclean_contact_verification_db() {
    Fred::OperationContext ctx;

    Database::Result pre_existing_res = ctx.get_conn().exec(
        "SELECT o_r.name AS contact_handle_ "
        "   FROM object_registry AS o_r "
        "       JOIN contact USING(id)");

    for(Database::Result::Iterator it = pre_existing_res.begin(); it != pre_existing_res.end(); ++it) {
        handles_to_preserve_.push_back( static_cast<std::string>( (*it)["contact_handle_"] ) );
    }

    if(foreign_keys.empty()) {
        foreign_keys.push_back(
            boost::make_tuple(
                "object_id_fkey",
                "object", "id",
                "object_registry", "id"
            )
        );

        foreign_keys.push_back(
            boost::make_tuple(
                "object_history_id_fkey",
                "object_history", "id",
                "object_registry", "id"
            )
        );

        foreign_keys.push_back(
            boost::make_tuple(
                "contact_history_id_fkey",
                "contact_history", "id",
                "object_registry", "id"
            )
        );

        foreign_keys.push_back(
            boost::make_tuple(
                "domain_contact_map_history_contactid_fkey",
                "domain_contact_map_history", "domainid",
                "object_registry", "id"
            )
        );

        foreign_keys.push_back(
            boost::make_tuple(
                "nsset_contact_map_history_contactid_fkey",
                "nsset_contact_map_history", "nssetid",
                "object_registry", "id"
            )
        );

        foreign_keys.push_back(
            boost::make_tuple(
                "keyset_contact_map_history_contactid_fkey",
                "keyset_contact_map_history", "keysetid",
                "object_registry", "id"
            )
        );

        foreign_keys.push_back(
            boost::make_tuple(
                "object_state_object_id_fkey",
                "object_state", "object_id",
                "object_registry", "id"
            )
        );

        foreign_keys.push_back(
            boost::make_tuple(
                "contact_id_fkey",
                "contact", "id",
                "object", "id"
            )
        );
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

    std::vector<setup_check_status> check_statuses;
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::ENQUEUED));
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::RUNNING));
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::AUTO_OK));
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::AUTO_FAIL));
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::AUTO_TO_BE_DECIDED));
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::OK));
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::FAIL));
    check_statuses.push_back(setup_check_status(Fred::ContactCheckStatus::INVALIDATED));

    setup_empty_testsuite testsuite(Fred::TestsuiteName::AUTOMATIC);
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
            "   contact_check_message_map,"
            "   contact_check_object_state_map, "
            "   contact_check_poll_message_map");

    set_cascading_fkeys(_ctx);

    _ctx.get_conn()
        .exec(
            "DELETE "
            "   FROM object_registry "
            "   WHERE type = 1 "
            "   AND name NOT IN ('" + boost::join(handles_to_preserve_, "', '")+"')");

    restore_fkeys(_ctx);
}

autoclean_contact_verification_db::~autoclean_contact_verification_db() {
    Fred::OperationContext ctx;
    clean(ctx);
    ctx.commit_transaction();
}
