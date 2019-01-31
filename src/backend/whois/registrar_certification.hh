#ifndef REGISTRAR_CERTIFICATION_HH_41E9029ADF8B47A7B604F087420D5518
#define REGISTRAR_CERTIFICATION_HH_41E9029ADF8B47A7B604F087420D5518

#include "libfred/opcontext.hh"
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Whois {

class RegistrarCertificationData
{
    std::string registrar_handle_;
    short score_;
    unsigned long long evaluation_file_id_;

public:
    RegistrarCertificationData()
        : score_(0), evaluation_file_id_(0)
    {
    }

    RegistrarCertificationData(
            const std::string& registrar_handle,
            short score,
            unsigned long long evaluation_file_id)
        : registrar_handle_(registrar_handle), score_(score), evaluation_file_id_(evaluation_file_id)
    {
    }

    std::string get_registrar_handle() const
    {
        return registrar_handle_;
    }

    short get_registrar_score() const
    {
        return score_;
    }

    unsigned long long get_registrar_evaluation_file_id() const
    {
        return evaluation_file_id_;
    }
};

//list of current registrar certification data
std::vector<RegistrarCertificationData> get_registrar_certifications(
        LibFred::OperationContext& ctx);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
