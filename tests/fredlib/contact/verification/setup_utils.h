#ifndef TESTS_SETUP_UTILS_65446543512_
#define TESTS_SETUP_UTILS_65446543512_

#include <string>
#include <vector>

#include "tests/setup/fixtures_utils.h"

struct setup_registrar
{
    Fred::InfoRegistrarData data;

    setup_registrar();
};

struct setup_contact {
    Fred::InfoContactData data;

    setup_contact();
};

struct setup_nonexistent_contact_handle {
    std::string contact_handle;

    setup_nonexistent_contact_handle();
};

struct setup_nonexistent_contact_id {
    unsigned long long contact_id_;

    setup_nonexistent_contact_id();
};

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
    setup_contact contact_;

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

class autoclean_contact_verification_db {
    std::vector<unsigned long long> object_ids_to_preserve_;

    public:
        typedef
            std::vector<
                boost::tuple<
                    std::string,
                    std::string,
                    std::string,
                    std::string,
                    std::string
                >
            > T_foreign_keys;

    private:
        static T_foreign_keys foreign_keys;

    private:
        void set_cascading_fkeys(Fred::OperationContext& _ctx);

        void restore_fkeys(Fred::OperationContext& _ctx);
    public:
        autoclean_contact_verification_db();

        void clean(Fred::OperationContext& _ctx);

        ~autoclean_contact_verification_db();
};

#endif
