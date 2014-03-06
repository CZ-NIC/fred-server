#include "setup_utils.h"
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/case_conv.hpp>

#include "src/fredlib/contact/create_contact.h"
#include "src/fredlib/domain/create_domain.h"
#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/nsset/create_nsset.h"

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

            handle_ = "CONTACT_" + RandomDataGenerator().xnumstring(15);
            Fred::CreateContact(handle_, registrar_.registrar_handle)
                .exec(ctx);
            ctx.commit_transaction();

        } catch (Database::ResultFailed& ) {
            continue;
        } catch (Fred::InternalError& ) {
            continue;
        }
        break;
    }

    Fred::OperationContext ctx_check;
    data_ = Fred::InfoContactByHandle(handle_).exec(ctx_check);

    id_ = static_cast<unsigned long long>(
        ctx_check.get_conn().exec(
            "SELECT id "
            "   FROM contact "
            "   JOIN object_registry AS o_r USING(id) "
            "   WHERE o_r.name='" + handle_ + "' "
        )[0]["id"]);
}

setup_domain::setup_domain() {
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            fqdn_ = "DOMAIN" + RandomDataGenerator().xnumstring(15) + ".CZ";
            Fred::CreateDomain(fqdn_, registrar_.registrar_handle, owner_.handle_)
                .exec(ctx);
            ctx.commit_transaction();

        } catch (Database::ResultFailed& ) {
            continue;
        } catch (Fred::InternalError& ) {
            continue;
        }
        break;
    }

    Fred::OperationContext ctx_check;
    data_ = Fred::InfoDomainByHandle(fqdn_).exec(ctx_check);

    id_ = static_cast<unsigned long long>(
        ctx_check.get_conn().exec(
            "SELECT id "
            "   FROM domain "
            "   JOIN object_registry AS o_r USING(id) "
            "   WHERE o_r.name='" + boost::algorithm::to_lower_copy(fqdn_) + "' "
        )[0]["id"]);
}

setup_keyset::setup_keyset() {
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            handle_ = "KEYSET_" + RandomDataGenerator().xnumstring(15);
            Fred::CreateKeyset(handle_, registrar_.registrar_handle)
                .exec(ctx);
            ctx.commit_transaction();

        } catch (Database::ResultFailed& ) {
            continue;
        } catch (Fred::InternalError& ) {
            continue;
        }
        break;
    }

    Fred::OperationContext ctx_check;
    data_ = Fred::InfoKeysetByHandle(handle_).exec(ctx_check);

    id_ = static_cast<unsigned long long>(
        ctx_check.get_conn().exec(
            "SELECT id "
            "   FROM keyset "
            "   JOIN object_registry AS o_r USING(id) "
            "   WHERE o_r.name='" + handle_ + "' "
        )[0]["id"]);
}

setup_nsset::setup_nsset() {
    // prevent handle collisions
    while(true) {
        try {
            Fred::OperationContext ctx;

            handle_ = "NSSET_" + RandomDataGenerator().xnumstring(15);
            Fred::CreateNsset(handle_, registrar_.registrar_handle)
                .exec(ctx);
            ctx.commit_transaction();

        } catch (Database::ResultFailed& ) {
            continue;
        } catch (Fred::InternalError& ) {
            continue;
        }
        break;
    }

    Fred::OperationContext ctx_check;
    data_ = Fred::InfoNssetByHandle(handle_).exec(ctx_check);

    id_ = static_cast<unsigned long long>(
        ctx_check.get_conn().exec(
            "SELECT id "
            "   FROM nsset "
            "   JOIN object_registry AS o_r USING(id) "
            "   WHERE o_r.name='" + handle_ + "' "
        )[0]["id"]);
}

setup_nonexistent_object_historyid::setup_nonexistent_object_historyid() {
    Database::Result res;
    // guarantee non-existence
    do {
        Fred::OperationContext ctx;
        history_id_ = Random::integer(0, 2147000000);
        res = ctx.get_conn().exec(
            "SELECT historyid "
            "   FROM object_history "
            "   WHERE historyid='" + boost::lexical_cast<std::string>(history_id_) + "'" );
    } while(res.size() != 0);
}
