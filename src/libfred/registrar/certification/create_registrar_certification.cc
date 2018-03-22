#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/certification/create_registrar_certification.hh"
#include "src/libfred/registrar/certification/exceptions.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {
namespace Registrar {

unsigned long long CreateRegistrarCertification::exec(OperationContext& _ctx)
{
    try
    {
        if (valid_from_ > valid_until_)
        {
            throw WrongIntervalOrder();
        }

        const Database::Result cert_in_past = _ctx.get_conn().exec_params(
                "SELECT now()::date > $1::date",
                Database::query_param_list(valid_until_));
        if (cert_in_past[0][0])
        {
            throw CertificationInPast();
        }

        _ctx.get_conn().exec("LOCK TABLE registrar_certification IN ACCESS EXCLUSIVE MODE");
        const Database::Result range_overlap = _ctx.get_conn().exec_params(
                "SELECT * FROM registrar_certification "
                "WHERE registrar_id = $3::bigint AND "
                "(valid_from <= $1::date "
                "AND $1::date <= valid_until "
                "OR valid_from <= $2::date "
                "AND $2::date <= valid_until) ",
                Database::query_param_list(valid_from_)(valid_until_)(registrar_id_));
        if (range_overlap.size() > 0)
        {
            throw OverlappingRange();
        }

        if (classification_ < 0 || classification_ > 5)
        {
            throw ScoreOutOfRange();
        }

        const Database::Result created_certification = _ctx.get_conn().exec_params(
                "INSERT INTO registrar_certification ("
                "registrar_id, valid_from, valid_until, classification, eval_file_id)"
                " VALUES ( "
                "$1::bigint, $2::date, $3::date, $4::integer, $5::bigint)"
                "RETURNING id",
                Database::query_param_list(registrar_id_)(valid_from_)(valid_until_)(classification_)(eval_file_id_));

        return created_certification[0][0];
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
