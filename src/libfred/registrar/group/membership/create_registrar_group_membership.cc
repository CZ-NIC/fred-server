#include "src/libfred/registrar/group/membership/create_registrar_group_membership.hh"
#include "src/libfred/registrar/group/membership/exceptions.hh"

#include "src/libfred/db_settings.hh"

namespace LibFred {
namespace Registrar {

unsigned long long CreateRegistrarGroupMembership::exec(OperationContext& _ctx)
{
    try
    {
        if (!member_until_.is_not_a_date() && member_from_ > member_until_)
        {
            throw WrongIntervalOrder();
        }

        _ctx.get_conn().exec("LOCK TABLE registrar_group_map IN ACCESS EXCLUSIVE MODE");
        const Database::Result last_membership = _ctx.get_conn().exec_params(
                "SELECT id, member_until FROM registrar_group_map "
                "WHERE registrar_id=$1::bigint "
                "AND registrar_group_id=$2::bigint "
                "ORDER BY member_until DESC LIMIT 1",
                Database::query_param_list(registrar_id_)(group_id_));
        if (last_membership.size() > 0)
        {
            if (static_cast<std::string>(last_membership[0][1]).empty())
            {
                _ctx.get_conn().exec_params(
                        "UPDATE registrar_group_map SET member_until=$1::date WHERE id=$2::bigint ",
                        Database::query_param_list(member_from_)(static_cast<unsigned long long>(last_membership[0][0])));
            }
            else if (boost::gregorian::from_string(last_membership[0][1]) > member_from_)
            {
                throw IntervalIntersection();
            }
        }

        const std::string query =
                    "INSERT INTO registrar_group_map (registrar_id, registrar_group_id, member_from, member_until) "
                    "VALUES ($1::bigint, $2::bigint, $3::date, $4::date) RETURNING ID";
        Database::query_param_list query_list = Database::query_param_list(registrar_id_)(group_id_)(member_from_);
        if (!member_until_.is_not_a_date() && !member_until_.is_pos_infinity())
        {
            query_list = query_list(member_until_);
        }
        else
        {
            query_list = query_list(Database::QueryParam());
        }
        return _ctx.get_conn().exec_params(query, query_list)[0][0];
    }
    catch (const std::exception& e)
    {
        LOGGER(PACKAGE).error(e.what());
        throw;
    }
    catch (...)
    {
        LOGGER(PACKAGE).info("Failed to create registrar group membership due to an unknown exception");
        throw;
    }
}

} // namespace Registrar
} // namespace LibFred
