#include "src/fredlib/object/transfer_object.h"

#include "src/fredlib/object/transfer_object_exception.h"
#include "src/fredlib/registrar/info_registrar.h"

namespace Fred
{
    void transfer_object(
        Fred::OperationContext& _ctx,
        const unsigned long long _object_id,
        const std::string& _new_registrar_handle,
        const GeneratedAuthInfoPassword& _new_authinfopw
    ) {

        unsigned long long registrar_id = 0;

        try {
            registrar_id = Fred::InfoRegistrarByHandle(_new_registrar_handle).exec(_ctx).info_registrar_data.id;

        } catch(const Fred::InfoRegistrarByHandle::Exception& e) {
            if(e.is_set_unknown_registrar_handle()) {
                throw ExceptionUnknownRegistrar();
            }
            throw;
        }

        {
            const Database::Result sponsoring_registrar_res = _ctx.get_conn().exec_params(
                "SELECT clid FROM object WHERE id = $1::integer FOR UPDATE",
                Database::query_param_list(_object_id)
            );

            if(sponsoring_registrar_res.size() < 1) {
                throw ExceptionUnknownObjectId();

            }
            if(sponsoring_registrar_res.size() > 1) {
                throw std::runtime_error("something is really broken - nonunique record in object_registry");
            }

            if(static_cast<unsigned long long>(sponsoring_registrar_res[0]["clid"]) == registrar_id) {
                throw ExceptionNewRegistrarIsAlreadySponsoring();
            }
        }

        {
            const Database::Result existence_check_res = _ctx.get_conn().exec_params(
                "SELECT 1 FROM object_registry WHERE id = $1::integer AND erdate IS NULL FOR UPDATE",
                Database::query_param_list(_object_id)
            );

            if(existence_check_res.size() < 1) {
                throw ExceptionUnknownObjectId();
            } else if(existence_check_res.size() > 1) {
                throw std::runtime_error("something is really broken - nonunique record in object_registry");
            }
        }

        const Database::Result transfer_res = _ctx.get_conn().exec_params(
            "UPDATE object "
            "SET "
                "trdate = now(), "
                "clid = $1::integer, "
                "authinfopw = $2::text "
            "WHERE id = $3::integer "
            "RETURNING 1",
            Database::query_param_list
                (registrar_id)
                (_new_authinfopw.password_)
                (_object_id)
        );

        if(transfer_res.size() != 1) {
            throw std::runtime_error("transfer failed");
        }

    }
}
