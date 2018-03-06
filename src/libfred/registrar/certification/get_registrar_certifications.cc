#include "get_registrar_certifications.h"
#include "exceptions.h"
#include "registrar_certification_type.h"

//#include "src/fredlib/db_settings.h"
#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {

std::vector<RegistrarCertification> GetRegistrarCertifications::exec(OperationContext& ctx)
{
    try
    {
        std::vector<RegistrarCertification> result;

        Database::Result reg_exists = ctx.get_conn().exec_params(
                "SELECT id FROM registrar WHERE id = $1::integer",
                Database::query_param_list(m_registrar_id));
        if (reg_exists.size() != 1)
        {
            throw RegistrarNotFound();
        }

        Database::Result certifications = ctx.get_conn().exec_params(
                "SELECT id, valid_from, valid_until, classification, eval_file_id "
                "FROM registrar_certification WHERE registrar_id=$1::bigint "
                "ORDER BY valid_from DESC, id DESC",
                Database::query_param_list(m_registrar_id));
        result.reserve(certifications.size());
        for (Database::Result::Iterator it = certifications.begin(); it != certifications.end(); ++it)
        {
            Database::Row::Iterator col = (*it).begin();
            RegistrarCertification rc;
            rc.id = *col;
            rc.valid_from = *(++col);
            rc.valid_until = *(++col);
            rc.classification = *(++col);
            rc.eval_file_id = *(++col);
            result.push_back(rc);
        }
        return result;
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
