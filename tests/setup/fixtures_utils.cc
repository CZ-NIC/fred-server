#include "tests/setup/fixtures_utils.h"
#include <cmath>

namespace Test {
    unsigned long long get_nonexistent_object_id(Fred::OperationContext& ctx) {
        unsigned long long result;

        Database::Result check;
        // guarantee non-existence
        do {
            Fred::OperationContext ctx;
            // warning: type of column id in postgres is "integer", implicitly assuming POSITIVE
            result = std::abs(RandomDataGenerator().xint());
            check = ctx.get_conn().exec_params(
                "SELECT id "
                    "FROM object_registry "
                    "WHERE id=$1::bigint ",
                    Database::query_param_list(result) );
        } while(check.size() != 0);

        return result;
    }

    unsigned long long get_nonexistent_object_historyid(Fred::OperationContext& ctx) {
        unsigned long long result;

        Database::Result check;
        // guarantee non-existence
        do {
            Fred::OperationContext ctx;
            // warning: type of column id in postgres is "integer", implicitly assuming POSITIVE
            result = std::abs(RandomDataGenerator().xint());
            check = ctx.get_conn().exec_params(
                "SELECT historyid "
                    "FROM object_history "
                    "WHERE id=$1::bigint ",
                    Database::query_param_list(result) );
        } while(check.size() != 0);

        return result;
    }

    std::string get_nonexistent_object_handle(Fred::OperationContext& ctx) {
        std::string result;

        Database::Result check;
        // guarantee non-existence
        do {
            Fred::OperationContext ctx;
            result = "OBJECT_" + RandomDataGenerator().xnumstring(20);
            check = ctx.get_conn().exec_params(
                "SELECT name "
                    "FROM object_registry "
                    "WHERE name=$1::varchar ",
                    Database::query_param_list(result) );
        } while(check.size() != 0);

        return result;
    }

    unsigned long long get_nonexistent_registrar_id(Fred::OperationContext& ctx) {
        unsigned long long result;

        Database::Result check;
        // guarantee non-existence
        do {
            Fred::OperationContext ctx;
            result = RandomDataGenerator().xint();
            check = ctx.get_conn().exec_params(
                "SELECT id "
                    "FROM registrar "
                    "WHERE id=$1::bigint ",
                    Database::query_param_list(result) );
        } while(check.size() != 0);

        return result;
    }
    unsigned long long  get_nonexistent_zone_id(Fred::OperationContext& ctx) {
        unsigned long long result;

        Database::Result check;
        // guarantee non-existence
        do {
            Fred::OperationContext ctx;
            result = RandomDataGenerator().xint();
            check = ctx.get_conn().exec_params(
                "SELECT id "
                    "FROM zone "
                    "WHERE id=$1::bigint ",
                    Database::query_param_list(result) );
        } while(check.size() != 0);

        return result;
    }
    add_admin_contacts_to_domain::add_admin_contacts_to_domain(
        const std::string& _domain_handle,
        const std::string& _registrar_handle
    ) :
        domain_handle_(_domain_handle),
        registrar_handle_(_registrar_handle)
    { }

    std::pair<
        std::string,
        std::vector<std::string>
    >
    add_admin_contacts_to_domain::exec(Fred::OperationContext& ctx) {
        Fred::UpdateDomain update_domain(domain_handle_, registrar_handle_);
        BOOST_FOREACH(const std::string& c, values_) {
            update_domain.add_admin_contact(c);
        }

        update_domain.exec(ctx);

        return std::make_pair(domain_handle_, values_);
    }



    add_admin_contact_to_domains::add_admin_contact_to_domains(
        const std::string& _contact_handle,
        const std::string& _registrar_handle
    ) :
        contact_handle_(_contact_handle),
        registrar_handle_(_registrar_handle)
    { }

    std::pair<
        std::string,
        std::vector<std::string>
    >
    add_admin_contact_to_domains::exec(Fred::OperationContext& ctx) {
        BOOST_FOREACH(const std::string& d, values_) {
            Fred::UpdateDomain(d, registrar_handle_).add_admin_contact(contact_handle_).exec(ctx);
        }

        return std::make_pair(contact_handle_, values_);
    }
};
