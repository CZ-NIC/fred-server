#ifndef MOJEID_CONTACT_H_
#define MOJEID_CONTACT_H_

#include "random.h"
#include "register/db_settings.h"
#include "mojeid_request.h"

namespace Registry {


unsigned long long create_object(MojeIDRequest &_request,
                                 const std::string &_handle)
{
    /* object_registry record */
    Database::Result roreg = _request.conn.exec_params(
            "SELECT create_object($1::integer, $2::text, $3::integer)",
            Database::query_param_list(_request.get_registrar_id())(_handle)(1));
    if (roreg.size() != 1) {
        throw std::runtime_error("create object failed");
    }
    unsigned long long id = static_cast<unsigned long long>(roreg[0][0]);

    /* object record */
    Database::Result robject = _request.conn.exec_params(
            "INSERT INTO object (id, clid, authinfopw) VALUES ($1::integer, $2::integer,"
                " $3::text)",
            Database::query_param_list(id)
                (_request.get_registrar_id())
                (Random::string_alphanum(8)));

    return id;
}


void insert_contact(MojeIDRequest &_request,
                    const unsigned long long &_id,
                    Database::QueryParams &_pcontact)
{
    std::string qcontact =
        "INSERT INTO contact ("
                 "id, name, organization, street1, street2, street3,"
                 " city, stateorprovince, postalcode, country, telephone,"
                 " fax, email, notifyemail, vat, ssn, ssntype,"
                 " disclosename, discloseorganization, discloseaddress,"
                 " disclosetelephone, disclosefax, discloseemail,"
                 " disclosenotifyemail, disclosevat, discloseident"
            ") VALUES ("
                 " $26::integer, $1::text, $2::text, $3::text, $4::text,"
                 " $5::text, $6::text, $7::text, $8::text, $9::text,"
                 " $10::text, $11::text, $12::text, $13::text, $14::text,"
                 " $15::text, $16::integer, $17::boolean, $18::boolean,"
                 " $19::boolean, $20::boolean, $21::boolean, $22::boolean,"
                 " $23::boolean, $24::boolean, $25::boolean)";

    _pcontact.push_back(_id);
    Database::Result rcontact = _request.conn.exec_params(qcontact, _pcontact);
}


void update_contact(MojeIDRequest &_request,
                    const unsigned long long &_id,
                    Database::QueryParams &_pcontact)
{
    std::string qcontact =
        "UPDATE contact SET"
                 " name = $1::text, organization = $2::text,"
                 " street1 = $3::text, street2 = $4::text,"
                 " street3 = $5::text, city = $6::text,"
                 " stateorprovince = $7::text, postalcode = $8::text,"
                 " country = $9::text, telephone = $10::text,"
                 " fax = $11::text, email = $12::text, notifyemail = $13::text,"
                 " vat = $14::text, ssn = $15::text, ssntype = $16::integer,"
                 " disclosename = $17::boolean, discloseorganization = $18::boolean,"
                 " discloseaddress = $19::boolean, disclosetelephone = $20::boolean,"
                 " disclosefax = $21::boolean, discloseemail = $22::boolean,"
                 " disclosenotifyemail = $23::boolean, disclosevat = $24::boolean,"
                 " discloseident = $25::boolean"
            " WHERE"
                " id = $26::integer";

    _pcontact.push_back(_id);
    Database::Result rcontact = _request.conn.exec_params(qcontact, _pcontact);
}


unsigned long long insert_contact_history(MojeIDRequest &_request,
                                          const unsigned long long &_contact_id)
{
    _request.conn.exec_params("INSERT INTO history (id, action) VALUES (DEFAULT, $1::integer)",
                             Database::query_param_list(_request.get_id()));
    Database::Result rhistory = _request.conn.exec("SELECT currval('history_id_seq')");
    unsigned long long history_id = 0;
    history_id = rhistory[0][0];
    if (!history_id) {
        throw std::runtime_error("cannot save new history");
    }

    _request.conn.exec_params("UPDATE object_registry SET crhistoryid = $1::integer,"
            " historyid = $1::integer WHERE id = $2::integer",
            Database::query_param_list(history_id)(_contact_id));

    Database::Result robject_history = _request.conn.exec_params(
            "INSERT INTO object_history (historyid, id, clid, upid, trdate, update, authinfopw)"
            " SELECT $1::integer, o.id, o.clid, o.upid, o.trdate, o.update, o.authinfopw FROM object o"
            " WHERE o.id = $2::integer",
            Database::query_param_list(history_id)(_contact_id));

    Database::Result rcontact_history = _request.conn.exec_params(
            "INSERT INTO contact_history (historyid, id, name, organization, street1, street2, street3,"
            " city, stateorprovince, postalcode, country, telephone, fax, email, disclosename,"
            " discloseorganization, discloseaddress, disclosetelephone, disclosefax, discloseemail,"
            " notifyemail, vat, ssn, ssntype, disclosevat, discloseident, disclosenotifyemail)"
            " SELECT $1::integer, c.id, c.name, c.organization, c.street1, c.street2, c.street3,"
            " c.city, c.stateorprovince, c.postalcode, c.country, c.telephone, c.fax, c.email,"
            " c.disclosename, c.discloseorganization, c.discloseaddress, c.disclosetelephone, c.disclosefax,"
            " c.discloseemail, c.notifyemail, c.vat, c.ssn, c.ssntype, c.disclosevat, c.discloseident,"
            " c.disclosenotifyemail FROM contact c WHERE c.id = $2::integer",
            Database::query_param_list(history_id)(_contact_id));

    return history_id;
}

}

#endif /*MOJEID_CONTACT_H_*/

