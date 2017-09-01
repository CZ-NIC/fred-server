#include "tests/setup/fixtures_utils.h"
#include <cmath>

namespace Test {

    unsigned long long generate_random_bigserial() {
        return std::abs(RandomDataGenerator().xint());
    }

    std::string generate_random_handle() {
        return RandomDataGenerator().xnumstring(20);
    }

    unsigned long long get_nonexistent_object_id(Fred::OperationContext& ctx) {
        return get_nonexistent_value(ctx, "object_registry", "id", "bigint", generate_random_bigserial);
    }

    unsigned long long get_nonexistent_object_historyid(Fred::OperationContext& ctx) {
        return get_nonexistent_value(ctx, "object_history", "historyid", "bigint", generate_random_bigserial);
    }

    std::string get_nonexistent_object_handle(Fred::OperationContext& ctx) {
        return get_nonexistent_value(ctx, "object_registry", "name", "text", generate_random_handle);
    }

    unsigned long long get_nonexistent_message_id(Fred::OperationContext& ctx) {
        return get_nonexistent_value(ctx, "message", "id", "bigint", generate_random_bigserial);
    }

    unsigned long long get_nonexistent_registrar_id(Fred::OperationContext& ctx) {
        return get_nonexistent_value(ctx, "registrar", "id", "bigint", generate_random_bigserial);
    }
    unsigned long long  get_nonexistent_zone_id(Fred::OperationContext& ctx) {
        return get_nonexistent_value(ctx, "zone", "id", "bigint", generate_random_bigserial);
    }

    unsigned long long get_cz_zone_id(Fred::OperationContext& ctx) {
        return
            ctx.get_conn().exec(
                "SELECT id "
                    "FROM zone "
                    "WHERE fqdn='cz'"
            )[0]["id"];
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
