#include "src/epp/domain/domain_check_localization.h"

#include "src/epp/domain/domain_registration_obstruction.h"
#include "src/epp/session_lang.h"

#include <map>

namespace Epp {

namespace Domain {

namespace {

typedef std::map<DomainRegistrationObstruction::Enum, std::string> DomainRegistrationObstructionToDescription;
typedef std::map<unsigned, DomainRegistrationObstruction::Enum> DescriptionIdToDomainRegistrationObstruction;

std::string get_localized_reason_column_name(SessionLang::Enum lang)
{
    switch (lang) {
        case SessionLang::en: return "reason";
        case SessionLang::cs: return "reason_cs";
    }
    throw UnknownLocalizationLanguage();
}

DomainRegistrationObstructionToDescription get_localized_description_of_obstructions(
    Fred::OperationContext &ctx,
    const DescriptionIdToDomainRegistrationObstruction &obstructions,
    SessionLang::Enum lang)
{
    DomainRegistrationObstructionToDescription result;
    if (obstructions.empty()) {
        return result;
    }

    Database::query_param_list params;
    std::string query = "SELECT id AS obstruction_description_id, " +
                        get_localized_reason_column_name(lang) + " AS obstruction_description "
                        "FROM enum_reason "
                        "WHERE id IN (";
    for (DescriptionIdToDomainRegistrationObstruction::const_iterator obstruction_ptr = obstructions.begin();
         obstruction_ptr != obstructions.end();
         ++obstruction_ptr)
    {
        if (obstruction_ptr != obstructions.begin()) {
            query += ",";
        }
        query += ("$" + params.add(obstruction_ptr->first) + "::INTEGER");
    }
    query += ")";
    const Database::Result db_res = ctx.get_conn().exec_params(query, params);
    if (db_res.size() < obstructions.size()) {
        throw MissingLocalizedDescription();
    }

    for (std::size_t i = 0; i < db_res.size(); ++i) {
        const unsigned obstruction_description_id = static_cast<unsigned>(db_res[i]["obstruction_description_id"]);
        const DescriptionIdToDomainRegistrationObstruction::const_iterator obstruction_ptr =
            obstructions.find(obstruction_description_id);
        if (obstruction_ptr == obstructions.end()) {
            throw UnknownLocalizedDescriptionId();
        }
        const DomainRegistrationObstruction::Enum obstruction = obstruction_ptr->second;
        const std::string obstruction_description = static_cast< std::string >(db_res[i]["obstruction_description"]);
        result.insert(std::make_pair(obstruction, obstruction_description));
    }

    return result;
}

boost::optional<DomainLocalizedRegistrationObstruction> get_domain_localized_registration_obstruction(
    const Nullable<DomainRegistrationObstruction::Enum> &domain_registration_obstruction,
    const DomainRegistrationObstructionToDescription &localized_descriptions)
{
    if (domain_registration_obstruction.isnull()) {
        return boost::optional<DomainLocalizedRegistrationObstruction>();
    }
    const DomainRegistrationObstruction::Enum obstruction = domain_registration_obstruction.get_value();
    const DomainRegistrationObstructionToDescription::const_iterator localized_description_ptr =
        localized_descriptions.find(obstruction);
    if (localized_description_ptr == localized_descriptions.end()) {
        throw MissingLocalizedDescription();
    }
    const std::string description = localized_description_ptr->second;
    return DomainLocalizedRegistrationObstruction(obstruction, description);
}

} // namespace Epp::Domain::{anonymous}

DomainFqdnToDomainLocalizedRegistrationObstruction create_domain_fqdn_to_domain_localized_registration_obstruction(
    Fred::OperationContext &ctx,
    const DomainFqdnToDomainRegistrationObstruction &domain_fqdn_to_domain_registration_obstruction,
    SessionLang::Enum lang)
{
    DescriptionIdToDomainRegistrationObstruction used_obstructions;
    for (DomainFqdnToDomainRegistrationObstruction::const_iterator check_ptr = domain_fqdn_to_domain_registration_obstruction.begin();
         check_ptr != domain_fqdn_to_domain_registration_obstruction.end();
         ++check_ptr)
    {
        if (!check_ptr->second.isnull()) {
            const DomainRegistrationObstruction::Enum obstruction = check_ptr->second.get_value();
            const Reason::Enum reason = DomainRegistrationObstruction::to_reason(obstruction);
            const unsigned description_id = to_description_db_id(reason);
            used_obstructions[description_id] = obstruction;
        }
    }
    const DomainRegistrationObstructionToDescription localized_descriptions =
        get_localized_description_of_obstructions(ctx, used_obstructions, lang);

    DomainFqdnToDomainLocalizedRegistrationObstruction result;

    for (DomainFqdnToDomainRegistrationObstruction::const_iterator check_ptr = domain_fqdn_to_domain_registration_obstruction.begin();
         check_ptr != domain_fqdn_to_domain_registration_obstruction.end();
         ++check_ptr)
    {
        const std::string domain_fqdn = check_ptr->first;
        result.insert(std::make_pair(domain_fqdn, get_domain_localized_registration_obstruction(check_ptr->second, localized_descriptions)));
    }

    return result;
}

}

}
