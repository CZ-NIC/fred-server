/**
 *  @file
 *  test fredlib utils
 */

#ifndef TESTS_FREDLIB_UTIL_69498451224
#define TESTS_FREDLIB_UTIL_69498451224

#include "tests/setup/fixtures.h"
#include "tests/setup/fixtures_utils.h"

inline bool check_std_exception(const std::exception &e)
{
    return e.what()[0] != '\0';
}

namespace Test
{

struct autocommitting_context : virtual Fixture::instantiate_db_template {
    Fred::OperationContextCreator ctx;

    virtual ~autocommitting_context() {
        ctx.commit_transaction();
    }
};

struct has_registrar : Test::autocommitting_context {
    Fred::InfoRegistrarData registrar;

    has_registrar() {
        const std::string reg_handle = "REGISTRAR1";
        Fred::CreateRegistrar(reg_handle).exec(ctx);
        registrar = Fred::InfoRegistrarByHandle(reg_handle).exec(ctx).info_registrar_data;
    }
};

struct has_contact : has_registrar {
    Fred::InfoContactData contact;

    has_contact() {
        const std::string contact_handle = "CONTACT1";
        Fred::CreateContact(contact_handle, registrar.handle).exec(ctx);
        contact = Fred::InfoContactByHandle(contact_handle).exec(ctx).info_contact_data;
    }
};

struct has_contact_and_a_different_registrar : has_contact {
    Fred::InfoRegistrarData the_different_registrar;

    has_contact_and_a_different_registrar() {
        const std::string diff_reg_handle = "REGISTRAR2";
        Fred::CreateRegistrar(diff_reg_handle).exec(ctx);
        the_different_registrar = Fred::InfoRegistrarByHandle(diff_reg_handle).exec(ctx).info_registrar_data;
    }
};

}//namespace Test

#endif // #include guard end
