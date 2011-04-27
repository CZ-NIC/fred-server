#include <map>
#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>
#include <boost/lexical_cast.hpp>

#include "reminder.h"
#include "db_settings.h"
#include "log/logger.h"
#include "log/context.h"


using namespace boost::assign;


namespace Fred {


void row_map_copy(std::map<std::string, std::string> &_dest,
                  const Database::Row &_row,
                  const std::vector<std::string> &_keys)
{
    for (std::vector<std::string>::const_iterator key = _keys.begin();
            key != _keys.end();
            ++key)
    {
        _dest[*key] = static_cast<std::string>(_row[*key]);
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
                  const Database::Row &_linked_data)
    {
        /* check if results have all necessary data */
        if (static_cast<unsigned long long>(_contact_data["id"])
                != static_cast<unsigned long long>(_linked_data["id"]))
        {
            throw IntegrityError("rows contact id mismatch!");
        }
        contact_id_ = static_cast<unsigned long long>(_contact_data["id"]);

        row_map_copy(email_params_, _contact_data,
                list_of("roid")
                       ("crdate")
                       ("handle")
                       ("registrar")
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

        row_map_copy(email_params_, _linked_data,
                list_of("arr_domains")
                       ("arr_nssets")
                       ("arr_keysets")
                       ("arr_domains_owner")
                       ("arr_domains_admin_c"));

        /* transform objects arrays to clearsilver list */
        insert_parameter_list(email_params_, "domains", Database::array_to_vector(email_params_["arr_domains"]));
        insert_parameter_list(email_params_, "nssets",  Database::array_to_vector(email_params_["arr_nssets"]));
        insert_parameter_list(email_params_, "keysets", Database::array_to_vector(email_params_["arr_keysets"]));
   }


    std::string get_template() const
    {
        return "annual_contact_reminder";
    }


    const ParametersMap& get_template_params() const
    {
        return email_params_;
    }


    void save_relation(boost::gregorian::date &_date,
                       const unsigned long long &_msg_id) const
    {
        Database::Connection conn = Database::Manager::acquire();
        conn.exec_params("INSERT INTO contact_reminder_message_map"
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
    Database::Result linked_data_;



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
          /* TODO: join with already notified on given date to prevert multiple notification */
          dummy2_(conn_.exec_params(
                "INSERT INTO tmp_reminder"
                " SELECT coreg.id, coreg.crdate"
                " FROM object_registry coreg"
                " WHERE coreg.type = 1 AND coreg.erdate IS NULL"
                " AND extract('month' FROM ($1::date - interval '10 month')) = extract('month' FROM coreg.crdate)"
                " AND extract('day' FROM ($1::date - interval '10 month')) = extract('day' FROM coreg.crdate)"
                " ORDER BY crdate",
                Database::query_param_list(_date))),
          contact_data_(conn_.exec(
                "SELECT tmp.contact_id AS id,"
                " coreg.roid AS roid,"
                " coreg.name AS handle,"
                " coreg.crdate AS crdate,"
                " r.handle AS registrar,"
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
                " JOIN contact c ON c.id = coreg.id"
                " LEFT JOIN enum_ssntype est ON est.id = c.ssntype)"
                " ON coreg.id = tmp.contact_id"
                " ORDER BY tmp.contact_id")),
          linked_data_(conn_.exec(
                "SELECT tmp.contact_id AS id,"
                " array_accum(DISTINCT doreg.name) AS arr_domains_admin_c,"
                " array_accum(DISTINCT doreg2.name) AS arr_domains_owner,"
                " array(SELECT DISTINCT (array_accum(DISTINCT doreg.name)||array_accum(DISTINCT doreg2.name))[i]"
                " FROM generate_series("
                " array_lower((array_accum(DISTINCT doreg.name)||array_accum(DISTINCT doreg2.name)), 1),"
                " array_upper((array_accum(DISTINCT doreg.name)||array_accum(DISTINCT doreg2.name)), 1)) g(i)"
                " ) AS arr_domains,"
                " array_accum(DISTINCT noreg.name) AS arr_nssets,"
                " array_accum(DISTINCT koreg.name) AS arr_keysets"
                " FROM tmp_reminder tmp"
                " LEFT JOIN (domain_contact_map dcm"
                " JOIN object_registry doreg ON doreg.id = dcm.domainid"
                " ) ON dcm.contactid = tmp.contact_id"
                " LEFT JOIN (domain d"
                " JOIN object_registry doreg2 ON doreg2.id = d.id"
                " ) ON d.registrant = tmp.contact_id"
                " LEFT JOIN (nsset_contact_map ncm"
                " JOIN object_registry noreg ON noreg.id = ncm.nssetid"
                " ) ON ncm.contactid = tmp.contact_id"
                " LEFT JOIN (keyset_contact_map kcm"
                " JOIN object_registry koreg ON koreg.id = kcm.keysetid"
                " ) ON kcm.contactid = tmp.contact_id"
                " GROUP BY tmp.contact_id"))
    {
        if (contact_data_.size() != linked_data_.size()) {
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
            LOGGER(PACKAGE).error("unable to drop temporary table (tmp_reminder)");
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

        return ReminderEmail(contact_data_[lindex], linked_data_[lindex]);
    }
};


void run_reminder(Mailer::Manager *_mailer, const boost::gregorian::date &_date)
{
    try {
        Logging::Context ctx("reminder");

        if (_date.is_not_a_date()) {
            throw std::runtime_error("date is not valid");
        }
        LOGGER(PACKAGE).info(boost::format("reminder run date: %1%")
                % boost::gregorian::to_iso_extended_string(_date));

        ReminderEmailGenerator rem = ReminderEmailGenerator(_date);
        while (rem.is_next()) {
            try {
                ReminderEmail email = rem.next();

                ReminderEmail::ParametersMap params = email.get_template_params();
                LOGGER(PACKAGE).info(boost::format("processing %1% (crdate: %2%) %3%")
                            % params["roid"] % params["crdate"] % params["handle"]);
                LOGGER(PACKAGE).debug(boost::format(
                            "%1% data ((owner: %2%, admin-c: %3%) => domains: %4%, nssets: %5%, keysets: %6%)")
                            % params["handle"]
                            % params["arr_domains_owner"]
                            % params["arr_domains_admin_c"]
                            % params["arr_domains"]
                            % params["arr_nssets"]
                            % params["arr_keysets"]);

                Mailer::Handles handles;
                Mailer::Attachments attach;
                _mailer->sendEmail("", params["email"], "", email.get_template(), params, handles, attach);
            }
            catch (ReminderEmail::IntegrityError &_ie) {
                /* stop on integrity error */
                throw;
            }
            catch (std::exception &_ex) {
                /* do not fail after one contact error */
                LOGGER(PACKAGE).error(boost::format("problem occured (%1%)") % _ex.what());
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


