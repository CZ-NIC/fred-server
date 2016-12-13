#include "src/epp/impl/util.h"

#include "src/epp/impl/exception.h"
#include "src/epp/impl/reason.h"
#include "src/epp/impl/session_lang.h"
#include "src/fredlib/db_settings.h"
#include "src/fredlib/opcontext.h"

#include <boost/algorithm/string/join.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

#include <string>
#include <set>
#include <vector>

namespace Epp {

std::string get_db_table_enum_reason_column_name_for_language(SessionLang::Enum _lang)
{
    switch (_lang)
    {
        case SessionLang::en:
            return "reason";

        case SessionLang::cs:
            return "reason_cs";
    }
    throw UnknownLocalizationLanguage();
}

/**
 * @returns untyped postgres array
 * Caller should cast it properly before using in query.
 */
std::string convert_values_to_pg_array(const std::set<unsigned>& _input) {
    std::vector<std::string> string_values;
    string_values.reserve(_input.size());

    BOOST_FOREACH(unsigned elem, _input) {
        string_values.push_back( boost::lexical_cast<std::string>(elem) );
    }

    return "{" + boost::algorithm::join(string_values, ", ") + "}";
}

}
