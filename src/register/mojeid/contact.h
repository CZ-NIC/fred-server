#ifndef MOJEID_CONTACT_H_
#define MOJEID_CONTACT_H_

#include "nullable.h"
#include <string>
#include <map>

namespace MojeID {


class Contact
{
public:
    Contact() : id(0)
    {
    }

    unsigned long long id;
    std::string handle;
    std::string name;
    Nullable<std::string> organization;
    Nullable<std::string> street1;
    Nullable<std::string> street2;
    Nullable<std::string> street3;
    Nullable<std::string> city;
    Nullable<std::string> stateorprovince;
    Nullable<std::string> postalcode;
    Nullable<std::string> country;
    Nullable<std::string> telephone;
    Nullable<std::string> fax;
    Nullable<std::string> email;
    Nullable<bool> disclosename;
    Nullable<bool> discloseorganization;
    Nullable<bool> discloseaddress;
    Nullable<bool> disclosetelephone;
    Nullable<bool> disclosefax;
    Nullable<bool> discloseemail;
    Nullable<std::string> notifyemail;
    Nullable<std::string> vat;
    Nullable<std::string> ssn;
    Nullable<std::string> ssntype;
    Nullable<bool>disclosevat;
    Nullable<bool>discloseident;
    Nullable<bool>disclosenotifyemail;
};


unsigned long long contact_create(const unsigned long long &_action_id,
                                  const unsigned long long &_request_id,
                                  const unsigned long long &_registrar_id,
                                  Contact &_data);

unsigned long long contact_transfer(const unsigned long long &_action_id,
                                    const unsigned long long &_request_id,
                                    const unsigned long long &_registrar_id,
                                    const unsigned long long &_contact_id);

unsigned long long contact_update(const unsigned long long &_action_id,
                                  const unsigned long long &_request_id,
                                  Contact &_data);

const MojeID::Contact contact_info(const unsigned long long &_id);

}

#endif /*MOJEID_CONTACT_H_*/

