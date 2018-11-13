#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/certification/create_registrar_certification.hh"
#include "src/libfred/registrar/certification/exceptions.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace LibFred {
namespace Registrar {

CreateRegistrarCertification& CreateRegistrarCertification::set_valid_until(const boost::gregorian::date& _valid_until)
{
    valid_until_ = _valid_until;
    return *this;
}

unsigned long long CreateRegistrarCertification::exec(OperationContext& _ctx)
{
    try
    {
        if (valid_from_.is_special())
        {
            throw InvalidDateFrom();
        }
        if (valid_from_ > valid_until_)
        {
            throw WrongIntervalOrder();
        }

        constexpr int min_classification_value = 0;
        constexpr int max_classification_value = 5;
        const bool clasification_is_out_of_range = (classification_ < min_classification_value) ||
                                                   (classification_ > max_classification_value);
        if (clasification_is_out_of_range)
        {
            throw ScoreOutOfRange();
        }

        if (!valid_until_.is_special())
        {
            const Database::Result cert_in_past = _ctx.get_conn().exec_params(
                    // clang-format off
                    "SELECT now()::date > $1::date",
                    // clang-format on
                    Database::query_param_list(valid_until_));
            const bool expired_date_to = static_cast<bool>(cert_in_past[0][0]);
            if (expired_date_to)
            {
                throw CertificationInPast();
            }
        }

        _ctx.get_conn().exec("LOCK TABLE registrar_certification IN ACCESS EXCLUSIVE MODE");
        const Database::Result terminate_last_cert = _ctx.get_conn().exec_params(
                // clang-format off
                "UPDATE registrar_certification "
                "SET valid_until = ($1::date - interval '1 day') "
                "WHERE registrar_id = $2::bigint "
                "AND valid_from < $1::date "
                "AND valid_until IS NULL ",
                // clang-format on
                Database::query_param_list(valid_from_)(registrar_id_));

        const Database::Result range_overlap = _ctx.get_conn().exec_params(
                // clang-format off
                "SELECT * FROM registrar_certification "
                "WHERE registrar_id = $1::bigint "
                "AND (valid_until IS NULL "
                "OR valid_until >= $2::date) ",
                // clang-format on
                Database::query_param_list(registrar_id_)(valid_from_));
        if (range_overlap.size() > 0)
        {
            throw OverlappingRange();
        }

        const Database::Result created_certification = _ctx.get_conn().exec_params(
                // clang-format off
                "INSERT INTO registrar_certification "
                "(registrar_id, valid_from, valid_until, classification, eval_file_id) "
                "VALUES ($1::bigint, $2::date, $3::date, $4::integer, $5::bigint) "
                "RETURNING id",
                // clang-format on
                Database::query_param_list(registrar_id_)
                                        (valid_from_)
                                        (valid_until_.is_special() ? Database::QPNull : valid_until_)
                                        (classification_)
                                        (eval_file_id_));
        const auto id = static_cast<unsigned long long>(created_certification[0][0]);
        return id;
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to create registrar certification due to unknown exception");
        throw;
    }
}

} // namespace LibFred::Registrar
} // namespace LibFred
