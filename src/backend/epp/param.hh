/*
 * Copyright (C) 2016-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
#ifndef PARAM_HH_035697EE12824255929BBCF05A575625
#define PARAM_HH_035697EE12824255929BBCF05A575625

namespace Epp {

struct Param
{
    enum Enum
    {
        poll_msg_id,
        contact_handle,
        contact_cc,
        nsset_handle,
        nsset_tech,
        nsset_dns_name,
        nsset_dns_addr,
        nsset_dns_name_add,
        nsset_dns_name_rem,
        nsset_tech_add,
        nsset_tech_rem,
        domain_fqdn,
        domain_registrant,
        domain_nsset,
        domain_keyset,
        domain_period,
        domain_admin,
        domain_tmpcontact,
        domain_ext_val_date,
        domain_ext_val_date_missing,
        domain_cur_exp_date,
        domain_admin_add,
        domain_admin_rem,
        keyset_handle,
        keyset_tech,
        keyset_dsrecord,
        keyset_dsrecord_add,
        keyset_dsrecord_rem,
        keyset_tech_add,
        keyset_tech_rem,
        registrar_autor,
        keyset_dnskey,
        keyset_dnskey_add,
        keyset_dnskey_rem
    };
};

} // namespace Epp

#endif
