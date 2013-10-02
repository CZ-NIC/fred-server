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
#include <sstream>
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

    InfoContactByHandle::InfoContactByHandle(const std::string& handle)
        : handle_(handle)
        , lock_(false)
    {}

    InfoContactByHandle& InfoContactByHandle::set_lock(bool lock)//set lock object_registry row for contact
    {
        lock_ = lock;
        return *this;
    }

    InfoContactOutput InfoContactByHandle::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_res;

        try
        {
            contact_res = InfoContactImpl()
                    .set_handle(handle_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_contact_handle(handle_));
            }

            if (contact_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_res.at(0);
    }//InfoContactByHandle::exec

    std::string InfoContactByHandle::to_string() const
    {
        return Util::format_operation_state("InfoContactByHandle",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("handle", handle_))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoContactById::InfoContactById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    InfoContactById& InfoContactById::set_lock(bool lock)//set lock object_registry row for contact
    {
        lock_ = lock;
        return *this;
    }

    InfoContactOutput InfoContactById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_res;

        try
        {
            contact_res = InfoContactImpl()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(false)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

            if (contact_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_res.at(0);
    }//InfoContactById::exec

    std::string InfoContactById::to_string() const
    {
        return Util::format_operation_state("InfoContactById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoContactHistory::InfoContactHistory(const std::string& roid
            , const Optional<boost::posix_time::ptime>& history_timestamp)
        : roid_(roid)
        , history_timestamp_(history_timestamp)
        , lock_(false)
    {}

    InfoContactHistory::InfoContactHistory(const std::string& roid)
    : roid_(roid)
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

    std::vector<InfoContactOutput> InfoContactHistory::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_history_res;

        try
        {
            contact_history_res = InfoContactImpl()
                    .set_roid(roid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_registry_object_identifier(roid_));
            }
        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_history_res;
    }//InfoContactHistory::exec

    std::string InfoContactHistory::to_string() const
    {
        return Util::format_operation_state("InfoContactHistory",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("roid",roid_))
        (std::make_pair("history_timestamp",history_timestamp_.print_quoted()))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }


    bool InfoContactOutput::operator==(const InfoContactOutput& rhs) const
    {
        return info_contact_data == rhs.info_contact_data;
    }

    bool InfoContactOutput::operator!=(const InfoContactOutput& rhs) const
    {
        return !this->operator ==(rhs);
    }

    std::string InfoContactOutput::to_string() const
    {
        return Util::format_data_structure("InfoContactOutput",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("info_contact_data",info_contact_data.to_string()))
        (std::make_pair("next_historyid",next_historyid.print_quoted()))
        (std::make_pair("history_valid_from",boost::lexical_cast<std::string>(history_valid_from)))
        (std::make_pair("history_valid_to",history_valid_to.print_quoted()))
        (std::make_pair("logd_request_id",logd_request_id.print_quoted()))
        );
    }

    HistoryInfoContactById::HistoryInfoContactById(unsigned long long id)
        : id_(id)
        , lock_(false)
    {}

    HistoryInfoContactById& HistoryInfoContactById::set_lock(bool lock)//set lock object_registry row for contact
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoContactOutput> HistoryInfoContactById::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_history_res;

        try
        {
            contact_history_res = InfoContactImpl()
                    .set_id(id_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_id(id_));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_history_res;
    }//HistoryInfoContactById::exec

    std::string HistoryInfoContactById::to_string() const
    {
        return Util::format_operation_state("HistoryInfoContactById",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("id",boost::lexical_cast<std::string>(id_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    HistoryInfoContactByHistoryid::HistoryInfoContactByHistoryid(unsigned long long historyid)
        : historyid_(historyid)
        , lock_(false)
    {}

    HistoryInfoContactByHistoryid& HistoryInfoContactByHistoryid::set_lock(bool lock)//set lock object_registry row for contact
    {
        lock_ = lock;
        return *this;
    }

    InfoContactOutput HistoryInfoContactByHistoryid::exec(OperationContext& ctx, const std::string& local_timestamp_pg_time_zone_name)
    {
        std::vector<InfoContactOutput> contact_history_res;

        try
        {
            contact_history_res = InfoContactImpl()
                    .set_historyid(historyid_)
                    .set_lock(lock_)
                    .set_history_query(true)
                    .exec(ctx,local_timestamp_pg_time_zone_name);

            if (contact_history_res.empty())
            {
                BOOST_THROW_EXCEPTION(Exception().set_unknown_object_historyid(historyid_));
            }

            if (contact_history_res.size() > 1)
            {
                BOOST_THROW_EXCEPTION(InternalError("query result size > 1"));
            }

        }//try
        catch(ExceptionStack& ex)
        {
            ex.add_exception_stack_info(to_string());
            throw;
        }
        return contact_history_res.at(0);
    }//HistoryInfoContactByHistoryid::exec

    std::string HistoryInfoContactByHistoryid::to_string() const
    {
        return Util::format_operation_state("HistoryInfoContactByHistoryid",
        Util::vector_of<std::pair<std::string,std::string> >
        (std::make_pair("historyid",boost::lexical_cast<std::string>(historyid_)))
        (std::make_pair("lock",lock_ ? "true":"false"))
        );
    }

    InfoContactImpl::InfoContactImpl()
        : history_query_(false)
        , lock_(false)
        {}

    InfoContactImpl& InfoContactImpl::set_handle(const std::string& contact_handle)
    {
        contact_handle_ = contact_handle;
        return *this;
    }

    InfoContactImpl& InfoContactImpl::set_roid(const std::string& contact_roid)
    {
        contact_roid_ = contact_roid;
        return *this;
    }

    InfoContactImpl& InfoContactImpl::set_id(unsigned long long contact_id)
    {
        contact_id_ = contact_id;
        return *this;
    }

    InfoContactImpl& InfoContactImpl::set_historyid(unsigned long long contact_historyid)
    {
        contact_historyid_ = contact_historyid;
        return *this;
    }

    InfoContactImpl& InfoContactImpl::set_history_timestamp(const boost::posix_time::ptime& history_timestamp)
    {
        history_timestamp_ = history_timestamp;
        return *this;
    }

    InfoContactImpl& InfoContactImpl::set_history_query(bool history_query)
    {
        history_query_ = history_query;
        return *this;
    }

    InfoContactImpl& InfoContactImpl::set_lock(bool lock)
    {
        lock_ = lock;
        return *this;
    }

    std::vector<InfoContactOutput> InfoContactImpl::exec(OperationContext& ctx
            , const std::string& local_timestamp_pg_time_zone_name
        )
    {
        std::vector<InfoContactOutput> result;

        //query params
        Database::QueryParams params;
        std::ostringstream sql;

        params.push_back(local_timestamp_pg_time_zone_name);//refered as $1

        //query to get info and lock object_registry row for update if set
        sql << "SELECT cobr.id, cobr.roid, cobr.name "//contact 0-2
        " , (cobr.erdate AT TIME ZONE 'UTC' ) AT TIME ZONE $1::text " //contact 3
        " , obj.id, h.id , h.next, (h.valid_from AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//history 4-7
        " , (h.valid_to AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//history 8
        " , obj.clid, clr.handle "//sponsoring registrar 9-10
        " , cobr.crid, crr.handle "//creating registrar 11-12
        " , obj.upid, upr.handle "//last updated by registrar 13-14
        " , (cobr.crdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 15
        " , (obj.trdate AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 16
        " , (obj.update AT TIME ZONE 'UTC') AT TIME ZONE $1::text "//registration dates 17
        " , obj.authinfopw "//transfer passwd 18
        " , cobr.crhistoryid "//first historyid 19
        " , h.request_id "//logd request_id 20
        " , ct.name, ct.organization, ct.street1, ct.street2, ct.street3, ct.city, ct.stateorprovince, ct.postalcode, ct.country "//contact data
        " , ct.telephone, ct.fax, ct.email , ct.notifyemail, ct.vat, ct.ssn, est.type "
        " , ct.disclosename, ct.discloseorganization, ct.discloseaddress, ct.disclosetelephone, ct.disclosefax "
        " , ct.discloseemail, ct.disclosevat, ct.discloseident, ct.disclosenotifyemail "
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC')::timestamp AS utc_timestamp "// utc timestamp 46
        " , (CURRENT_TIMESTAMP AT TIME ZONE 'UTC' AT TIME ZONE $1::text)::timestamp AS local_timestamp  "// local zone timestamp 47
        " FROM object_registry cobr ";
        if(history_query_)
        {
            sql << " JOIN object_history obj ON obj.id = cobr.id "
            " JOIN contact_history ct ON ct.historyid = obj.historyid "
            " JOIN history h ON h.id = ct.historyid ";
        }
        else
        {
            sql << " JOIN object obj ON obj.id = cobr.id "
            " JOIN contact ct ON ct.id = obj.id "
            " JOIN history h ON h.id = cobr.historyid ";
        }
        sql << " JOIN registrar clr ON clr.id = obj.clid "
        " JOIN registrar crr ON crr.id = cobr.crid "
        " LEFT JOIN registrar upr ON upr.id = obj.upid "
        " LEFT JOIN enum_ssntype est ON est.id = ct.ssntype "
        " WHERE "
        " cobr.type = (SELECT id FROM enum_object_type eot WHERE eot.name='contact'::text) ";


        if(contact_handle_.isset())
        {
            params.push_back(contact_handle_);
            sql << " AND cobr.name = $"<< params.size() <<"::text ";
        }


        if(contact_roid_.isset())
        {
            params.push_back(contact_roid_);
            sql << " AND cobr.roid = $"<< params.size() <<"::text ";
        }

        if(contact_id_.isset())
        {
            params.push_back(contact_id_);
            sql << " AND cobr.id = $"<< params.size() <<"::bigint ";
        }

        if(contact_historyid_.isset())
        {
            params.push_back(contact_historyid_);
            sql << " AND h.id = $"<< params.size() <<"::bigint ";
        }

        if(history_timestamp_.isset())
        {
            params.push_back(history_timestamp_);
            sql << " AND h.valid_from <= ($"<< params.size() <<"::timestamp AT TIME ZONE $1::text) AT TIME ZONE 'UTC' "
            " AND ($"<< params.size() <<"::timestamp AT TIME ZONE $1::text) AT TIME ZONE 'UTC' < h.valid_to ";
        }

        sql << " ORDER BY h.id DESC ";

        if(lock_)
        {
            sql << " FOR UPDATE of cobr ";
        }

        Database::Result query_result = ctx.get_conn().exec_params(sql.str(),params);
        result.reserve(query_result.size());//alloc
        for(Database::Result::size_type i = 0; i < query_result.size(); ++i)
        {
            InfoContactOutput info_contact_output;

            info_contact_output.info_contact_data.id = static_cast<unsigned long long>(query_result[i][0]);//cobr.id
            info_contact_output.info_contact_data.roid = static_cast<std::string>(query_result[i][1]);//cobr.roid

            info_contact_output.info_contact_data.handle = static_cast<std::string>(query_result[i][2]);//cobr.name

            info_contact_output.info_contact_data.delete_time = query_result[i][3].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][3])));//cobr.erdate

            info_contact_output.info_contact_data.historyid = static_cast<unsigned long long>(query_result[i][5]);//h.id

            info_contact_output.next_historyid = query_result[i][6].isnull() ? Nullable<unsigned long long>()
            : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][6]));//h.next

            info_contact_output.history_valid_from = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][7]));//h.valid_from

            info_contact_output.history_valid_to = query_result[i][8].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][8])));//h.valid_to

            info_contact_output.info_contact_data.sponsoring_registrar_handle = static_cast<std::string>(query_result[i][10]);//clr.handle

            info_contact_output.info_contact_data.create_registrar_handle = static_cast<std::string>(query_result[i][12]);//crr.handle

            info_contact_output.info_contact_data.update_registrar_handle = query_result[i][14].isnull() ? Nullable<std::string>()
            : Nullable<std::string> (static_cast<std::string>(query_result[i][14]));//upr.handle

            info_contact_output.info_contact_data.creation_time = boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][15]));//cobr.crdate

            info_contact_output.info_contact_data.transfer_time = query_result[i][16].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][16])));//obj.trdate

            info_contact_output.info_contact_data.update_time = query_result[i][17].isnull() ? Nullable<boost::posix_time::ptime>()
            : Nullable<boost::posix_time::ptime>(boost::posix_time::time_from_string(static_cast<std::string>(query_result[i][17])));//obj.update

            info_contact_output.info_contact_data.authinfopw = static_cast<std::string>(query_result[i][18]);//obj.authinfopw

            info_contact_output.info_contact_data.crhistoryid = static_cast<unsigned long long>(query_result[i][19]);//cobr.crhistoryid

            info_contact_output.logd_request_id = query_result[i][20].isnull() ? Nullable<unsigned long long>()
                : Nullable<unsigned long long>(static_cast<unsigned long long>(query_result[i][20]));

            info_contact_output.info_contact_data.name = query_result[i][21].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][21]));
            info_contact_output.info_contact_data.organization = query_result[i][22].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][22]));
            info_contact_output.info_contact_data.street1 = query_result[i][23].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][23]));
            info_contact_output.info_contact_data.street2 = query_result[i][24].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][24]));
            info_contact_output.info_contact_data.street3 = query_result[i][25].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][25]));
            info_contact_output.info_contact_data.city = query_result[i][26].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][26]));
            info_contact_output.info_contact_data.stateorprovince = query_result[i][27].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][27]));
            info_contact_output.info_contact_data.postalcode = query_result[i][28].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][28]));
            info_contact_output.info_contact_data.country = query_result[i][29].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][29]));
            info_contact_output.info_contact_data.telephone = query_result[i][30].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][30]));
            info_contact_output.info_contact_data.fax = query_result[i][31].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][31]));
            info_contact_output.info_contact_data.email = query_result[i][32].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][32]));
            info_contact_output.info_contact_data.notifyemail = query_result[i][33].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][33]));
            info_contact_output.info_contact_data.vat = query_result[i][34].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][34]));
            info_contact_output.info_contact_data.ssn = query_result[i][35].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][35]));
            info_contact_output.info_contact_data.ssntype = query_result[i][36].isnull() ? Nullable<std::string>()
                    : Nullable<std::string> (static_cast<std::string>(query_result[i][36]));
            info_contact_output.info_contact_data.disclosename = query_result[i][37].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][37]));
            info_contact_output.info_contact_data.discloseorganization = query_result[i][38].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][38]));
            info_contact_output.info_contact_data.discloseaddress = query_result[i][39].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][39]));
            info_contact_output.info_contact_data.disclosetelephone = query_result[i][40].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][40]));
            info_contact_output.info_contact_data.disclosefax = query_result[i][41].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][41]));
            info_contact_output.info_contact_data.discloseemail = query_result[i][42].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][42]));
            info_contact_output.info_contact_data.disclosevat = query_result[i][43].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][43]));
            info_contact_output.info_contact_data.discloseident = query_result[i][44].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][44]));
            info_contact_output.info_contact_data.disclosenotifyemail = query_result[i][45].isnull() ? Nullable<bool>()
                    : Nullable<bool> (static_cast<bool>(query_result[i][45]));

            info_contact_output.utc_timestamp = query_result[i][0].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[0][46]));// utc timestamp
            info_contact_output.local_timestamp = query_result[i][1].isnull() ? boost::posix_time::ptime(boost::date_time::not_a_date_time)
            : boost::posix_time::time_from_string(static_cast<std::string>(query_result[0][47]));//local zone timestamp

            result.push_back(info_contact_output);
        }//for res

        return result;
    }




}//namespace Fred

