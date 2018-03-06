#include "update_registrar_certification.h"
#include "exceptions.h"

//#include "src/fredlib/db_settings.h"
#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {

void UpdateRegistrarCertification::exec(OperationContext& ctx)
{
    try
    {
        Database::ParamQuery query = Database::ParamQuery("UPDATE registrar_certification SET ");
        if (m_valid_until_set)
        {
            Database::Result cert_in_past = ctx.get_conn().exec_params(
                    "select now()::date > $1::date",
                    Database::query_param_list(m_valid_until));
            if (cert_in_past[0][0])
            {
                throw CertificationInPast();
            }

            Database::Result from_until = ctx.get_conn().exec_params(
                    "select valid_from, valid_until from registrar_certification "
                    "where id = $1::bigint for update",
                    Database::query_param_list(m_certification_id));
            boost::gregorian::date old_from = from_until[0][0],
                old_until = from_until[0][1];
            if (old_until < m_valid_until)
            {
                throw CertificationExtension();
            }
            if (old_from > m_valid_until)
            {
                throw WrongIntervalOrder();
            }

            query = query("valid_until = ")
                .param_date(m_valid_until);
        }
        else
        {
            if (m_classification < 0 || m_classification > 5)
            {
                throw ScoreOutOfRange();
            }
            query = query("classification = ")
                .param(m_classification, "integer")
                (", eval_file_id = ")
                .param_bigint(m_eval_file_id);
        }
        query = query(" where id = ")
            .param_bigint(m_certification_id);

        ctx.get_conn().exec_params(query);
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
