/* Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/fredlib/registrar/get_registrar_zone_credit.h"
#include "src/fredlib/registrar/info_registrar.h"

#include "src/fredlib/opexception.h"

namespace Fred {

namespace {

unsigned long long get_registrar_id_by_handle(OperationContext& ctx, const std::string& registrar_handle)
{
    try
    {
        return InfoRegistrarByHandle(registrar_handle).exec(ctx).info_registrar_data.id;
    }
    catch (const InfoRegistrarByHandle::Exception& e)
    {
        if (e.is_set_unknown_registrar_handle())
        {
            struct RegistrarNotFound:Fred::OperationException
            {
                const char* what()const noexcept { return "no registrar identified by given handle"; }
            };
            throw RegistrarNotFound();
        }
    }
    catch (...) { }
    struct UnexpectedExceptionOccurred:Fred::InternalError
    {
        UnexpectedExceptionOccurred():Fred::InternalError(std::string()) { }
        const char* what()const noexcept { return "InfoRegistrarByHandle failure"; }
    };
    throw UnexpectedExceptionOccurred();
}

}//namespace Fred::{anonymous}

RegistrarZoneCredit GetRegistrarZoneCredit::exec(OperationContext& _ctx, const std::string& _registrar_handle)const
{
    const unsigned long long registrar_id = get_registrar_id_by_handle(_ctx, _registrar_handle);

    const Database::Result db_res = _ctx.get_conn().exec_params(
            "SELECT LOWER(z.fqdn),rc.credit "
            "FROM zone z "
            "LEFT JOIN registrar_credit rc ON rc.registrar_id=$1::BIGINT AND rc.zone_id=z.id",
            Database::query_param_list(registrar_id));

    RegistrarZoneCredit registrar_zone_credit;

    for (unsigned idx = 0; idx < db_res.size(); ++idx)
    {
        struct GetZoneCredit
        {
            static ZoneCredit from(const Database::Result::Row& _columns)
            {
                const std::string zone_fqdn = static_cast<std::string>(_columns[0]);
                const bool credit_presents = !_columns[1].isnull();
                if (!credit_presents)
                {
                    return ZoneCredit(zone_fqdn);
                }
                const Decimal credit = static_cast<Decimal>(static_cast<std::string>(_columns[1]));
                return ZoneCredit(zone_fqdn, credit);
            }
        };
        const bool zone_is_unique = registrar_zone_credit.insert(GetZoneCredit::from(db_res[idx])).second;
        if (!zone_is_unique)
        {
            struct ZoneCreditNotUnique:Fred::InternalError
            {
                ZoneCreditNotUnique():Fred::InternalError(std::string()) { }
                const char* what()const noexcept { return "second credit assigned to one zone"; }
            };
            throw ZoneCreditNotUnique();
        }
    }
    return registrar_zone_credit;
}

}//namespace Fred
