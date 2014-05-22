/*
 * Copyright (C) 2012  CZ.NIC, z.s.p.o.
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
 *  @contact_verification_validators.cc
 *  mojeid contact verification
 */

#include "mojeid_validators.h"
#include "util/types/birthdate.h"

namespace Fred {
namespace Contact {
namespace Verification {

/**
 * return true in case the contacts are equal in terms registry data
 */
bool check_conditionally_identified_contact_diff(
        const Contact &_c1,
        const Contact &_c2)
{
    /* name */
    if (_c1.name != _c2.name) {
        return false;
    }
    /* organization */
    if (_c1.organization.get_value_or_default() != _c2.organization.get_value_or_default()) {
        return false;
    }
    /* dic */
    if (_c1.vat.get_value_or_default() != _c2.vat.get_value_or_default()) {
        return false;
    }
    /* address */
    if ((_c1.street1.get_value_or_default() != _c2.street1.get_value_or_default())
            || (_c1.street2.get_value_or_default() != _c2.street2.get_value_or_default())
            || (_c1.street3.get_value_or_default() != _c2.street3.get_value_or_default())
            || (_c1.city.get_value_or_default() != _c2.city.get_value_or_default())
            || (_c1.stateorprovince.get_value_or_default() != _c2.stateorprovince.get_value_or_default())
            || (_c1.country.get_value_or_default() != _c2.country.get_value_or_default())
            || (_c1.postalcode.get_value_or_default() != _c2.postalcode.get_value_or_default())) {
        return false;
    }
    /* identification type */
    if (_c1.ssntype.get_value_or_default() != _c2.ssntype.get_value_or_default()) {
        return false;
    }
    /* identification regardless of type*/
    if (_c1.ssn.get_value_or_default() != _c2.ssn.get_value_or_default()) {

        if (_c1.ssntype.get_value_or_default() == "BIRTHDAY") {
            boost::gregorian::date before = birthdate_from_string_to_date(_c1.ssn.get_value_or_default());
            boost::gregorian::date after = birthdate_from_string_to_date(_c2.ssn.get_value_or_default());
            if (before != after) {
                return false;
            }

        }
        else {
            return false;
        }
    }
    /* telephone and email */
    if ((_c1.telephone.get_value_or_default() != _c2.telephone.get_value_or_default())
            || (_c1.fax.get_value_or_default() != _c2.fax.get_value_or_default())
            || (_c1.email.get_value_or_default() != _c2.email.get_value_or_default())
            || (_c1.notifyemail.get_value_or_default() != _c2.notifyemail.get_value_or_default())) {
        return false;
    }
    /* all disclose disclose flags */
    if ( (_c1.disclosename != _c2.disclosename)
            || (_c1.discloseorganization != _c2.discloseorganization)
            || (_c1.discloseaddress != _c2.discloseaddress)
            || (_c1.disclosetelephone != _c2.disclosetelephone)
            || (_c1.disclosefax != _c2.disclosefax)
            || (_c1.discloseemail != _c2.discloseemail)
            || (_c1.disclosevat != _c2.disclosevat)
            || (_c1.discloseident != _c2.discloseident)
            || (_c1.disclosenotifyemail != _c2.disclosenotifyemail)) {
        return false;
    }

    return true;
}


bool check_validated_contact_diff(
        const Contact &_c1,
        const Contact &_c2)
{
    /* name */
    if (_c1.name != _c2.name) {
        return false;
    }
    /* organization */
    if (_c1.organization.get_value_or_default() != _c2.organization.get_value_or_default()) {
        return false;
    }
    /* dic */
    if (_c1.vat.get_value_or_default() != _c2.vat.get_value_or_default()) {
        return false;
    }
    /* address */
    if ((_c1.street1.get_value_or_default() != _c2.street1.get_value_or_default())
            || (_c1.street2.get_value_or_default() != _c2.street2.get_value_or_default())
            || (_c1.street3.get_value_or_default() != _c2.street3.get_value_or_default())
            || (_c1.city.get_value_or_default() != _c2.city.get_value_or_default())
            || (_c1.stateorprovince.get_value_or_default() != _c2.stateorprovince.get_value_or_default())
            || (_c1.country.get_value_or_default() != _c2.country.get_value_or_default())
            || (_c1.postalcode.get_value_or_default() != _c2.postalcode.get_value_or_default())) {
        return false;
    }
    /* identification type */
    if (_c1.ssntype.get_value_or_default() != _c2.ssntype.get_value_or_default()) {
        return false;
    }
    /* identification regardless of type*/
    if (_c1.ssn.get_value_or_default() != _c2.ssn.get_value_or_default()) {

        if(_c1.ssntype.get_value_or_default() == "BIRTHDAY") {
            boost::gregorian::date before = birthdate_from_string_to_date(_c1.ssn.get_value_or_default());
            boost::gregorian::date after = birthdate_from_string_to_date(_c2.ssn.get_value_or_default());
            if(before != after) {
                return false;
            }
        }
        else {
            return false;
        }
    }

    return true;
}



ContactValidator create_conditional_identification_validator_mojeid()
{
    ContactValidator tmp = create_conditional_identification_validator();
    tmp.add_checker(contact_checker_username);
    tmp.add_checker(contact_checker_birthday);
    return tmp;
}

ContactValidator create_finish_identification_validator_mojeid()
{
    ContactValidator tmp = create_default_contact_validator();
    return tmp;
}

ContactValidator create_verified_transfer_validator_mojeid()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_username);
    tmp.add_checker(contact_checker_birthday);
    return tmp;
}

ContactValidator create_contact_update_validator_mojeid()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_birthday);
    return tmp;
}


}
}
}
