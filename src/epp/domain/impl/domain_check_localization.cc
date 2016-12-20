#include "src/epp/domain/impl/domain_check_localization.h"

#include "src/epp/domain/impl/domain_registration_obstruction.h"
#include "src/epp/impl/session_lang.h"

#include <boost/optional.hpp>

#include <map>

namespace Epp {

namespace Domain {

namespace {

typedef std::map<DomainRegistrationObstruction::Enum, std::string> DomainRegistrationObstructionToDescription;
typedef std::map<unsigned, DomainRegistrationObstruction::Enum> DescriptionIdToDomainRegistrationObstruction;

std::string get_localized_reason_column_name(SessionLang::Enum _lang)
{
    switch (_lang) {
        case SessionLang::en: return "reason";
        case SessionLang::cs: return "reason_cs";
    }
    throw UnknownLocalizationLanguage();
}

DomainRegistrationObstructionToDescription get_localized_description_of_obstructions(
    Fred::OperationContext& _ctx,
    const DescriptionIdToDomainRegistrationObstruction& _obstructions,
    SessionLang::Enum _lang)
{
    DomainRegistrationObstructionToDescription result;
    if (_obstructions.empty()) {
        return result;
    }

    Database::query_param_list params;
    std::string query = "SELECT id AS obstruction_description_id, " +
                        get_localized_reason_column_name(_lang) + " AS obstruction_description "
                        "FROM enum_reason "
                        "WHERE id IN (";
    for (DescriptionIdToDomainRegistrationObstruction::const_iterator obstruction_ptr = _obstructions.begin();
         obstruction_ptr != _obstructions.end();
         ++obstruction_ptr)
    {
        if (obstruction_ptr != _obstructions.begin()) {
            query += ",";
        }
        query += ("$" + params.add(obstruction_ptr->first) + "::INTEGER");
    }
    query += ")";
    const Database::Result db_res = _ctx.get_conn().exec_params(query, params);
    if (db_res.size() < _obstructions.size()) {
        throw MissingLocalizedDescription();
    }

    for (std::size_t i = 0; i < db_res.size(); ++i) {
        const unsigned obstruction_description_id = static_cast<unsigned>(db_res[i]["obstruction_description_id"]);
        const DescriptionIdToDomainRegistrationObstruction::const_iterator obstruction_ptr =
            _obstructions.find(obstruction_description_id);
        if (obstruction_ptr == _obstructions.end()) {
            throw UnknownLocalizedDescriptionId();
        }
        const DomainRegistrationObstruction::Enum obstruction = obstruction_ptr->second;
        const std::string obstruction_description = static_cast<std::string>(db_res[i]["obstruction_description"]);
        result.insert(std::make_pair(obstruction, obstruction_description));
    }

    return result;
}

boost::optional<DomainLocalizedRegistrationObstruction> get_domain_localized_registration_obstruction(
    const Nullable<DomainRegistrationObstruction::Enum>& _domain_registration_obstruction,
    const DomainRegistrationObstructionToDescription& _localized_descriptions)
{
    if (_domain_registration_obstruction.isnull()) {
        return boost::optional<DomainLocalizedRegistrationObstruction>();
    }
    const DomainRegistrationObstruction::Enum obstruction = _domain_registration_obstruction.get_value();
    const DomainRegistrationObstructionToDescription::const_iterator localized_description_ptr =
        _localized_descriptions.find(obstruction);
    if (localized_description_ptr == _localized_descriptions.end()) {
        throw MissingLocalizedDescription();
    }
    const std::string description = localized_description_ptr->second;
    return DomainLocalizedRegistrationObstruction(obstruction, description);
}

} // namespace Epp::Domain::{anonymous}

DomainFqdnToDomainLocalizedRegistrationObstruction create_domain_fqdn_to_domain_localized_registration_obstruction(
    Fred::OperationContext& _ctx,
    const DomainFqdnToDomainRegistrationObstruction& _domain_fqdn_to_domain_registration_obstruction,
    SessionLang::Enum _lang)
{
    DescriptionIdToDomainRegistrationObstruction used_obstructions;
    for (DomainFqdnToDomainRegistrationObstruction::const_iterator check_ptr = _domain_fqdn_to_domain_registration_obstruction.begin();
         check_ptr != _domain_fqdn_to_domain_registration_obstruction.end();
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
        get_localized_description_of_obstructions(_ctx, used_obstructions, _lang);

    DomainFqdnToDomainLocalizedRegistrationObstruction result;

    for (DomainFqdnToDomainRegistrationObstruction::const_iterator check_ptr = _domain_fqdn_to_domain_registration_obstruction.begin();
         check_ptr != _domain_fqdn_to_domain_registration_obstruction.end();
         ++check_ptr)
    {
        const std::string domain_fqdn = check_ptr->first;
        result.insert(std::make_pair(domain_fqdn, get_domain_localized_registration_obstruction(check_ptr->second, localized_descriptions)));
    }

    return result;
}

}

}
