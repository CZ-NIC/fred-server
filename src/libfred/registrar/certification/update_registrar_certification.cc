#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/certification/exceptions.hh"
#include "src/libfred/registrar/certification/update_registrar_certification.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {
namespace Registrar {

void UpdateRegistrarCertification::exec(OperationContext& _ctx)
{
    try
    {
        Database::ParamQuery query = Database::ParamQuery("UPDATE registrar_certification SET ");
        if (valid_until_set_)
        {
            const Database::Result cert_in_past = _ctx.get_conn().exec_params(
                    "SELECT now()::date > $1::date",
                    Database::query_param_list(valid_until_));
            if (cert_in_past[0][0])
            {
                throw CertificationInPast();
            }

            const Database::Result from_until = _ctx.get_conn().exec_params(
                    "SELECT valid_from, valid_until FROM registrar_certification "
                    "WHERE id = $1::bigint FOR UPDATE",
                    Database::query_param_list(certification_id_));
            const boost::gregorian::date old_from = from_until[0][0];
            const boost::gregorian::date old_until = from_until[0][1];
            if (old_until < valid_until_)
            {
                throw CertificationExtension();
            }
            if (old_from > valid_until_)
            {
                throw WrongIntervalOrder();
            }

            query = query("valid_until = ")
                .param_date(valid_until_);
        }
        else
        {
            if (classification_ < 0 || classification_ > 5)
            {
                throw ScoreOutOfRange();
            }
            query = query("classification = ")
                .param(classification_, "integer")
                (", eval_file_id = ")
                .param_bigint(eval_file_id_);
        }
        query = query(" WHERE id = ")
            .param_bigint(certification_id_);

        _ctx.get_conn().exec_params(query);
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Unknown error");
        throw;
    }
}

} // namespace Registrar
} // namespace Fred
