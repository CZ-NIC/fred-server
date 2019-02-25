/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/reminder.hh"
#include "src/deprecated/libfred/db_settings.hh"
#include "libfred/db_settings.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/util/file_lock.hh"

#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/trim.hpp>

#include <map>
#include <string>
#include <vector>

using namespace boost::assign;


namespace LibFred {


void row_map_copy(std::map<std::string, std::string> &_dest,
                  const Database::Row &_row,
                  const std::vector<std::string> &_keys)
{
    for (std::vector<std::string>::const_iterator key = _keys.begin();
            key != _keys.end();
            ++key)
    {
        if (_row[*key].isnull()) {
            _dest[*key] = std::string("");
        }
        else {
            _dest[*key] = static_cast<std::string>(_row[*key]);
        }
    }
}

void row_map_copy_single(std::map<std::string, std::string> &_dest,
        const Database::Row &_row, const std::string &_key)
{

    if (_row[_key].isnull()) {
        _dest[_key] = std::string("");
    } else {
        _dest[_key] = static_cast<std::string> (_row[_key]);
    }
}

void insert_parameter_list(std::map<std::string, std::string> &_params,
                             const std::string &_base_key,
                             const std::vector<std::string> &_list)
{
    for (std::vector<std::string>::size_type i = 0; i < _list.size(); ++i)
    {
        _params.insert(std::make_pair(
                       _base_key + "." + boost::lexical_cast<std::string>(i),
                       _list[i]));
    }
}

class ReminderEmail
{
public:
    struct IntegrityError : public std::runtime_error
    {
        IntegrityError(const std::string &_err)
            : std::runtime_error("integrity_error: " + _err) { }
    };

    typedef std::map<std::string, std::string> ParametersMap;


    ReminderEmail(const Database::Row &_contact_data,
                  const Database::Row &_linked_data_domain,
                  const Database::Row &_linked_data_nsset,
                  const Database::Row &_linked_data_keyset)
    {
        /* check if results have all necessary data */
        if (static_cast<unsigned long long>(_contact_data["id"])
                != static_cast<unsigned long long>(_linked_data_domain["id"])
            || static_cast<unsigned long long>(_contact_data["id"])
                != static_cast<unsigned long long>(_linked_data_nsset["id"])
            || static_cast<unsigned long long>(_contact_data["id"])
                != static_cast<unsigned long long>(_linked_data_keyset["id"]))
        {
            throw IntegrityError("rows contact id mismatch!");
        }
        contact_id_ = static_cast<unsigned long long>(_contact_data["id"]);

        row_map_copy(email_params_, _contact_data,
                list_of("roid")
                       ("crdate")
                       ("handle")
                       ("registrar")
                       ("registrar_name")
                       ("registrar_url")
                       ("registrar_memo_cz")
                       ("registrar_memo_en")
                       ("registrar_reply_to")
                       ("name")
                       ("address")
                       ("organization")
                       ("ident_type")
                       ("ident_value")
                       ("dic")
                       ("telephone")
                       ("fax")
                       ("email")
                       ("notify_email"));

        // email_params_["arr_domains"] direct TODO
        row_map_copy_single(email_params_, _linked_data_domain, "arr_domains");
        row_map_copy_single(email_params_, _linked_data_nsset, "arr_nssets");
        row_map_copy_single(email_params_, _linked_data_keyset, "arr_keysets");

        /* transform objects arrays to clearsilver list */
        insert_parameter_list(email_params_, "domains", Database::array_to_vector(email_params_["arr_domains"]));
        insert_parameter_list(email_params_, "nssets",  Database::array_to_vector(email_params_["arr_nssets"]));
        insert_parameter_list(email_params_, "keysets", Database::array_to_vector(email_params_["arr_keysets"]));
   }


    std::string get_template() const
    {
        return "annual_contact_reminder";
    }


    unsigned long long get_contact_id() const
    {
        return contact_id_;
    }


    const ParametersMap& get_template_params() const
    {
        return email_params_;
    }


    void save_relation(const boost::gregorian::date &_date,
                       const unsigned long long &_msg_id) const
    {
        if (_date.is_not_a_date() || _msg_id == 0) {
            throw std::runtime_error("can't save reminder run info - invalid date or message id");
        }
        Database::Connection conn = Database::Manager::acquire();
        conn.exec_params("INSERT INTO reminder_contact_message_map"
                " (reminder_date, contact_id, message_id) VALUES"
                " ($1::date, $2::bigint, $3::bigint)",
                Database::query_param_list(_date)(contact_id_)(_msg_id));
    }


private:
    unsigned long long contact_id_;
    ParametersMap email_params_;
};


class ReminderEmailGenerator
{
private:

    Database::Connection conn_;
    Database::Result::size_type index_;
    Database::Result dummy1_;
    Database::Result dummy2_;
    Database::Result contact_data_;
    Database::Result linked_data_domain_;
    Database::Result linked_data_nsset_;
    Database::Result linked_data_keyset_;



public:

    struct StopIteration : public std::runtime_error
    {
        StopIteration() : std::runtime_error("stop iteration") { }
    };


    ReminderEmailGenerator(const boost::gregorian::date _date)
        : conn_(Database::Manager::acquire()),
          index_(0),
          dummy1_(conn_.exec(
                "CREATE TEMPORARY TABLE tmp_reminder"
                " (contact_id bigint, contact_crdate timestamp without time zone)")),
          dummy2_(conn_.exec_params(
                "INSERT INTO tmp_reminder"
                " SELECT coreg.id, coreg.crdate"
                " FROM object_registry coreg"
                " JOIN object o ON o.id = coreg.id"
                " JOIN object_state os ON os.object_id = coreg.id"
                " JOIN enum_object_states eos ON eos.id = os.state_id"
                " LEFT JOIN reminder_contact_message_map rcmm ON rcmm.contact_id = coreg.id AND rcmm.reminder_date = $1::date"
                " WHERE coreg.type = 1 AND coreg.erdate IS NULL"
                " AND eos.name = 'linked'"
                " AND os.valid_from < current_timestamp AND (os.valid_to > current_timestamp OR os.valid_to is null)"
                " AND extract('month' FROM ($1::date - interval '300 day')) = extract('month' FROM coreg.crdate)"
                " AND extract('day' FROM ($1::date - interval '300 day')) = extract('day' FROM coreg.crdate)"
                " AND extract('year' FROM ($1::date - interval '300 day')) >= extract('year' FROM coreg.crdate)"
                " AND (o.update IS NULL OR o.update::date NOT BETWEEN ($1::date - interval '2 month') AND $1::date)"
                " AND rcmm.contact_id IS NULL"
                " ORDER BY crdate",
                Database::query_param_list(_date))),
          contact_data_(conn_.exec(
                "SELECT tmp.contact_id AS id,"
                " coreg.roid AS roid,"
                " coreg.name AS handle,"
                " coreg.crdate AS crdate,"
                " r.handle AS registrar,"
                " r.name AS registrar_name,"
                " r.url AS registrar_url,"
                " split_part(rrp.template_memo, '~@~', 1) AS registrar_memo_cz,"
                " split_part(rrp.template_memo, '~@~', 2) AS registrar_memo_en,"
                " rrp.reply_to AS registrar_reply_to,"
                " c.name AS name,"
                " c.street1 || ', ' || c.city || ', ' || c.postalcode as address,"
                " c.organization AS organization,"
                " est.type AS ident_type,"
                " c.ssn AS ident_value,"
                " c.vat AS dic,"
                " c.telephone AS telephone,"
                " c.fax AS fax,"
                " c.email AS email,"
                " c.notifyemail AS notify_email"
                " FROM tmp_reminder tmp"
                " LEFT JOIN (object_registry coreg"
                " JOIN object co ON co.id = coreg.id"
                " JOIN registrar r ON r.id = co.clid"
                " LEFT JOIN reminder_registrar_parameter rrp ON rrp.registrar_id = r.id"
                " JOIN contact c ON c.id = coreg.id"
                " LEFT JOIN enum_ssntype est ON est.id = c.ssntype)"
                " ON coreg.id = tmp.contact_id"
                " ORDER BY tmp.contact_id")),
          linked_data_domain_(conn_.exec(
                  " SELECT t.contact_id AS id, array_filter_null(array_agg(t.name)) AS arr_domains"
                   " FROM ("
                       " SELECT tmp.contact_id, oreg.name"
                       " FROM tmp_reminder tmp"
                       " LEFT JOIN domain_contact_map dcm ON dcm.contactid = tmp.contact_id"
                       " LEFT JOIN object_registry oreg ON oreg.id = dcm.domainid"
                   " UNION"
                       " SELECT tmp.contact_id, oreg.name"
                       " FROM tmp_reminder tmp"
                       " LEFT JOIN domain d ON d.registrant = tmp.contact_id"
                       " LEFT JOIN object_registry oreg ON oreg.id = d.id"
                   " ) t"
                   " GROUP BY t.contact_id"
                   " ORDER BY t.contact_id")),
          linked_data_nsset_(conn_.exec(
                  " SELECT tmp.contact_id AS id, "
                   " array_filter_null(array_agg(DISTINCT noreg.name)) AS arr_nssets"
                   " FROM tmp_reminder tmp"
                   " LEFT JOIN nsset_contact_map ncm ON ncm.contactid = tmp.contact_id"
                   " LEFT JOIN object_registry noreg ON noreg.id = ncm.nssetid"
                   " GROUP BY tmp.contact_id"
                   " ORDER BY tmp.contact_id")),
          linked_data_keyset_(conn_.exec(
                  "SELECT tmp.contact_id AS id, "
                  " array_filter_null(array_agg(DISTINCT koreg.name)) AS arr_keysets "
                  " FROM tmp_reminder tmp "
                  " LEFT JOIN keyset_contact_map kcm ON kcm.contactid = tmp.contact_id "
                  " LEFT JOIN object_registry koreg ON koreg.id = kcm.keysetid"
                  " GROUP BY tmp.contact_id"
                  " ORDER BY tmp.contact_id"))
    {
        if (contact_data_.size() != linked_data_domain_.size()
             && contact_data_.size() != linked_data_nsset_.size()
             && contact_data_.size() != linked_data_keyset_.size()
        ) {
            throw std::runtime_error("selected size of contact data and linked object mismatch!");
        }
    }


    ~ReminderEmailGenerator()
    {
        /* don't let destructor throw */
        try {
            conn_.exec("DROP TABLE tmp_reminder");
        }
        catch (...) {
            /* try to log error */
            try {
                LOGGER.error("unable to drop temporary table (tmp_reminder)");
            }
            catch (...) {
            }
        }
    }


    bool is_next() const
    {
        return (index_ < contact_data_.size());
    }


    ReminderEmail next()
    {
        unsigned long long lindex = index_;
        index_ += 1;

        return ReminderEmail(contact_data_[lindex], linked_data_domain_[lindex], linked_data_nsset_[lindex], linked_data_keyset_[lindex]);
    }
};


void run_reminder(Mailer::Manager *_mailer, const boost::gregorian::date &_date)
{
    try {
        Logging::Context ctx("reminder");

        if (_date.is_not_a_date()) {
            throw std::runtime_error("date is not valid");
        }

        if (_date > boost::gregorian::day_clock::universal_day()) {
            throw std::runtime_error("running reminder with date in future is not allowed");
        }

        /* TODO: consider using boost::interprocess::file_lock */
        FileLock lock("/tmp/fred_reminder_run.lock");

        LOGGER.info(boost::format("reminder run date: %1%")
                % boost::gregorian::to_iso_extended_string(_date));

        ReminderEmailGenerator rem = ReminderEmailGenerator(_date);
        while (rem.is_next()) {
            try {
                ReminderEmail email = rem.next();

                ReminderEmail::ParametersMap params = email.get_template_params();
                LOGGER.info(boost::format("processing %1% (crdate: %2%) %3%")
                            % params["roid"] % params["crdate"] % params["handle"]);
                LOGGER.debug(boost::format(
                            "%1% data (domains: %2%, nssets: %3%, keysets: %4%)")
                            % params["handle"]
                            % params["arr_domains"]
                            % params["arr_nssets"]
                            % params["arr_keysets"]);

                Mailer::Handles handles;
                Mailer::Attachments attach;
                std::string email_addr = boost::algorithm::trim_copy(params["email"]);
                std::string email_registrar_reply_to = boost::algorithm::trim_copy(params["registrar_reply_to"]);
                if (email_addr.empty()) {
                    throw std::runtime_error(str(boost::format("no email address (contact_id=%1%)")
                                             % email.get_contact_id()));
                }
                unsigned long long mid = _mailer->sendEmail("", email_addr, "", email.get_template(),
                                                            params, handles, attach, email_registrar_reply_to);
                try {
                    email.save_relation(_date, mid);
                }
                catch (...) {
                    LOGGER.error(boost::format(
                                "reminder run info save failure (contact_id=%1%  msg_id=%2%)")
                                % email.get_contact_id() % mid);
                }
            }
            catch (ReminderEmail::IntegrityError &_ie) {
                /* stop on integrity error */
                throw;
            }
            catch (std::exception &_ex) {
                /* do not fail after one contact error */
                LOGGER.error(boost::format("problem occured (%1%)") % _ex.what());
            }
        }
    }
    catch (std::exception &_ex) {
        throw ReminderError(_ex.what());
    }
    catch (...) {
        throw ReminderError("unknown error");
    }
}


}


