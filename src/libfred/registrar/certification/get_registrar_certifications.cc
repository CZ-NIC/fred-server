#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/certification/exceptions.hh"
#include "src/libfred/registrar/certification/get_registrar_certifications.hh"
#include "src/libfred/registrar/certification/registrar_certification_type.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {
namespace Registrar {

std::vector<RegistrarCertification> GetRegistrarCertifications::exec(OperationContext& _ctx)
{
    try
    {
        std::vector<RegistrarCertification> result;

        const Database::Result reg_exists = _ctx.get_conn().exec_params(
                "SELECT id FROM registrar WHERE id = $1::integer",
                Database::query_param_list(registrar_id_));
        if (reg_exists.size() != 1)
        {
            throw RegistrarNotFound();
        }

        const Database::Result certifications = _ctx.get_conn().exec_params(
                "SELECT id, valid_from, valid_until, classification, eval_file_id "
                "FROM registrar_certification WHERE registrar_id=$1::bigint "
                "ORDER BY valid_from DESC, id DESC",
                Database::query_param_list(registrar_id_));
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
            result.push_back(std::move(rc));
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
        LOGGER(PACKAGE).info("Failed to get registrar certifications due to unknown exception");
        throw;
    }
}

} // namespace LibFred::Registrar
} // namespace LibFred
