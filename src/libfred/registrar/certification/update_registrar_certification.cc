#include "src/libfred/opcontext.hh"
#include "src/libfred/registrar/certification/exceptions.hh"
#include "src/libfred/registrar/certification/update_registrar_certification.hh"

#include <boost/date_time/gregorian/gregorian.hpp>

namespace LibFred {
namespace Registrar {

void UpdateRegistrarCertification::exec(OperationContext& _ctx)
{
    try
    {
        Database::ParamQuery query = Database::ParamQuery("UPDATE registrar_certification SET ");
        if (valid_until_ != boost::none)
        {
            if (valid_until_->is_special())
            {
                throw InvalidDateTo();
            }
            const Database::Result cert_in_past = _ctx.get_conn().exec_params(
                    // clang-format off
                    "SELECT now()::date > $1::date",
                    // clang-format on
                    Database::query_param_list(*valid_until_));
            const bool expired_date_to = static_cast<bool>(cert_in_past[0][0]);
            if (expired_date_to)
            {
                throw CertificationInPast();
            }

            const Database::Result from_until = _ctx.get_conn().exec_params(
                    // clang-format off
                    "SELECT valid_from FROM registrar_certification "
                    "WHERE id = $1::bigint FOR UPDATE",
                    // clang-format on
                    Database::query_param_list(certification_id_));
            const boost::gregorian::date old_from = boost::gregorian::from_string(
                    static_cast<std::string>(from_until[0][0]));
            if (old_from > *valid_until_)
            {
                throw WrongIntervalOrder();
            }

            query = query("valid_until = ")
                .param_date(*valid_until_);
        }
        else
        {
            constexpr int min_classification_value = 0;
            constexpr int max_classification_value = 5;

            const bool is_clasification_out_of_range = (*classification_ < min_classification_value) ||
                                                       (*classification_ > max_classification_value);
            if (is_clasification_out_of_range)
            {
                throw ScoreOutOfRange();
            }
            query = query("classification = ")
                            .param(*classification_, "integer")(", eval_file_id = ")
                            .param_bigint(*eval_file_id_);
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
        LOGGER(PACKAGE).info("Failed to update registrar certification due to an unknown exception");
        throw;
    }
}

} // namespace LibFred::Registrar
} // namespace LibFred
