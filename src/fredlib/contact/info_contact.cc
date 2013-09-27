/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
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

/**
 *  @file
 *  contact info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/contact/info_contact.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{
    InfoContact::InfoContact(const std::string& handle)
    : handle_(handle)
    , lock_(false)
    {}

    InfoContact& InfoContact::set_lock(bool lock)//set lock object_registry row
    {
        lock_ = lock;
        return *this;
    }

    InfoContactOutput InfoContact::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        InfoContactOutput contact_info_output;

        try
        {
            //info about contact and optionally lock object_registry row for update
            {
                Database::Result res = ctx.get_conn().exec_params(std::string(
                "SELECT (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp "// utc timestamp 0
                " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp  "// local zone timestamp 1
                " , cobr.crhistoryid "//first historyid 2
                " , cobr.historyid "// last historyid 3
                " , cobr.erdate "// contact delete time 4
                " , cobr.id,cobr.name,cobr.roid " //contact 5-7
                " , o.clid,clr.handle "//sponzoring registrar 8-9
                " , cobr.crid, crr.handle "//creating registrar 10-11
                " , o.upid, upr.handle "//updated by registrar 12-13
                " , cobr.crdate,o.trdate,o.update "//registration dates 14-16
                " , o.authinfopw "//authinfo 17
                " , c.name, c.organization, c.street1, c.street2, c.street3, c.city, c.stateorprovince, c.postalcode, c.country "// contact data 18-26
                " , c.telephone, c.fax, c.email , c.notifyemail, c.vat, c.ssn, est.type "// 27-33
                " , c.disclosename, c.discloseorganization, c.discloseaddress, c.disclosetelephone, c.disclosefax "// 34-38
                " , c.discloseemail, c.disclosevat, c.discloseident, c.disclosenotifyemail "// 39-42
                " FROM object_registry cobr "
                " JOIN contact c ON cobr.id=c.id "
                " JOIN object o ON c.id=o.id "
                " JOIN registrar clr ON clr.id = o.clid "
                " JOIN registrar crr ON crr.id = cobr.crid "
                " LEFT JOIN registrar upr ON upr.id = o.upid "
                " LEFT JOIN enum_ssntype est ON est.id = c.ssntype "
                " WHERE cobr.name=UPPER($2::text) AND cobr.erdate IS NULL "
                " AND cobr.type = ( SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text)")
                + (lock_ ? std::string(" FOR UPDATE OF cobr") : std::string(""))
                , Database::query_param_list(local_timestamp_pg_time_zone_name)(handle_));

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_contact_handle(handle_));
                }
                if (res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get contact"));
                }

                contact_info_output.utc_timestamp = res[0][0].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][0]));// utc timestamp

                contact_info_output.local_timestamp = res[0][1].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
                : boost::posix_time::time_from_string(static_cast<std::string>(res[0][1]));//local zone timestamp

                contact_info_output.info_contact_data.crhistoryid = static_cast<unsigned long long>(res[0][2]);//cobr.crhistoryid

                contact_info_output.info_contact_data.historyid = static_cast<unsigned long long>(res[0][3]);//cobr.historyid

                contact_info_output.info_contact_data.delete_time = res[0][4].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][4])));//cobr.erdate

                contact_info_output.info_contact_data.handle = static_cast<std::string>(res[0][6]);//cobr.name

                contact_info_output.info_contact_data.roid = static_cast<std::string>(res[0][7]);//cobr.roid

                contact_info_output.info_contact_data.sponsoring_registrar_handle = static_cast<std::string>(res[0][9]);//clr.handle

                contact_info_output.info_contact_data.create_registrar_handle = static_cast<std::string>(res[0][11]);//crr.handle

                contact_info_output.info_contact_data.update_registrar_handle = res[0][13].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[0][13]));//upr.handle

                contact_info_output.info_contact_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[0][14]));//cobr.crdate

                contact_info_output.info_contact_data.transfer_time = res[0][15].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][15])));//o.trdate

                contact_info_output.info_contact_data.update_time = res[0][16].isnull() ? Nullable<boost::posix_time::ptime>()
                : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[0][16])));//o.update

                contact_info_output.info_contact_data.authinfopw = static_cast<std::string>(res[0][17]);//o.authinfopw


                contact_info_output.info_contact_data.name = res[0][18].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][18]));
                contact_info_output.info_contact_data.organization = res[0][19].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][19]));
                contact_info_output.info_contact_data.street1 = res[0][20].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][20]));
                contact_info_output.info_contact_data.street2 = res[0][21].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][21]));
                contact_info_output.info_contact_data.street3 = res[0][22].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][22]));
                contact_info_output.info_contact_data.city = res[0][23].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][23]));
                contact_info_output.info_contact_data.stateorprovince = res[0][24].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][24]));
                contact_info_output.info_contact_data.postalcode = res[0][25].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][25]));
                contact_info_output.info_contact_data.country = res[0][26].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][26]));
                contact_info_output.info_contact_data.telephone = res[0][27].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][27]));
                contact_info_output.info_contact_data.fax = res[0][28].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][28]));
                contact_info_output.info_contact_data.email = res[0][29].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][29]));
                contact_info_output.info_contact_data.notifyemail = res[0][30].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][30]));
                contact_info_output.info_contact_data.vat = res[0][31].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][31]));
                contact_info_output.info_contact_data.ssn = res[0][32].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][32]));
                contact_info_output.info_contact_data.ssntype = res[0][33].isnull() ? Nullable<std::string>()
                        : Nullable<std::string> (static_cast<std::string>(res[0][33]));
                contact_info_output.info_contact_data.disclosename = res[0][34].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][34]));
                contact_info_output.info_contact_data.discloseorganization = res[0][35].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][35]));
                contact_info_output.info_contact_data.discloseaddress = res[0][36].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][36]));
                contact_info_output.info_contact_data.disclosetelephone = res[0][37].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][37]));
                contact_info_output.info_contact_data.disclosefax = res[0][38].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][38]));
                contact_info_output.info_contact_data.discloseemail = res[0][39].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][39]));
                contact_info_output.info_contact_data.disclosevat = res[0][40].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][40]));
                contact_info_output.info_contact_data.discloseident = res[0][41].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][41]));
                contact_info_output.info_contact_data.disclosenotifyemail = res[0][42].isnull() ? Nullable<bool>()
                        : Nullable<bool> (static_cast<bool>(res[0][42]));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_info_output;
    }//InfoContact::exec

    std::string InfoContact::to_string() const
    {
        return Util::format_operation_state("InfoContact",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle",handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    std::string InfoContactOutput::to_string() const
    {
        return Util::format_data_structure("InfoContactOutput",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_contact_data",info_contact_data.to_string()))
        (std::make_pair("utc_timestamp",boost::lexical_cast<std::string>(utc_timestamp)))
        (std::make_pair("local_timestamp",boost::lexical_cast<std::string>(local_timestamp)))
        );
    }

}//namespace Fred

