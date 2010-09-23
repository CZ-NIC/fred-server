#include "mojeid_impl.h"
#include "corba_wrap.h"
#include "mojeid_request.h"

#include "log/logger.h"
#include "log/context.h"

#include "register/db_settings.h"
#include "register/contact.h"
#include "corba/connection_releaser.h"

#include <stdexcept>
#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>



boost::mt19937 gen;

const std::string create_ctx_name(const std::string &_name)
{
    boost::uniform_int<unsigned int> ruint(1, 10000);
    boost::variate_generator<boost::mt19937&, boost::uniform_int<unsigned int> > gen_uint(gen, ruint);

    return str(boost::format("%1%-<%2%>") % _name % gen_uint());
}


namespace Registry {


MojeIDImpl::MojeIDImpl(const std::string &_server_name)
    : server_name_(_server_name),
      mojeid_registrar_id_(0)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx("init");

    try {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result result = conn.exec_params(
                "SELECT id FROM registrar WHERE handle = $1::text",
                Database::query_param_list("REG-MOJEID"));

        if (result.size() != 1 || (mojeid_registrar_id_ = result[0][0]) == 0) {
            throw std::runtime_error("failed to find dedicated registrar in database");
        }
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).alert(_ex.what());
        throw;
    }
}


MojeIDImpl::~MojeIDImpl()
{
}


CORBA::ULongLong MojeIDImpl::contactCreate(const Contact &_contact,
                                           IdentificationMethod _method,
                                           const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-create");
    ConnectionReleaser releaser;

    try {
        std::string handle = static_cast<std::string>(_contact.username);
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  identification_method: %2%  request_id: %3%")
                % handle % _method % _request_id);

        /* start new request - here for logging into action table - until 
         * fred-logd fully migrated */
        Registry::MojeIDRequest request(204);
        Logging::Context ctx_request(request.get_servertrid());

        try {
            /* TODO: 2nd parameter should be from configuration */
            Register::Contact::ManagerPtr contact_mgr(Register::Contact::Manager::create(0, false));
            LOGGER(PACKAGE).debug(boost::format("handle '%1%' availability check")
                    % handle);

            Register::NameIdPair cinfo;
            Register::Contact::Manager::CheckAvailType check_result;
            check_result = contact_mgr->checkAvail(handle, cinfo);

            if (check_result == Register::Contact::Manager::CA_INVALID_HANDLE) {
                throw std::runtime_error(str(boost::format("handle '%1%' is not valid")
                            % handle));
            }
            else if (check_result == Register::Contact::Manager::CA_REGISTRED) {
                throw std::runtime_error(str(boost::format("handle '%1%' is already registered")
                            % handle));
            }
            else if (check_result == Register::Contact::Manager::CA_PROTECTED) {
                throw std::runtime_error(str(boost::format("handle '%1%' is in protection period")
                        % handle));
            }
            LOGGER(PACKAGE).debug(boost::format("handle '%1%' check passed")
                    % handle);
        }
        catch (std::exception &_ex) {
            throw;
        }
        catch (...) {
            throw std::runtime_error("contact handle availability check failed");
        }

        if (_contact.addresses.length() == 0) {
            throw std::runtime_error("contact has no address");
        }
        if (_contact.emails.length() == 0) {
            throw std::runtime_error("contact has no email");
        }


        /* object_registry record */
        Database::Result roreg = request.conn.exec_params(
                "SELECT create_object($1::integer, $2::text, $3::integer)",
                Database::query_param_list(mojeid_registrar_id_)(handle)(1));
        if (roreg.size() != 1) {
            throw std::runtime_error("create object failed");
        }
        unsigned long long id = static_cast<unsigned long long>(roreg[0][0]);

        /* object record */
        Database::Result robject = request.conn.exec_params(
                "INSERT INTO object (id, clid, authinfopw) VALUES ($1::integer, $2::integer,"
                    " $3::text)",
                Database::query_param_list(id)(mojeid_registrar_id_)("TODO: random string"));

        std::string qcontact = "INSERT INTO contact ("
                                    "id, name, organization, street1, street2, street3,"
                                    " city, stateorprovince, postalcode, country, telephone,"
                                    " fax, email, notifyemail, vat, ssn, ssntype,"
                                    " disclosename, discloseorganization, discloseaddress,"
                                    " disclosetelephone, disclosefax, discloseemail,"
                                    " disclosenotifyemail, disclosevat, discloseident"
                               ") VALUES ("
                                    "$1::integer, $2::text, $3::text, $4::text, $5::text,"
                                    " $6::text, $7::text, $8::text, $9::text, $10::text,"
                                    " $11::text, $12::text, $13::text, $14::text, $15::text,"
                                    " $16::text, $17::integer, $18::boolean, $19::boolean,"
                                    " $20::boolean, $21::boolean, $22::boolean, $23::boolean,"
                                    " $24::boolean, $25::boolean, $26::boolean)";

        Database::QueryParam telephone = Database::QueryParam();
        Database::QueryParam fax = Database::QueryParam();
        for (unsigned int i = 0; i <  _contact.phones.length(); ++i) {
            std::string type = corba_unwrap_string(_contact.phones[i].type);
            if (type == "DEFAULT") {
                telephone = corba_unwrap_string(_contact.phones[i].number);
            }
            else if (type == "FAX") {
                fax = corba_unwrap_string(_contact.phones[i].number);
            }
        }

        Database::QueryParam email = Database::QueryParam();
        Database::QueryParam notify_email = Database::QueryParam();
        for (unsigned int i = 0; i < _contact.emails.length(); ++i) {
            std::string type = corba_unwrap_string(_contact.emails[i].type);
            if (type == "DEFAULT") {
                email = corba_unwrap_string(_contact.emails[i].email_address);
            }
            else if (type == "NOTIFY") {
                notify_email = corba_unwrap_string(_contact.emails[i].email_address);
            }
        }

        Database::QueryParam ssn = Database::QueryParam();
        Database::QueryParam ssn_type = Database::QueryParam();
        if (_contact.ssn_type) {
            int type = boost::lexical_cast<int>(_contact.ssn_type->_value());
            ssn_type = type;
            if (type == 2) ssn = corba_unwrap_nullable_string(_contact.id_card_num);
            if (type == 3) ssn = corba_unwrap_nullable_string(_contact.passport_num);
            if (type == 4) ssn = corba_unwrap_nullable_string(_contact.vat_id_num);
            if (type == 5) ssn = corba_unwrap_nullable_string(_contact.ssn_id_num);
            if (type == 6) ssn = corba_unwrap_nullable_date(_contact.birth_date);
        }

        Database::QueryParams pcontact = Database::query_param_list
                                    (id)
                                    (corba_unwrap_string(_contact.first_name)
                                        + " " + corba_unwrap_string(_contact.last_name))
                                    (corba_unwrap_nullable_string(_contact.organization))
                                    (corba_unwrap_string(_contact.addresses[0].street1))
                                    (corba_unwrap_nullable_string(_contact.addresses[0].street2))
                                    (corba_unwrap_nullable_string(_contact.addresses[0].street3))
                                    (corba_unwrap_string(_contact.addresses[0].city))
                                    (corba_unwrap_nullable_string(_contact.addresses[0].state))
                                    (corba_unwrap_string(_contact.addresses[0].postal_code))
                                    (corba_unwrap_string(_contact.addresses[0].country))
                                    (telephone)
                                    (fax)
                                    (email)
                                    (notify_email)
                                    (corba_unwrap_nullable_string(_contact.vat_reg_num))
                                    (ssn)
                                    (ssn_type)
                                    (corba_unwrap_nullable_boolean(_contact.disclose_name))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_organization))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_address))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_phone))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_fax))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_email))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_notify_email))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_vat))
                                    (corba_unwrap_nullable_boolean(_contact.disclose_ident));
        /* contact record */
        Database::Result rcontact = request.conn.exec_params(qcontact, pcontact);

        /* save history */
        request.conn.exec_params("INSERT INTO history (id, action) VALUES (DEFAULT, $1::integer)",
                                 Database::query_param_list(request.get_id()));
        Database::Result rhistory = request.conn.exec("SELECT currval('history_id_seq')");
        unsigned long long history_id = 0;
        history_id = rhistory[0][0];
        if (!history_id) {
            throw std::runtime_error("cannot save new history");
        }

        request.conn.exec_params("UPDATE object_registry SET crhistoryid = $1::integer,"
                " historyid = $2::integer WHERE id = $3::integer",
                Database::query_param_list(history_id)(history_id)(id));

        Database::Result robject_history = request.conn.exec_params(
                "INSERT INTO object_history (historyid, id, clid, upid, trdate, update, authinfopw)"
                " SELECT $1::integer, o.id, o.clid, o.upid, o.trdate, o.update, o.authinfopw FROM object o"
                " WHERE o.id = $2::integer",
                Database::query_param_list(history_id)(id));

        Database::Result rcontact_history = request.conn.exec_params(
                "INSERT INTO contact_history (historyid, id, name, organization, street1, street2, street3,"
                " city, stateorprovince, postalcode, country, telephone, fax, email, disclosename,"
                " discloseorganization, discloseaddress, disclosetelephone, disclosefax, discloseemail,"
                " notifyemail, vat, ssn, ssntype, disclosevat, discloseident, disclosenotifyemail)"
                " SELECT $1::integer, c.id, c.name, c.organization, c.street1, c.street2, c.street3,"
                " c.city, c.stateorprovince, c.postalcode, c.country, c.telephone, c.fax, c.email,"
                " c.disclosename, c.discloseorganization, c.discloseaddress, c.disclosetelephone, c.disclosefax,"
                " c.discloseemail, c.notifyemail, c.vat, c.ssn, c.ssntype, c.disclosevat, c.discloseident,"
                " c.disclosenotifyemail FROM contact c WHERE c.id = $2::integer",
                Database::query_param_list(history_id)(id));

        request.end_success();
        LOGGER(PACKAGE).info("request completed successfully");
        return id;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::ErrorReport();
    }
}


void MojeIDImpl::processIdentification(const char* _ident_request_id,
                                       const char* _password,
                                       const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("process-identification");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  identification_id: %1%  password: %2%  request_id: %3%")
                % _ident_request_id % _password % _request_id);

        throw std::runtime_error("not implemented");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::ErrorReport();
    }

}


void MojeIDImpl::contactUpdatePrepare(const Contact &_contact,
                                      const char* _trans_id,
                                      const CORBA::ULongLong _request_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-update");

    try {
        LOGGER(PACKAGE).info(boost::format("request data --"
                    "  handle: %1%  transaction_id: %2%"
                    "  request_id: %3%")  % static_cast<std::string>(_contact.username)
                    % _trans_id % _request_id);

        throw std::runtime_error("not implemented");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::ErrorReport();
    }

}


Contact* MojeIDImpl::contactInfo(const CORBA::ULongLong _id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("contact-info");
    ConnectionReleaser releaser;

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- id: %1%") % _id);
        Database::Connection conn = Database::Manager::acquire();

        std::string qinfo = "SELECT oreg.id, oreg.name,"
            " SPLIT_PART(c.name, ' ', 1) as first_name,"
            " SPLIT_PART(c.name, ' ', 2) as last_name,"
            " c.organization, c.vat, c.ssntype, c.ssn,"
            " c.disclosename, c.discloseorganization,"
            " c.disclosevat, c.discloseident,"
            " c.discloseemail, c.disclosenotifyemail,"
            " c.discloseaddress, c.disclosetelephone,"
            " c.disclosefax,"
            " c.street1, c.street2, c.street3,"
            " c.city, c.stateorprovince, c.postalcode, c.country,"
            " c.email, c.notifyemail, c.telephone, c.fax"
            " FROM object_registry oreg JOIN contact c ON c.id = oreg.id"
            " WHERE oreg.id = $1::integer AND oreg.erdate IS NULL";
        Database::QueryParams pinfo = Database::query_param_list(_id);

        Database::Result rinfo = conn.exec_params(qinfo, pinfo);
        if (!rinfo.size()) {
            throw std::runtime_error("not found");
        }

        Contact *data = new Contact();
        data->username     = corba_wrap_string(rinfo[0][1]);
        data->first_name   = corba_wrap_string(rinfo[0][2]);
        data->last_name    = corba_wrap_string(rinfo[0][3]);
        data->organization = corba_wrap_nullable_string(rinfo[0][4]);
        data->vat_reg_num  = corba_wrap_nullable_string(rinfo[0][5]);
        data->ssn_type     = corba_wrap_nullable_string(rinfo[0][6]);

        int ident_type = static_cast<int>(rinfo[0][6]);
        data->id_card_num  = ident_type == 2 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->passport_num = ident_type == 3 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->vat_id_num   = ident_type == 4 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->ssn_id_num   = ident_type == 5 ? corba_wrap_nullable_string(rinfo[0][7]) : 0;
        data->birth_date   = ident_type == 6 ? corba_wrap_nullable_date(rinfo[0][7]) : 0;

        data->disclose_name         = corba_wrap_nullable_boolean(rinfo[0][8]);
        data->disclose_organization = corba_wrap_nullable_boolean(rinfo[0][9]);
        data->disclose_vat          = corba_wrap_nullable_boolean(rinfo[0][10]);
        data->disclose_ident        = corba_wrap_nullable_boolean(rinfo[0][11]);
        data->disclose_email        = corba_wrap_nullable_boolean(rinfo[0][12]);
        data->disclose_notify_email = corba_wrap_nullable_boolean(rinfo[0][13]);
        data->disclose_address      = corba_wrap_nullable_boolean(rinfo[0][14]);
        data->disclose_phone        = corba_wrap_nullable_boolean(rinfo[0][15]);
        data->disclose_fax          = corba_wrap_nullable_boolean(rinfo[0][16]);

        data->addresses.length(1);
        data->addresses[0].type         = "DEFAULT";
        data->addresses[0].street1      = corba_wrap_string(rinfo[0][17]);
        data->addresses[0].street2      = corba_wrap_nullable_string(rinfo[0][18]);
        data->addresses[0].street3      = corba_wrap_nullable_string(rinfo[0][19]);
        data->addresses[0].city         = corba_wrap_string(rinfo[0][20]);
        data->addresses[0].state        = corba_wrap_nullable_string(rinfo[0][21]);
        data->addresses[0].postal_code  = corba_wrap_string(rinfo[0][22]);
        data->addresses[0].country      = corba_wrap_string(rinfo[0][23]);

        data->emails.length(1);
        data->emails[0].type = "DEFAULT";
        data->emails[0].email_address = corba_wrap_string(rinfo[0][24]);
        std::string notify_email = static_cast<std::string>(rinfo[0][25]);
        if (notify_email.size()) {
            data->emails.length(2);
            data->emails[1].type = "NOTIFY";
            data->emails[1].email_address = corba_wrap_string(notify_email);
        }

        std::string telephone = static_cast<std::string>(rinfo[0][26]);
        std::string fax = static_cast<std::string>(rinfo[0][27]);
        if (telephone.size()) {
            data->phones.length(1);
            data->phones[0].type = "DEFAULT";
            data->phones[0].number = corba_wrap_string(telephone);
        }
        if (fax.size()) {
            unsigned int s = data->phones.length();
            data->phones.length(s + 1);
            data->phones[s].type = "FAX";
            data->phones[s].number = corba_wrap_string(fax);
        }

        LOGGER(PACKAGE).info("request completed successfully");
        return data;
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::ErrorReport();
    }
}


void MojeIDImpl::commitPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("commit-prepared");

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- transaction_id: ")
                % _trans_id);

        throw std::runtime_error("not implemented");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::ErrorReport();
    }
}


void MojeIDImpl::rollbackPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(create_ctx_name(server_name_));
    Logging::Context ctx("rollback-prepared");

    try {
        LOGGER(PACKAGE).info(boost::format("request data -- transaction_id: ")
                % _trans_id);

        throw std::runtime_error("not implemented");
    }
    catch (std::exception &_ex) {
        LOGGER(PACKAGE).error(boost::format("request failed (%1%)") % _ex.what());
        throw Registry::MojeID::ErrorReport(_ex.what());
    }
    catch (...) {
        LOGGER(PACKAGE).error("request failed (unknown error)");
        throw Registry::MojeID::ErrorReport();
    }
}

char* MojeIDImpl::getIdentificationInfo(CORBA::ULongLong)
{
    return 0;
}


}

