#ifndef TESTS_SETUP_UTILS_65446543512_
#define TESTS_SETUP_UTILS_65446543512_

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/create_test.h"
#include "fredlib/contact/verification/enum_testsuite_name.h"
#include "fredlib/contact/delete_contact.h"
#include "fredlib/contact/info_contact.h"
#include "util/random.h"

struct setup_get_registrar_handle
{
    std::string registrar_handle;

    setup_get_registrar_handle( ) {
        Fred::OperationContext ctx;

        registrar_handle = static_cast<std::string>(
            ctx.get_conn().exec("SELECT handle FROM registrar LIMIT 1;")[0][0] );

        ctx.commit_transaction();

        if(registrar_handle.empty()) {
            throw std::runtime_error("no registrar found");
        }
    }
};

struct setup_contact {
    std::string contact_handle;
    setup_get_registrar_handle registrar;
    Fred::InfoContactOutput data_;
    long long contact_id_;

    setup_contact() {
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
};

struct setup_nonexistent_contact_handle {
    std::string contact_handle;

    setup_nonexistent_contact_handle() {
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
};

struct setup_nonexistent_contact_id {
    long long contact_id_;

    setup_nonexistent_contact_id() {
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
};

struct setup_testdef {
    long testdef_id_;
    std::string testdef_name_;
    std::string testdef_description_;

    setup_testdef() {
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
};

struct setup_nonexistent_testdef_name {
    std::string testdef_name;

    setup_nonexistent_testdef_name() {
        Database::Result res;
        // prevent name collisions
        do {
            Fred::OperationContext ctx;

            testdef_name = "NONEX_TEST_" + RandomDataGenerator().xnumstring(15);
            res = ctx.get_conn().exec(
                "SELECT name FROM enum_contact_testsuite WHERE name='"+testdef_name+"';" );
        } while(res.size() != 0);
    }
};

struct setup_testdef_in_testsuite {
    setup_testdef_in_testsuite(const std::string& testdef_name, const std::string& testsuite_name) {
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
};

struct setup_testdef_in_testsuite_of_check {
    setup_testdef_in_testsuite_of_check(const std::string testdef_name, const std::string check_handle) {
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
};

struct setup_empty_testsuite {
    long testsuite_id;
    std::string testsuite_name;
    std::string testsuite_description;

    setup_empty_testsuite() {
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

    setup_empty_testsuite(const std::string& _testsuite_name)
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

};

struct setup_testsuite : public setup_empty_testsuite {
    std::vector<setup_testdef> testdefs;
    setup_testsuite() {
        testdefs.push_back(setup_testdef());
        setup_testdef_in_testsuite(testdefs.front().testdef_name_, testsuite_name);
    }

};

struct setup_nonexistent_testsuite_name {
    std::string testsuite_name;

    setup_nonexistent_testsuite_name() {
        Database::Result res;
        // prevent name collisions
        do {
            Fred::OperationContext ctx;

            testsuite_name = "NONEX_TESTSUITE_" + RandomDataGenerator().xnumstring(15);
            res = ctx.get_conn().exec(
                "SELECT name FROM enum_contact_testsuite WHERE name='"+testsuite_name+"';" );
        } while(res.size() != 0);

    }
};

struct setup_logd_request_id {
    long long logd_request_id;

    setup_logd_request_id() {
        logd_request_id = RandomDataGenerator().xuint();
    }
};

struct setup_check_status {
    std::string status_name_;

    setup_check_status() {
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
};

struct setup_test_status {
    std::string status_name_;

    setup_test_status() {
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
};

struct setup_error_msg {
    std::string error_msg;

    setup_error_msg() {
        error_msg = "ERROR_MSG_" + RandomDataGenerator().xnumstring(20);
    }
};

struct setup_check {
    std::string check_handle_;
    Optional<long long> logd_request_;
    setup_contact contact_;

    setup_check(const std::string& _testsuite_name, Optional<long long> _logd_request = Optional<long long>())
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
};

struct setup_nonexistent_check_handle {
    std::string check_handle;

    setup_nonexistent_check_handle() {
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
};

struct setup_nonexistent_check_status_name {
    std::string status_name_;

    setup_nonexistent_check_status_name() {
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
};

struct setup_test {
    std::string testdef_name_;
    Optional<long long> logd_request_;

    setup_test(
        const std::string& _check_handle,
        const std::string& _testdef_name,
        Optional<long long> _logd_request = Optional<long long>()
    ) :
        testdef_name_(_testdef_name),
        logd_request_(_logd_request)
    {
        Fred::CreateContactTest create_test(_check_handle, testdef_name_, logd_request_);

        Fred::OperationContext ctx;
        create_test.exec(ctx);
        ctx.commit_transaction();
    }

};

struct setup_nonexistent_test_status_name {
    std::string status_name_;

    setup_nonexistent_test_status_name() {
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
};

struct autoclean_contact_verification_db {
    std::vector<std::string> handles_to_preserve_;

    autoclean_contact_verification_db() {
        Fred::OperationContext ctx;

        Database::Result pre_existing_res = ctx.get_conn().exec(
            "SELECT o_r.name AS contact_handle_ "
            "   FROM object_registry AS o_r "
            "       JOIN contact USING(id)");

        for(Database::Result::Iterator it = pre_existing_res.begin(); it != pre_existing_res.end(); ++it) {
            handles_to_preserve_.push_back( static_cast<std::string>( (*it)["contact_handle_"] ) );
        }

        clean();
    }

    void clean() {
        Fred::OperationContext ctx;

        Database::Result to_delete_res = ctx.get_conn().exec(
            "SELECT o_r.name AS contact_handle_ "
            "   FROM object_registry AS o_r "
            "       JOIN contact USING(id)"
            "   WHERE o_r.name "
            "       NOT IN ('" + boost::algorithm::join(handles_to_preserve_, "', '")+ "'); ");

        for(Database::Result::Iterator it = to_delete_res.begin(); it != to_delete_res.end(); ++it) {
            Fred::DeleteContact(static_cast<std::string>( (*it)["contact_handle_"] )).exec(ctx);
        }

        ctx.get_conn().exec("DELETE FROM contact_test_result_history;");
        ctx.get_conn().exec("DELETE FROM contact_check_history;");
        ctx.get_conn().exec("DELETE FROM contact_test_result;");
        ctx.get_conn().exec("DELETE FROM contact_check;");
        ctx.get_conn().exec("DELETE FROM contact_testsuite_map;");
        ctx.get_conn().exec("DELETE FROM enum_contact_test;");
        ctx.get_conn().exec("DELETE FROM enum_contact_testsuite "
                            "   WHERE name != '"+ Fred::TestsuiteName::AUTOMATIC+"' "
                            "   AND name != '"+ Fred::TestsuiteName::MANUAL+"';");
        ctx.get_conn().exec("DELETE FROM enum_contact_test_status WHERE id > 7;");
        ctx.get_conn().exec("DELETE FROM enum_contact_check_status WHERE id > 6;");

        ctx.commit_transaction();
    }

    ~autoclean_contact_verification_db() {
        clean();
    }
};

#endif
