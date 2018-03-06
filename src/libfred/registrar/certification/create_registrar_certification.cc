#include "create_registrar_certification.h"
#include "exceptions.h"

//#include "src/fredlib/db_settings.h"
#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {

unsigned long long CreateRegistrarCertification::exec(OperationContext& ctx)
{
    try
    {
        if (m_valid_from > m_valid_until)
        {
            throw WrongIntervalOrder();
        }

        Database::Result cert_in_past = ctx.get_conn().exec_params(
                "SELECT now()::date > $1::date",
                Database::query_param_list(m_valid_until));
        if (cert_in_past[0][0])
        {
            throw CertificationInPast();
        }

        ctx.get_conn().exec("LOCK TABLE registrar_certification IN ACCESS EXCLUSIVE MODE");
        Database::Result range_overlap = ctx.get_conn().exec_params(
                "SELECT * FROM registrar_certification "
                "WHERE registrar_id = $3::bigint AND "
                "(valid_from <= $1::date "
                "AND $1::date <= valid_until "
                "OR valid_from <= $2::date "
                "AND $2::date <= valid_until) ",
                Database::query_param_list(m_valid_from)(m_valid_until)(m_registrar_id));
        if (range_overlap.size() > 0)
        {
            throw OverlappingRange();
        }

        if (m_classification < 0 || m_classification > 5)
        {
            throw ScoreOutOfRange();
        }

        Database::ParamQuery query =
            Database::ParamQuery("INSERT INTO registrar_certification (")
            ("registrar_id, valid_from, valid_until, classification, eval_file_id)")
            (" VALUES (")
            .param_bigint(m_registrar_id)(", ")
            .param_date(m_valid_from)(", ")
            .param_date(m_valid_until)(", ")
            .param(m_classification, "integer")(", ")
            .param_bigint(m_eval_file_id)
            (") RETURNING id");

        return ctx.get_conn().exec_params(query)[0][0];
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

} // namespace Fred
