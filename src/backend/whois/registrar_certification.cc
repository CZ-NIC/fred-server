#include "src/backend/whois/registrar_certification.hh"

namespace Fred {
namespace Backend {
namespace Whois {

    std::vector<RegistrarCertificationData> get_registrar_certifications(
            LibFred::OperationContext& ctx)
    {
        std::vector<RegistrarCertificationData> ret;

        Database::Result registrar_certs_res = ctx.get_conn().exec(
            // clang-format off
            "SELECT DISTINCT ON (r.handle)"
                " r.handle AS registrar_handle,"
                " rc.classification AS score,"
                " rc.eval_file_id AS evaluation_file_id"
            " FROM registrar_certification rc"
                " JOIN registrar r ON rc.registrar_id = r.id"
                " AND rc.valid_from <= CURRENT_DATE"
                " AND rc.valid_until >= CURRENT_DATE"
            " ORDER BY r.handle, rc.valid_from DESC");
            // clang-format on

        ret.reserve(registrar_certs_res.size());
        for(unsigned long long i = 0; i < registrar_certs_res.size(); ++i)
        {
            ret.push_back(RegistrarCertificationData(
                static_cast<std::string>(registrar_certs_res[i]["registrar_handle"]),
                static_cast<short>(registrar_certs_res[i]["score"]),
                static_cast<unsigned long long>(registrar_certs_res[i]["evaluation_file_id"])
            ));
        }
        return ret;
    }

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred
