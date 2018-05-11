#include "tools/disclose_flags_updater/contact_search_query.hh"

namespace {


template<typename T>
std::string sql_disclose_flag_condition(const std::string& _column, const std::string& _operator, T _value)
{
    std::string condition;
    if (_value != T::not_set) {
        condition += _column + " " + _operator + " ";
        if (_value == T::show) {
            condition += "True";
        }
        else if (_value == T::hide) {
            condition += "False";
        }
    }
    return condition;
}


void sql_or(std::string& _dst_sql, const std::string& _expr)
{
    if (!_expr.empty())
    {
        if (!_dst_sql.empty())
        {
            _dst_sql += " OR ";
        }
        _dst_sql += _expr;
    }
}


}


namespace Tools {
namespace DiscloseFlagsUpdater {


std::string make_query_search_contact_needs_update(const DiscloseSettings& _discloses)
{
    std::string contact_search_sql =
        "SELECT contact_list.id, contact_list.hide_address_allowed"
         " FROM (SELECT c.id, array_agg(eos.name) && '{identifiedContact,validatedContact}'::varchar[]"
                    " AND coalesce(btrim(c.organization), '') = '' AS hide_address_allowed"
                 " FROM contact c"
                 " LEFT JOIN object_state os ON os.object_id = c.id AND os.valid_to IS NULL"
                 " LEFT JOIN enum_object_states eos ON eos.id = os.state_id"
                " GROUP BY 1) AS contact_list"
         " JOIN contact c ON c.id = contact_list.id";

    std::string search_conditions;
    sql_or(search_conditions, sql_disclose_flag_condition("disclosename", "IS NOT", _discloses.name));
    sql_or(search_conditions, sql_disclose_flag_condition("discloseorganization", "IS NOT", _discloses.org));
    sql_or(search_conditions, sql_disclose_flag_condition("disclosetelephone", "IS NOT", _discloses.voice));
    sql_or(search_conditions, sql_disclose_flag_condition("disclosefax", "IS NOT", _discloses.fax));
    sql_or(search_conditions, sql_disclose_flag_condition("discloseemail", "IS NOT", _discloses.email));
    sql_or(search_conditions, sql_disclose_flag_condition("disclosevat", "IS NOT", _discloses.vat));
    sql_or(search_conditions, sql_disclose_flag_condition("discloseident", "IS NOT", _discloses.ident));
    sql_or(search_conditions, sql_disclose_flag_condition("disclosenotifyemail", "IS NOT", _discloses.notify_email));
    if (_discloses.addr == DiscloseAddressValue::hide_verified)
    {
        std::string subcondition = "(contact_list.hide_address_allowed AND "
            + sql_disclose_flag_condition("discloseaddress", "IS NOT", DiscloseValue::hide) + ")";
        sql_or(search_conditions, subcondition);
    }
    else
    {
        sql_or(search_conditions, sql_disclose_flag_condition("discloseaddress", "IS NOT", _discloses.addr));
    }

    contact_search_sql += " WHERE " + search_conditions;
    return contact_search_sql;
}


}
}
