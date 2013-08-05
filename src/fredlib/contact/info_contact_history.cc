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
 *  @file info_contact_history.cc
 *  contact history info
 */

#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

#include "fredlib/contact/info_contact_history.h"
#include "fredlib/object/object.h"

#include "fredlib/opcontext.h"
#include "fredlib/db_settings.h"
#include "util/optional_value.h"
#include "util/db/nullable.h"
#include "util/util.h"

namespace Fred
{

    InfoContactHistory::InfoContactHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp
            , const std::string& registrar)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , registrar_(registrar)
        , lock_(false)
    {}

    InfoContactHistory::InfoContactHistory(const std::string& roid, const std::string& registrar)
    : roid_(roid)
    , registrar_(registrar)
    , lock_(false)
    {}

    InfoContactHistory& InfoContactHistory::set_history_timestamp(boost::posix_time::ptime history_timestamp)//set history timestamp
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoContactHistory& InfoContactHistory::set_lock(bool lock)//set lock object_registry row for contact
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoContactHistoryOutput> InfoContactHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactHistoryOutput> contact_history_res;

        try
        {
            //check registrar exists
            //TODO: check registrar access
            {
                Database::Result res = ctx.get_conn().exec_params(
                        "SELECT id FROM registrar WHERE handle = UPPER($1::text) FOR SHARE"
                    , Database::query_param_list(registrar_));

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registrar_handle(registrar_));
                }
                if (res.size() != 1)
                {
                    BOOST_THROW_EXCEPTION(InternalError("failed to get registrar"));
                }
            }

            //info about contact history by roid and optional history timestamp
            if(!roid_.empty())
            {
                //query params
                Database::QueryParams params;
                params.push_back(roid_);
                if (history_timestamp_.isset())
                {
                    params.push_back(history_timestamp_);
                    params.push_back(local_timestamp_pg_time_zone_name);
                }
                //get info and lock object_registry row for update if set
                Database::Result res = ctx.get_conn().exec_params(std::string(
                "SELECT cobr.id, cobr.roid, cobr.name, cobr.erdate " //contact 0-3
                " , oh.historyid, h.id , h.next, h.valid_from, h.valid_to "//history 4-8
                " , oh.clid, clr.handle "//sponsoring registrar 9-10
                " , cobr.crid, crr.handle "//creating registrar 11-12
                " , oh.upid, upr.handle "//last updated by registrar 13-14
                " , cobr.crdate, oh.trdate, oh.update "//registration dates 15-17
                " , oh.authinfopw "//transfer passwd 18
                " , cobr.crhistoryid "//first historyid 19
                " , h.request_id "//logd request_id 20
                " , ch.name, ch.organization, ch.street1, ch.street2, ch.street3, ch.city, ch.stateorprovince, ch.postalcode, ch.country "//contact data
                " , ch.telephone, ch.fax, ch.email , ch.notifyemail, ch.vat, ch.ssn, est.type "
                " , ch.disclosename, ch.discloseorganization, ch.discloseaddress, ch.disclosetelephone, ch.disclosefax "
                " , ch.discloseemail, ch.disclosevat, ch.discloseident, ch.disclosenotifyemail "
                " FROM object_registry cobr "
                " JOIN object_history oh ON oh.id = cobr.id "
                " JOIN contact_history ch ON ch.historyid = oh.historyid "
                " JOIN history h ON h.id = ch.historyid "
                " JOIN registrar clr ON clr.id = oh.clid "
                " JOIN registrar crr ON crr.id = cobr.crid "
                " LEFT JOIN registrar upr ON upr.id = oh.upid "
                " LEFT JOIN enum_ssntype est ON est.id = ch.ssntype "
                " WHERE "
                " cobr.roid = $1::text "
                " AND cobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) ")
                + (history_timestamp_.isset()
                  ? " AND h.valid_from <= ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' "
                    " AND ($2::timestamp AT TIME ZONE $3::text) AT TIME ZONE 'UTC' < h.valid_to "
                  : std::string())
                + std::string(" ORDER BY h.id DESC ")
                + (lock_ ? std::string(" FOR UPDATE of cobr ") : std::string())
                , params);

                if (res.size() == 0)
                {
                    BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
                }

                contact_history_res.reserve(res.size());//alloc
                for(Database::Result::size_type i = 0; i < res.size(); ++i)
                {
                    InfoContactHistoryOutput contact_history_output;

                    contact_history_output.info_contact_data.roid = static_cast<std::string>(res[i][1]);//cobr.roid

                    contact_history_output.info_contact_data.handle = static_cast<std::string>(res[i][2]);//cobr.name

                    contact_history_output.info_contact_data.delete_time = res[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][3])));//cobr.erdate

                    contact_history_output.info_contact_data.historyid = static_cast<unsigned long long>(res[i][4]);//oh.historyid

                    contact_history_output.next_historyid = res[i][6].isnull() ? Nullable<unsigned long long>()
                    : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][6]));//h.next

                    contact_history_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(res[i][7]));//h.valid_from

                    contact_history_output.history_valid_to = res[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][8])));//h.valid_to

                    contact_history_output.info_contact_data.sponsoring_registrar_handle = static_cast<std::string>(res[i][10]);//clr.handle

                    contact_history_output.info_contact_data.create_registrar_handle = static_cast<std::string>(res[i][12]);//crr.handle

                    contact_history_output.info_contact_data.update_registrar_handle = res[i][14].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(res[i][14]));//upr.handle

                    contact_history_output.info_contact_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(res[i][15]));//cobr.crdate

                    contact_history_output.info_contact_data.transfer_time = res[i][16].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][16])));//oh.trdate

                    contact_history_output.info_contact_data.update_time = res[i][17].isnull() ? Nullable<boost::posix_time::ptime>()
                    : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(res[i][17])));//oh.update

                    contact_history_output.info_contact_data.authinfopw = static_cast<std::string>(res[i][18]);//oh.authinfopw

                    contact_history_output.info_contact_data.crhistoryid = static_cast<unsigned long long>(res[i][19]);//cobr.crhistoryid

                    contact_history_output.logd_request_id = res[i][20].isnull() ? Nullable<unsigned long long>()
                        : Nullable<unsigned long long>(static_cast<unsigned long long>(res[i][20]));

                    contact_history_output.info_contact_data.name = res[i][21].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][21]));
                    contact_history_output.info_contact_data.organization = res[i][22].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][22]));
                    contact_history_output.info_contact_data.street1 = res[i][23].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][23]));
                    contact_history_output.info_contact_data.street2 = res[i][24].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][24]));
                    contact_history_output.info_contact_data.street3 = res[i][25].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][25]));
                    contact_history_output.info_contact_data.city = res[i][26].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][26]));
                    contact_history_output.info_contact_data.stateorprovince = res[i][27].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][27]));
                    contact_history_output.info_contact_data.postalcode = res[i][28].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][28]));
                    contact_history_output.info_contact_data.country = res[i][29].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][29]));
                    contact_history_output.info_contact_data.telephone = res[i][30].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][30]));
                    contact_history_output.info_contact_data.fax = res[i][31].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][31]));
                    contact_history_output.info_contact_data.email = res[i][32].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][32]));
                    contact_history_output.info_contact_data.notifyemail = res[i][33].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][33]));
                    contact_history_output.info_contact_data.vat = res[i][34].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][34]));
                    contact_history_output.info_contact_data.ssn = res[i][35].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][35]));
                    contact_history_output.info_contact_data.ssntype = res[i][36].isnull() ? Nullable<std::string>()
                            : Nullable<std::string> (static_cast<std::string>(res[i][36]));
                    contact_history_output.info_contact_data.disclosename = res[i][37].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][37]));
                    contact_history_output.info_contact_data.discloseorganization = res[i][38].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][38]));
                    contact_history_output.info_contact_data.discloseaddress = res[i][39].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][39]));
                    contact_history_output.info_contact_data.disclosetelephone = res[i][40].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][40]));
                    contact_history_output.info_contact_data.disclosefax = res[i][41].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][41]));
                    contact_history_output.info_contact_data.discloseemail = res[i][42].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][42]));
                    contact_history_output.info_contact_data.disclosevat = res[i][43].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][43]));
                    contact_history_output.info_contact_data.discloseident = res[i][44].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][44]));
                    contact_history_output.info_contact_data.disclosenotifyemail = res[i][45].isnull() ? Nullable<bool>()
                            : Nullable<bool> (static_cast<bool>(res[i][45]));

                    contact_history_res.push_back(contact_history_output);
                }//for res
            }//if roid
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_history_res;
    }//InfoContactHistory::exec

    std::ostream& operator<<(std::ostream& os, const InfoContactHistory& ich)
    {
        return os << "#InfoContactHistory roid: " << ich.roid_
                << " history_timestamp: " << ich.history_timestamp_.print_quoted()
                << " registrar: " << ich.registrar_
                << " lock: " << ich.lock_
                ;
    }
    std::string InfoContactHistory::to_string()
    {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }


}//namespace Fred

