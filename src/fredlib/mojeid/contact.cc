#include "contact.h"
#include "random.h"
#include "fredlib/db_settings.h"


namespace MojeID {


unsigned long long db_contact_object_create(const unsigned long long &_registrar_id,
                                            const std::string &_handle,
                                            const std::string &_auth_info)
{
    /* object_registry record */
    Database::Connection conn = Database::Manager::acquire();
    Database::Result roreg = conn.exec_params(
            "SELECT create_object($1::integer, $2::text, $3::integer)",
            Database::query_param_list(_registrar_id)(_handle)(1));
    if (roreg.size() != 1) {
        throw std::runtime_error("create object failed");
    }
    unsigned long long id = static_cast<unsigned long long>(roreg[0][0]);
    if (id == 0) {
        throw std::runtime_error("create object failed");
    }
    /* object record */
    Database::Result robject = conn.exec_params(
            "INSERT INTO object (id, clid, authinfopw) VALUES ($1::integer, $2::integer,"
                " $3::text)",
            Database::query_param_list(id)
                (_registrar_id)
                (_auth_info));

    return id;
}


unsigned long long db_contact_insert(Contact &_data)
{
    if (static_cast<unsigned long long>(_data.id) == 0) {
        throw std::runtime_error("cannot insert contact record without contact id");
    }

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
                 " $15::text, (SELECT id FROM enum_ssntype WHERE type = $16::text),"
                 " $17::boolean, $18::boolean,"
                 " $19::boolean, $20::boolean, $21::boolean, $22::boolean,"
                 " $23::boolean, $24::boolean, $25::boolean)";

    Database::QueryParams pcontact = Database::query_param_list
        (_data.name)
        (_data.organization)
        (_data.street1)
        (_data.street2)
        (_data.street3)
        (_data.city)
        (_data.stateorprovince)
        (_data.postalcode)
        (_data.country)
        (_data.telephone)
        (_data.fax)
        (_data.email)
        (_data.notifyemail)
        (_data.vat)
        (_data.ssn)
        (_data.ssntype)
        (_data.disclosename)
        (_data.discloseorganization)
        (_data.discloseaddress)
        (_data.disclosetelephone)
        (_data.disclosefax)
        (_data.discloseemail)
        (_data.disclosenotifyemail)
        (_data.disclosevat)
        (_data.discloseident)
        (_data.id);

    Database::Connection conn = Database::Manager::acquire();
    Database::Result rcontact = conn.exec_params(qcontact, pcontact);
    return _data.id;
}


void db_contact_update(Contact &_data)
{
    if (static_cast<unsigned long long>(_data.id) == 0) {
        throw std::runtime_error("cannot update contact record without contact id");
    }

    std::string qcontact =
        "UPDATE contact SET"
                 " name = $1::text, organization = $2::text,"
                 " street1 = $3::text, street2 = $4::text,"
                 " street3 = $5::text, city = $6::text,"
                 " stateorprovince = $7::text, postalcode = $8::text,"
                 " country = $9::text, telephone = $10::text,"
                 " fax = $11::text, email = $12::text, notifyemail = $13::text,"
                 " vat = $14::text, ssn = $15::text,"
                 " ssntype = (SELECT id FROM enum_ssntype WHERE type = $16::text),"
                 " disclosename = $17::boolean, discloseorganization = $18::boolean,"
                 " discloseaddress = $19::boolean, disclosetelephone = $20::boolean,"
                 " disclosefax = $21::boolean, discloseemail = $22::boolean,"
                 " disclosenotifyemail = $23::boolean, disclosevat = $24::boolean,"
                 " discloseident = $25::boolean"
            " WHERE"
                " id = $26::integer";

    Database::QueryParams pcontact = Database::query_param_list
        (_data.name)
        (_data.organization)
        (_data.street1)
        (_data.street2)
        (_data.street3)
        (_data.city)
        (_data.stateorprovince)
        (_data.postalcode)
        (_data.country)
        (_data.telephone)
        (_data.fax)
        (_data.email)
        (_data.notifyemail)
        (_data.vat)
        (_data.ssn)
        (_data.ssntype)
        (_data.disclosename)
        (_data.discloseorganization)
        (_data.discloseaddress)
        (_data.disclosetelephone)
        (_data.disclosefax)
        (_data.discloseemail)
        (_data.disclosenotifyemail)
        (_data.disclosevat)
        (_data.discloseident)
        (_data.id);

    Database::Connection conn = Database::Manager::acquire();
    Database::Result rcontact = conn.exec_params(qcontact, pcontact);
}


unsigned long long db_contact_insert_history(const unsigned long long &_request_id,
                                             const unsigned long long &_contact_id)
{
    if (_contact_id == 0) {
        throw std::runtime_error("cannot insert contact history record without contact id");
    }

    Database::Connection conn = Database::Manager::acquire();

    conn.exec_params("INSERT INTO history (id, request_id)"
                     " VALUES (DEFAULT, $1::bigint)",
                     Database::query_param_list(_request_id));
    Database::Result rhistory = conn.exec("SELECT currval('history_id_seq')");
    unsigned long long history_id = 0;
    history_id = rhistory[0][0];
    if (!history_id) {
        throw std::runtime_error("cannot save new history");
    }

    conn.exec_params("UPDATE object_registry SET historyid = $1::integer"
            " WHERE id = $2::integer",
            Database::query_param_list(history_id)(_contact_id));

    Database::Result robject_history = conn.exec_params(
            "INSERT INTO object_history (historyid, id, clid, upid, trdate, update, authinfopw)"
            " SELECT $1::integer, o.id, o.clid, o.upid, o.trdate, o.update, o.authinfopw FROM object o"
            " WHERE o.id = $2::integer",
            Database::query_param_list(history_id)(_contact_id));

    Database::Result rcontact_history = conn.exec_params(
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


unsigned long long contact_create(const unsigned long long &_request_id,
                                  const unsigned long long &_registrar_id,
                                  MojeID::Contact &_data)
{
    std::string auth_info = (_data.auth_info.isnull() == true) ? Random::string_alphanum(8)
            : static_cast<std::string>(_data.auth_info);
    _data.id = db_contact_object_create(_registrar_id, _data.handle, auth_info);
    db_contact_insert(_data);
    unsigned long long hid = db_contact_insert_history(_request_id, _data.id);
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("UPDATE object_registry SET crhistoryid = $1::integer"
                     " WHERE id = $2::integer",
                     Database::query_param_list(hid)(_data.id));
    return hid;
}


unsigned long long contact_transfer(const unsigned long long &_request_id,
                                    const unsigned long long &_registrar_id,
                                    const unsigned long long &_contact_id)
{
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("UPDATE object SET clid = $1::integer, trdate = now(),"
                     " authinfopw = $2::text WHERE id = $3::integer",
                     Database::query_param_list
                         (_registrar_id)
                         (Random::string_alphanum(8))
                         (_contact_id));
    return db_contact_insert_history(_request_id, _contact_id);
}


unsigned long long contact_update(const unsigned long long &_request_id,
                                  const unsigned long long &_registrar_id,
                                  Contact &_data)
{
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("UPDATE object SET upid = $1::integer, update = now()"
                     " , authinfopw = $3::text "
                     " WHERE id = $2::integer",
                     Database::query_param_list
                        (_registrar_id)
                        (_data.id)
                        (_data.auth_info));
    db_contact_update(_data);
    return db_contact_insert_history(_request_id, _data.id);
}


const MojeID::Contact contact_info(const unsigned long long &_id)
{
    Database::Connection conn = Database::Manager::acquire();

    std::string qinfo = "SELECT oreg.id, oreg.name,"
        " c.name,"
        " c.organization, c.vat, c.ssntype, c.ssn,"
        " c.disclosename, c.discloseorganization,"
        " c.disclosevat, c.discloseident,"
        " c.discloseemail, c.disclosenotifyemail,"
        " c.discloseaddress, c.disclosetelephone,"
        " c.disclosefax,"
        " c.street1, c.street2, c.street3,"
        " c.city, c.stateorprovince, c.postalcode, c.country,"
        " c.email, c.notifyemail, c.telephone, c.fax, est.type,"
        " o.authinfopw"
        " FROM object_registry oreg JOIN contact c ON c.id = oreg.id"
        " JOIN object o ON o.id = oreg.id"
        " LEFT JOIN enum_ssntype est ON est.id = c.ssntype"
        " WHERE oreg.id = $1::integer AND oreg.erdate IS NULL";

    Database::Result rinfo = conn.exec_params(qinfo, Database::query_param_list(_id));
    if (!rinfo.size()) {
        throw std::runtime_error("not found");
    }

    /* when attribute is Nullable<T> type value is assinged from
     * result only if is not null
     * (checking is done in Nullable<T>::operator=(Database::Value)) */
    Contact data;
    data.id = rinfo[0][0];
    data.handle = static_cast<std::string>(rinfo[0][1]);
    data.name = static_cast<std::string>(rinfo[0][2]);
    data.organization = rinfo[0][3];
    data.vat = rinfo[0][4];
    data.ssntype = rinfo[0][27];
    data.ssn  = rinfo[0][6];
    data.disclosename = rinfo[0][7];
    data.discloseorganization = rinfo[0][8];
    data.disclosevat = rinfo[0][9];
    data.discloseident = rinfo[0][10];
    data.discloseemail = rinfo[0][11];
    data.disclosenotifyemail = rinfo[0][12];
    data.discloseaddress = rinfo[0][13];
    data.disclosetelephone = rinfo[0][14];
    data.disclosefax = rinfo[0][15];
    data.street1 = rinfo[0][16];
    data.street2 = rinfo[0][17];
    data.street3 = rinfo[0][18];
    data.city    = rinfo[0][19];
    data.stateorprovince = rinfo[0][20];
    data.postalcode = rinfo[0][21];
    data.country = rinfo[0][22];
    data.email = rinfo[0][23];
    data.notifyemail = rinfo[0][24];
    data.telephone = rinfo[0][25];
    data.fax = rinfo[0][26];
    data.auth_info = rinfo[0][28];

    return data;
}


void contact_transfer_poll_message(const unsigned long long &_old_registrar_id,
                                   const unsigned long long &_contact_id)
{
    Database::Connection conn = Database::Manager::acquire();
    Database::Transaction tx(conn);
    conn.exec_params(
            "INSERT INTO message (id, clid, crdate, exdate, seen, msgtype)"
            " VALUES (DEFAULT, $1::integer, CURRENT_TIMESTAMP,"
            " CURRENT_TIMESTAMP + $2::interval, false, $3::integer)",
            Database::query_param_list
                (_old_registrar_id)
                ("7 days")
                (3 /* MT_TRANSFER_CONTACT = 3 from poll.h */));

    conn.exec_params(
            "INSERT INTO poll_eppaction (msgid, objid)"
            " SELECT currval('message_id_seq'), historyid "
            " FROM object_registry WHERE id = $1::integer",
            Database::query_param_list
                (_contact_id));
    tx.commit();
}


}

