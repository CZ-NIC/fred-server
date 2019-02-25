/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/contact_verification/contact.hh"
#include "src/deprecated/libfred/registrable_object/contact.hh"
#include "src/deprecated/libfred/object_states.hh"
#include "libfred/db_settings.hh"
#include "src/util/types/birthdate.hh"
#include "util/random.hh"
#include <map>


namespace LibFred {
namespace Contact {
namespace Verification {


bool ContactAddress::operator==(const ContactAddress &_b)const
{
    // compare default values because frontend doesn't preserve NULL values
    return this->type == _b.type &&
           this->company_name.get_value_or_default() == _b.company_name.get_value_or_default() &&
           this->street1.get_value_or_default() == _b.street1.get_value_or_default() &&
           this->street2.get_value_or_default() == _b.street2.get_value_or_default() &&
           this->street3.get_value_or_default() == _b.street3.get_value_or_default() &&
           this->city.get_value_or_default() == _b.city.get_value_or_default() &&
           this->stateorprovince.get_value_or_default() == _b.stateorprovince.get_value_or_default() &&
           this->postalcode.get_value_or_default() == _b.postalcode.get_value_or_default() &&
           this->country.get_value_or_default() == _b.country.get_value_or_default();
}

ContactAddress Contact::get_mailing_address()const
{
    for (std::vector< ContactAddress >::const_iterator ptr_addr = addresses.begin();
         ptr_addr != addresses.end(); ++ptr_addr) {
        if (ptr_addr->type == Address::Type::MAILING) {
            return *ptr_addr;
        }
    }
    ContactAddress addr;
    addr.type = Address::Type::MAILING;
    addr.company_name = Nullable<std::string>();
    addr.street1 = this->street1;
    addr.street2 = this->street2;
    addr.street3 = this->street3;
    addr.city = this->city;
    addr.stateorprovince = this->stateorprovince;
    addr.postalcode = this->postalcode;
    addr.country = this->country;
    return addr;
}

bool transform_ssn_birthday_value(Contact &_data)
{
    if (_data.ssntype.get_value_or_default() == "BIRTHDAY")
    {
        std::string orig = _data.ssn.get_value_or_default();
        boost::gregorian::date conv = birthdate_from_string_to_date(orig);
        if (conv.is_special())
        {
            throw std::runtime_error("invalid ssn value for type BIRTHDAY");
        }
        _data.ssn = to_iso_extended_string(conv);
        return _data.ssn.get_value_or_default() != orig;
    }
    return false;
}


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


void db_contact_addresses_insert(Contact &_data)
{
    std::string qaddress =
        "INSERT INTO contact_address ("
        "contactid,type,company_name,street1,street2,street3,"
        " city,stateorprovince,postalcode,country)"
        " VALUES ("
         "$1::integer,$2::contact_address_type,$3::text,$4::text,$5::text,"
         "$6::text,$7::text,$8::text,$9::text,$10::text)";

    Database::Connection conn = Database::Manager::acquire();

    for (std::vector<ContactAddress>::const_iterator it = _data.addresses.begin(); it != _data.addresses.end(); ++it)
    {
        Database::QueryParams paddress = Database::query_param_list
            (_data.id)
            (it->type)
            (it->company_name)
            (it->street1)
            (it->street2)
            (it->street3)
            (it->city)
            (it->stateorprovince)
            (it->postalcode)
            (it->country);

        Database::Result raddress = conn.exec_params(qaddress, paddress);
    }
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
    db_contact_addresses_insert(_data);

    return _data.id;
}

namespace
{

typedef std::vector< ContactAddress > Addresses;
typedef std::map< std::string, ContactAddress > TypeToAddress;

void db_contact_delete_unnecessary(Database::Connection &_conn, unsigned long long _contactid,
                                   const Addresses &_addresses, TypeToAddress &_current_addresses,
                                   TypeToAddress &_required_addresses)
{
    std::ostringstream types_to_delete;
    Database::query_param_list to_delete_param(_contactid);
    _current_addresses.clear();
    _required_addresses.clear();

    for (Addresses::const_iterator pca = _addresses.begin(); pca != _addresses.end(); ++pca) {
        _required_addresses[pca->type] = *pca;
    }

    Database::Result res_addr = _conn.exec_params(
        "SELECT type,company_name,street1,street2,street3,city,stateorprovince,"
               "postalcode,country "
        "FROM contact_address "
        "WHERE contactid=$1::integer",
        Database::query_param_list(_contactid));
    for (::size_t idx = 0; idx < res_addr.size(); ++idx) {
        ContactAddress current_address;
        current_address.type = static_cast< std::string >(res_addr[idx][0]);
        if (_required_addresses.find(current_address.type) == _required_addresses.end()) {
            to_delete_param(current_address.type);
            if (!types_to_delete.str().empty()) {
                types_to_delete << ",";
            }
            types_to_delete << "$" << to_delete_param.size() << "::contact_address_type";
            continue;
        }
        if (!res_addr[idx][1].isnull()) {
            current_address.company_name = static_cast< std::string >(res_addr[idx][1]);
        }
        if (!res_addr[idx][2].isnull()) {
          current_address.street1 = static_cast< std::string >(res_addr[idx][2]);
        }
        if (!res_addr[idx][3].isnull()) {
          current_address.street2 = static_cast< std::string >(res_addr[idx][3]);
        }
        if (!res_addr[idx][4].isnull()) {
          current_address.street3 = static_cast< std::string >(res_addr[idx][4]);
        }
        if (!res_addr[idx][5].isnull()) {
          current_address.city = static_cast< std::string >(res_addr[idx][5]);
        }
        if (!res_addr[idx][6].isnull()) {
          current_address.stateorprovince = static_cast< std::string >(res_addr[idx][6]);
        }
        if (!res_addr[idx][7].isnull()) {
          current_address.postalcode = static_cast< std::string >(res_addr[idx][7]);
        }
        if (!res_addr[idx][8].isnull()) {
          current_address.country = static_cast< std::string >(res_addr[idx][8]);
        }
        _current_addresses[current_address.type] = current_address;
    }

    if (!types_to_delete.str().empty()) {
        _conn.exec_params("DELETE FROM contact_address "
                          "WHERE contactid=$1::integer AND "
                                "type IN (" + types_to_delete.str() + ")", to_delete_param);
    }
}

void db_contact_insert_or_update(Database::Connection &_conn, unsigned long long _contactid,
                                 const TypeToAddress &_current_addresses,
                                 const TypeToAddress &_required_addresses)
{
    std::ostringstream values_to_insert;
    Database::query_param_list to_insert_param(_contactid);
    for (TypeToAddress::const_iterator ptr_required_addr = _required_addresses.begin();
         ptr_required_addr != _required_addresses.end(); ++ptr_required_addr) {
        const ContactAddress &required_addr = ptr_required_addr->second;
        TypeToAddress::const_iterator ptr_current_addr = _current_addresses.find(required_addr.type);
        if (ptr_current_addr == _current_addresses.end()) { // required address doesn't exist => insert
            if (!values_to_insert.str().empty()) {
                values_to_insert << ",";
            }
            values_to_insert << "($1::integer";
            to_insert_param(required_addr.type);
            values_to_insert << ",$" << to_insert_param.size() << "::contact_address_type";
            if (!required_addr.company_name.isnull()) {
                to_insert_param(required_addr.company_name.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            if (!required_addr.street1.isnull()) {
                to_insert_param(required_addr.street1.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            if (!required_addr.street2.isnull()) {
                to_insert_param(required_addr.street2.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            if (!required_addr.street3.isnull()) {
                to_insert_param(required_addr.street3.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            if (!required_addr.city.isnull()) {
                to_insert_param(required_addr.city.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            if (!required_addr.stateorprovince.isnull()) {
                to_insert_param(required_addr.stateorprovince.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            if (!required_addr.postalcode.isnull()) {
                to_insert_param(required_addr.postalcode.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            if (!required_addr.country.isnull()) {
                to_insert_param(required_addr.country.get_value());
                values_to_insert << ",$" << to_insert_param.size() << "::text";
            }
            else {
                values_to_insert << ",NULL";
            }
            values_to_insert << ")";
        }
        else { // required address exists => update?
            const ContactAddress &current_addr = ptr_current_addr->second;
            if (current_addr != required_addr) {
                std::ostringstream to_update_set;
                Database::query_param_list to_update_param(_contactid);
                to_update_param(required_addr.type);
                to_update_set << "company_name=";
                if (!required_addr.company_name.isnull()) {
                    to_update_param(required_addr.company_name.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text,";
                }
                else {
                    to_update_set << "NULL,";
                }
                to_update_set << "street1=";
                if (!required_addr.street1.isnull()) {
                    to_update_param(required_addr.street1.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text,";
                }
                else {
                    to_update_set << "NULL,";
                }
                to_update_set << "street2=";
                if (!required_addr.street2.isnull()) {
                    to_update_param(required_addr.street2.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text,";
                }
                else {
                    to_update_set << "NULL,";
                }
                to_update_set << "street3=";
                if (!required_addr.street3.isnull()) {
                    to_update_param(required_addr.street3.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text,";
                }
                else {
                    to_update_set << "NULL,";
                }
                to_update_set << "city=";
                if (!required_addr.city.isnull()) {
                    to_update_param(required_addr.city.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text,";
                }
                else {
                    to_update_set << "NULL,";
                }
                to_update_set << "stateorprovince=";
                if (!required_addr.stateorprovince.isnull()) {
                    to_update_param(required_addr.stateorprovince.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text,";
                }
                else {
                    to_update_set << "NULL,";
                }
                to_update_set << "postalcode=";
                if (!required_addr.postalcode.isnull()) {
                    to_update_param(required_addr.postalcode.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text,";
                }
                else {
                    to_update_set << "NULL,";
                }
                to_update_set << "country=";
                if (!required_addr.country.isnull()) {
                    to_update_param(required_addr.country.get_value());
                    to_update_set << "$" << to_update_param.size() << "::text";
                }
                else {
                    to_update_set << "NULL";
                }
                _conn.exec_params("UPDATE contact_address "
                                  "SET " + to_update_set.str() + " "
                                  "WHERE contactid=$1::integer AND "
                                        "type=$2::contact_address_type", to_update_param);
            }
        }
    }

    if (!values_to_insert.str().empty()) {
        _conn.exec_params("INSERT INTO contact_address (contactid,type,company_name,"
                                                       "street1,street2,street3,city,"
                                                       "stateorprovince,postalcode,country) "
                          "VALUES " + values_to_insert.str(), to_insert_param);
    }
}

}

void db_contact_update(Contact &_data, const Optional<Nullable<bool> >& contact_warning_letter_preference)
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
                 " discloseident = $25::boolean" + std::string(contact_warning_letter_preference.isset() ? ", warning_letter = $27::boolean" : "") +
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

    if(contact_warning_letter_preference.isset())
    {
        Nullable<bool> new_warning_letter = contact_warning_letter_preference.get_value();
        if(new_warning_letter.isnull())
        {
            pcontact.push_back(Database::NullQueryParam);//NULL
        }
        else
        {
            pcontact.push_back(new_warning_letter.get_value());
        }
    }

    Database::Connection conn = Database::Manager::acquire();
    Database::Result rcontact = conn.exec_params(qcontact, pcontact);

    TypeToAddress current_addresses;
    TypeToAddress required_addresses;
    db_contact_delete_unnecessary(conn, _data.id, _data.addresses, current_addresses,
                                  required_addresses);
    db_contact_insert_or_update(conn, _data.id, current_addresses, required_addresses);
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
            " notifyemail, vat, ssn, ssntype, disclosevat, discloseident, disclosenotifyemail, warning_letter)"
            " SELECT $1::integer, c.id, c.name, c.organization, c.street1, c.street2, c.street3,"
            " c.city, c.stateorprovince, c.postalcode, c.country, c.telephone, c.fax, c.email,"
            " c.disclosename, c.discloseorganization, c.discloseaddress, c.disclosetelephone, c.disclosefax,"
            " c.discloseemail, c.notifyemail, c.vat, c.ssn, c.ssntype, c.disclosevat, c.discloseident,"
            " c.disclosenotifyemail, c.warning_letter FROM contact c WHERE c.id = $2::integer",
            Database::query_param_list(history_id)(_contact_id));

    Database::Result rcontact_address_history = conn.exec_params(
            "INSERT INTO contact_address_history (historyid,"
            " id, contactid, type, company_name, street1, street2, street3,"
            " city, stateorprovince, postalcode, country)"
            " SELECT $1::integer,"
            " id, contactid, type, company_name, street1, street2, street3,"
            " city, stateorprovince, postalcode, country"
            " FROM contact_address WHERE contactid = $2::integer",
            Database::query_param_list(history_id)(_contact_id));

    return history_id;
}


unsigned long long contact_create(const unsigned long long &_request_id,
                                  const unsigned long long &_registrar_id,
                                  Contact &_data)
{
    transform_ssn_birthday_value(_data);
    _data.id = db_contact_object_create(_registrar_id, _data.handle, Random::string_alphanum(8));
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
    unsigned long long hid = db_contact_insert_history(_request_id, _contact_id);

    Contact tmp = contact_info(_contact_id);
    if (transform_ssn_birthday_value(tmp))
    {
        hid = contact_update(_request_id, _registrar_id, tmp);
    }
    return hid;
}


unsigned long long contact_update(const unsigned long long &_request_id,
                                  const unsigned long long &_registrar_id,
                                  Contact &_data, const Optional<Nullable<bool> >& contact_warning_letter_preference)
{
    transform_ssn_birthday_value(_data);
    Database::Connection conn = Database::Manager::acquire();
    conn.exec_params("UPDATE object SET upid = $1::integer, update = now()"
                     " WHERE id = $2::integer",
                     Database::query_param_list
                        (_registrar_id)
                        (_data.id));
    db_contact_update(_data, contact_warning_letter_preference);
    return db_contact_insert_history(_request_id, _data.id);
}


const Contact contact_info(const unsigned long long &_id)
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
        " c.email, c.notifyemail, btrim(c.telephone), btrim(c.fax), est.type,"
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

    std::string qaddresses = 
        "SELECT type,company_name,street1,street2,street3,city,stateorprovince,postalcode,country "
         "FROM contact_address "
         "WHERE contactid=$1::integer";
    Database::Result raddresses = conn.exec_params(qaddresses, Database::query_param_list(_id));
    for (unsigned long long i = 0; i < raddresses.size(); ++i)
    {
        ContactAddress addr;
        addr.type = static_cast<std::string>(raddresses[i][0]);
        addr.company_name = static_cast<std::string>(raddresses[i][1]);
        addr.street1 = static_cast<std::string>(raddresses[i][2]);
        addr.street2 = static_cast<std::string>(raddresses[i][3]);
        addr.street3 = static_cast<std::string>(raddresses[i][4]);
        addr.city = static_cast<std::string>(raddresses[i][5]);
        addr.stateorprovince = static_cast<std::string>(raddresses[i][6]);
        addr.postalcode = static_cast<std::string>(raddresses[i][7]);
        addr.country = static_cast<std::string>(raddresses[i][8]);
        data.addresses.push_back(addr);
    }

    return data;
}


void contact_load_disclose_flags(Contact &_data)
{
    if (_data.id == 0) {
        throw std::runtime_error("can't load disclose flags for not saved contact");
    }
    Database::Connection conn = Database::Manager::acquire();
    Database::Result r = conn.exec_params(
            "SELECT disclosename, discloseorganization,"
            " discloseaddress, disclosetelephone,"
            " disclosefax, discloseemail, disclosevat,"
            " discloseident, disclosenotifyemail"
            " FROM contact WHERE id = $1",
            Database::query_param_list(_data.id));
    if (r.size() != 1) {
        throw std::runtime_error("unable to load contact dislose flags");
    }
    _data.disclosename = r[0][0];
    _data.discloseorganization = r[0][1];
    _data.discloseaddress = r[0][2];
    _data.disclosetelephone = r[0][3];
    _data.disclosefax = r[0][4];
    _data.discloseemail = r[0][5];
    _data.disclosevat = r[0][6];
    _data.discloseident = r[0][7];
    _data.disclosenotifyemail = r[0][8];
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

void contact_delete_not_linked(const unsigned long long &_id)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result contact_lock_result = conn.exec_params(
        "SELECT oreg.id AS id_ "
           "FROM object_registry AS oreg "
           "JOIN enum_object_type eot ON eot.id = oreg.type AND eot.name = 'contact'::text "
           "WHERE oreg.id = $1::integer "
               "AND oreg.erdate IS NULL "
           "FOR UPDATE OF oreg",
        Database::query_param_list(_id));
    if(contact_lock_result.size() == 0) {
        throw std::runtime_error("contact not found");
    }

    if(!LibFred::object_has_state(_id, "linked"))
    {
        conn.exec_params("DELETE FROM contact_address WHERE contactid = $1::integer",
            Database::query_param_list(_id));

        Database::Result delete_contact_res = conn.exec_params(
            "DELETE FROM contact WHERE id = $1::integer RETURNING id",
            Database::query_param_list(_id));
        if (delete_contact_res.size() != 1) {
            throw std::runtime_error("delete contact failed");
        }

        Database::Result update_erdate_res = conn.exec_params(
            "UPDATE object_registry "
                "SET erdate = now() "
                "WHERE id = $1::integer RETURNING id",
            Database::query_param_list(_id));
        if (update_erdate_res.size() != 1) {
            throw std::runtime_error("erdate update failed");
        }

        Database::Result delete_object_res = conn.exec_params(
            "DELETE FROM object WHERE id = $1::integer RETURNING id",
            Database::query_param_list(_id));
        if (delete_object_res.size() != 1) {
            throw std::runtime_error("delete object failed");
        }
    }
}

}
}
}

