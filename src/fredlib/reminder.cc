#include <map>
#include <string>
#include <vector>
#include <boost/assign/list_of.hpp>

#include "reminder.h"
#include "db_settings.h"

#include "log/logger.h"
#include "log/context.h"


using namespace boost::assign;


std::map<std::string, std::string> map_from_db_row(const Database::Row &_row,
                                                   const std::vector<std::string> &_keys)
{
    std::map<std::string, std::string> data;
    for (std::vector<std::string>::const_iterator key = _keys.begin();
            key != _keys.end();
            ++key)
    {
        data[*key] = static_cast<std::string>(_row[*key]);
    }
    return data;
}


namespace Fred {


class ReminderEmail
{
public:
    typedef std::map<std::string, std::string> ParametersMap;


    ReminderEmail(const Database::Row &_contact_data,
                  const Database::Row &_linked_data)
    {
        /* check if results have all necessary data */
        contact_id_ = static_cast<unsigned long long>(_contact_data["id"]);
        ParametersMap tmp1 = map_from_db_row(_contact_data,
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
        ParametersMap tmp2 = map_from_db_row(_linked_data,
                list_of("domains")
                       ("nssets")
                       ("keysets")
                       ("domains_owner")
                       ("domains_admin_c"));
        email_params_.insert(tmp1.begin(), tmp1.end());
        email_params_.insert(tmp2.begin(), tmp2.end());
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
                " array_accum(DISTINCT doreg.name) AS domains_admin_c,"
                " array_accum(DISTINCT doreg2.name) AS domains_owner,"
                " array(SELECT DISTINCT (array_accum(DISTINCT doreg.name)||array_accum(DISTINCT doreg2.name))[i]"
                " FROM generate_series("
                " array_lower((array_accum(DISTINCT doreg.name)||array_accum(DISTINCT doreg2.name)), 1),"
                " array_upper((array_accum(DISTINCT doreg.name)||array_accum(DISTINCT doreg2.name)), 1)) g(i)"
                " ) AS domains,"
                " array_accum(DISTINCT noreg.name) AS nssets,"
                " array_accum(DISTINCT koreg.name) AS keysets"
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
        if (index_ > contact_data_.size()) {
            throw StopIteration();
        }

        if (static_cast<unsigned long long >(contact_data_[index_]["id"])
                != static_cast<unsigned long long>(linked_data_[index_]["id"])) {
            throw std::runtime_error("intergrity error: row data mismatch!");
        }

        ReminderEmail next = ReminderEmail(contact_data_[index_], linked_data_[index_]);
        index_ += 1;
        return next;
    }
};


void run_reminder(const boost::gregorian::date &_date)
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
            ReminderEmail email = rem.next();
            ReminderEmail::ParametersMap params = email.get_template_params();

            LOGGER(PACKAGE).info(boost::format("processing %1% (crdate: %2%) %3%")
                        % params["roid"] % params["crdate"] % params["handle"]);
            LOGGER(PACKAGE).debug(boost::format(
                        "%1% data ((owner: %2%, admin-c: %3%) => domains: %4%, nssets: %5%, keysets: %6%)")
                        % params["handle"]
                        % params["domains_owner"]
                        % params["domains_admin_c"]
                        % params["domains"]
                        % params["nssets"]
                        % params["keysets"]);
        }
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(_ex.what());
        /* TODO: error handling */
    }
}


}


