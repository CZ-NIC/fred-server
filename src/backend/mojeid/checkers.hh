/*
 * Copyright (C) 2015-2020  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  declaration of partial contact check classes
 */

#ifndef CHECKERS_HH_1C4A77A46C944F69B5D59D1B75B30116
#define CHECKERS_HH_1C4A77A46C944F69B5D59D1B75B30116

#include "src/backend/mojeid/check_collector.hh"
#include "libfred/opcontext.hh"
#include "libfred/registrable_object/contact/info_contact_data.hh"

namespace Fred {
namespace Backend {

/**
 * How long can't be email or phone used for next identification request.
 * @return string value usable as parameter of INTERVAL type in SQL query
 */
inline std::string email_phone_protection_period()
{
    return "1MONTH";
}


/// General check classes
namespace GeneralCheck {

/**
 * Contact name verification.
 */
struct contact_name
{
    /**
     * Executes check.
     * @param _name contact name to verify
     */
    contact_name(const Nullable<std::string>& _name);
    /**
     * Executes check.
     * @param _first_name contact first name to verify
     * @param _last_name contact last name to verify
     */
    contact_name(
            const std::string& _first_name,
            const std::string& _last_name);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !(first_name_absent || last_name_absent);
    }
    bool first_name_absent : 1; ///< contact doesn't have first name
    bool last_name_absent : 1; ///< contact doesn't have last name
};

/**
 * Contact optional address verification.
 */
struct contact_optional_address
{
    /**
     * All checks set.
     * @param _success result of all checks
     */
    contact_optional_address(bool _success);
    /**
     * Executes check.
     * @param _street1 contact address part to verify
     * @param _city contact address part to verify
     * @param _postalcode contact address part to verify
     * @param _country contact address part to verify
     * @return self reference
     */
    contact_optional_address& operator()(
            const std::string& _street1,
            const std::string& _city,
            const std::string& _postalcode,
            const std::string& _country);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !(street1_absent || city_absent || postalcode_absent || country_absent);
    }

    bool street1_absent : 1;   ///< contact doesn't have street1 entry
    bool city_absent : 1;      ///< contact doesn't have city entry
    bool postalcode_absent : 1; ///< contact doesn't have postal code entry
    bool country_absent : 1;   ///< contact doesn't have country entry
};

/**
 * Contact address verification.
 */
struct contact_address
    : contact_optional_address
{
    /**
     * Executes check.
     * @param _street1 contact address part to verify
     * @param _city contact address part to verify
     * @param _postalcode contact address part to verify
     * @param _country contact address part to verify
     */
    contact_address(
            const std::string& _street1,
            const std::string& _city,
            const std::string& _postalcode,
            const std::string& _country);
};

/**
 * Contact e-mail presence checking.
 */
struct contact_email_presence
{
    /**
     * Executes check.
     * @param _email contact email to verify
     */
    contact_email_presence(const Nullable<std::string>& _email);
    /**
     * Contact e-mail presents.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !absent;
    }
    bool absent : 1; ///< contact e-mail doesn't present
};

/**
 * Contact e-mail format verification.
 */
struct contact_email_validity
{
    /**
     * Executes check.
     * @param _email contact email to verify
     */
    contact_email_validity(const Nullable<std::string>& _email);
    /**
     * Contact e-mail is valid or doesn't present.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !invalid;
    }
    bool invalid : 1; ///< contact e-mail presents but format fails to meet the requirements
};

/**
 * Contact e-mail availability verification.
 */
struct contact_email_availability
{
    /**
     * Executes check.
     * @param _email contact email to verify
     * @param _id contact id
     * @param _ctx operation context used to check processing
     */
    contact_email_availability(
            const Nullable<std::string>& _email,
            unsigned long long _id,
            LibFred::OperationContext& _ctx);
    /**
     * Contact e-mail is available for using in next identification request.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !(absent || used_recently);
    }
    bool absent : 1;       ///< contact e-mail doesn't present
    bool used_recently : 1; ///< contact e-mail used for identification request recently
};

/**
 * Contact notify e-mail format verification.
 */
struct contact_notifyemail_validity
{
    /**
     * Executes check.
     * @param _notifyemail contact email to verify
     */
    contact_notifyemail_validity(const Nullable<std::string>& _notifyemail);
    /**
     * Contact notify e-mail is valid or doesn't present.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !invalid;
    }
    bool invalid : 1; ///< contact notify e-mail presents and its format fails to meet the requirements
};

/**
 * Contact phone presence checking.
 */
struct contact_phone_presence
{
    /**
     * Executes check.
     * @param _telephone contact phone number to verify
     */
    contact_phone_presence(const Nullable<std::string>& _telephone);
    /**
     * Contact phone presents.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !absent;
    }
    bool absent : 1; ///< contact phone doesn't present
};

/**
 * Contact phone format verification.
 */
struct contact_phone_validity
{
    /**
     * Executes check.
     * @param _telephone contact phone number to verify
     */
    contact_phone_validity(const Nullable<std::string>& _telephone);
    /**
     * Contact phone is valid.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !invalid;
    }
    bool invalid : 1; ///< contact phone format fails to meet the requirements
};

/**
 * Contact phone availability verification.
 */
struct contact_phone_availability
{
    /**
     * Executes check.
     * @param _telephone contact phone number to verify
     * @param _id contact id
     * @param _ctx operation context used to check processing
     */
    contact_phone_availability(
            const Nullable<std::string>& _telephone,
            unsigned long long _id,
            LibFred::OperationContext& _ctx);
    /**
     * Contact phone is available for using in next identification request.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !(absent || used_recently);
    }
    bool absent : 1;       ///< contact phone doesn't present
    bool used_recently : 1; ///< contact phone used for identification request recently
};

/**
 * Contact fax format verification.
 */
struct contact_fax_validity
{
    /**
     * Executes check.
     * @param _fax contact fax number to verify
     */
    contact_fax_validity(const Nullable<std::string>& _fax);
    /**
     * Contact fax is valid.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !invalid;
    }
    bool invalid : 1; ///< contact fax format fails to meet the requirements
};

/// MojeId
namespace MojeId {

enum
{
    USERNAME_LENGTH_LIMIT = 30
};

/**
 * Regular expression which match correct mojeID contact handle.
 * @return pattern usable in boost::regex_match for checking correct username format
 */
extern const boost::regex username_pattern;

/**
 * MojeId contact handle verification.
 */
struct contact_username
{
    /**
     * Executes check.
     * @param _handle contact handle to verify
     */
    contact_username(const std::string& _handle);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !(absent || invalid);
    }
    bool absent : 1; ///< mojeID contact handle doesn't present
    bool invalid : 1; ///< mojeID contact handle format fails to meet the requirements
};

/**
 * MojeId contact handle verification.
 */
struct contact_username_availability
{
    /**
     * Executes check.
     * @param _handle contact handle to verify
     * @param _ctx operation context used to check processing
     */
    contact_username_availability(
            const std::string& _handle,
            LibFred::OperationContext& _ctx);
    /**
     * All checks successfully done.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !(taken || used_recently);
    }
    bool taken : 1;        ///< contact handle already exists
    bool used_recently : 1; ///< contact used recently, isn't available so far
};

/**
 * MojeId contact birthday verification.
 */
struct contact_birthday
{
    /**
     * Executes check.
     * @param _ssntype type of personal identification
     * @param _ssn personal identification to verify
     */
    contact_birthday(
            const Nullable<std::string>& _ssntype,
            const Nullable<std::string>& _ssn);
    /**
     * MojeId contact birthday presents and is correct.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !(absent || invalid);
    }
    bool absent : 1; ///< mojeID contact birthday doesn't present
    bool invalid : 1; ///< mojeID contact birthday format fails to meet the requirements
};

/**
 * MojeId contact birthday format verification.
 */
struct contact_birthday_validity
{
    /**
     * Executes check.
     * @param _ssntype type of personal identification
     * @param _ssn personal identification to verify
     */
    contact_birthday_validity(
            const Nullable<std::string>& _ssntype,
            const Nullable<std::string>& _ssn);
    /**
     * MojeId contact birthday is valid or doesn't present.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !invalid;
    }
    bool invalid : 1; ///< mojeID contact birthday format fails to meet the requirements
};

/**
 * MojeId contact vat_id presence checking.
 */
struct contact_vat_id_presence
{
    /**
     * Executes check.
     * @param _ssntype type of personal identification
     * @param _ssn personal identification to verify
     */
    contact_vat_id_presence(
            const Nullable<std::string>& _ssntype,
            const Nullable<std::string>& _ssn);
    /**
     * MojeId contact vat_id presents.
     * @return true if check was successfully
     */
    bool success() const
    {
        return !absent;
    }
    bool absent : 1; ///< mojeID contact vat_id doesn't present
};

} // Fred::Backend::GeneralCheck::MojeId
} // Fred::Backend::GeneralCheck

struct check_contact_name
    : GeneralCheck::contact_name
{
    check_contact_name(const LibFred::InfoContactData& _data)
        : GeneralCheck::contact_name(_data.name)
    {
    }
};

struct check_place_address
    : GeneralCheck::contact_address
{
    check_place_address(const LibFred::Contact::PlaceAddress& _data)
        : GeneralCheck::contact_address(
                  _data.street1,
                  _data.city,
                  _data.postalcode,
                  _data.country)
    {
    }
};

struct check_contact_mailing_address
    : check_place_address
{
    check_contact_mailing_address(const LibFred::InfoContactData& _data)
        : check_place_address(_data.get_address<LibFred::ContactAddressType::MAILING>())
    {
    }
};

struct check_contact_place_address
    : GeneralCheck::contact_optional_address
{
    check_contact_place_address(const LibFred::InfoContactData& _data);
    bool success() const
    {
        return !(absent || !this->GeneralCheck::contact_optional_address::success());
    }
    bool absent : 1; ///< contact place address doesn't present
};

struct check_contact_addresses
    : GeneralCheck::contact_optional_address
{
    check_contact_addresses(
            const LibFred::InfoContactData& _data,
            LibFred::ContactAddressType _address_type);
};

struct check_contact_addresses_mailing
    : check_contact_addresses
{
    check_contact_addresses_mailing(const LibFred::InfoContactData& _data)
        : check_contact_addresses(_data, LibFred::ContactAddressType::MAILING)
    {
    }
};

struct check_contact_addresses_billing
    : check_contact_addresses
{
    check_contact_addresses_billing(const LibFred::InfoContactData& _data)
        : check_contact_addresses(_data, LibFred::ContactAddressType::BILLING)
    {
    }
};

struct check_contact_addresses_shipping
    : check_contact_addresses
{
    check_contact_addresses_shipping(const LibFred::InfoContactData& _data)
        : check_contact_addresses(_data, LibFred::ContactAddressType::SHIPPING)
    {
    }
};

struct check_contact_addresses_shipping2
    : check_contact_addresses
{
    check_contact_addresses_shipping2(const LibFred::InfoContactData& _data)
        : check_contact_addresses(_data, LibFred::ContactAddressType::SHIPPING_2)
    {
    }
};

struct check_contact_addresses_shipping3
    : check_contact_addresses
{
    check_contact_addresses_shipping3(const LibFred::InfoContactData& _data)
        : check_contact_addresses(_data, LibFred::ContactAddressType::SHIPPING_3)
    {
    }
};

struct check_contact_email_presence
    : GeneralCheck::contact_email_presence
{
    check_contact_email_presence(const LibFred::InfoContactData& _data)
        : GeneralCheck::contact_email_presence(_data.email)
    {
    }
};

struct check_contact_email_validity
    : GeneralCheck::contact_email_validity
{
    check_contact_email_validity(const LibFred::InfoContactData& _data)
        : GeneralCheck::contact_email_validity(_data.email)
    {
    }
};

struct check_contact_email_availability
    : GeneralCheck::contact_email_availability
{
    check_contact_email_availability(
            const LibFred::InfoContactData& _data,
            LibFred::OperationContext& _ctx)
        : GeneralCheck::contact_email_availability(_data.email, _data.id, _ctx)
    {
    }
};

struct check_contact_notifyemail_validity
    : GeneralCheck::contact_notifyemail_validity
{
    check_contact_notifyemail_validity(const LibFred::InfoContactData& _data)
        : GeneralCheck::contact_notifyemail_validity(_data.notifyemail)
    {
    }
};

struct check_contact_phone_presence
    : GeneralCheck::contact_phone_presence
{
    check_contact_phone_presence(const LibFred::InfoContactData& _data)
        : GeneralCheck::contact_phone_presence(_data.telephone)
    {
    }
};

struct check_contact_phone_validity
    : GeneralCheck::contact_phone_validity
{
    check_contact_phone_validity(const LibFred::InfoContactData& _data)
        : GeneralCheck::contact_phone_validity(_data.telephone)
    {
    }
};

struct check_contact_phone_availability
    : GeneralCheck::contact_phone_availability
{
    check_contact_phone_availability(
            const LibFred::InfoContactData& _data,
            LibFred::OperationContext& _ctx)
        : GeneralCheck::contact_phone_availability(_data.telephone, _data.id, _ctx)
    {
    }
};

struct check_contact_fax_validity
    : GeneralCheck::contact_fax_validity
{
    check_contact_fax_validity(const LibFred::InfoContactData& _data)
        : GeneralCheck::contact_fax_validity(_data.fax)
    {
    }
};

/// MojeId
namespace MojeId {

struct check_contact_username
    : GeneralCheck::MojeId::contact_username
{
    check_contact_username(const LibFred::InfoContactData& _data)
        : GeneralCheck::MojeId::contact_username(_data.handle)
    {
    }
};

struct check_contact_username_availability
    : GeneralCheck::MojeId::contact_username_availability
{
    check_contact_username_availability(
            const LibFred::InfoContactData& _data,
            LibFred::OperationContext& _ctx)
        : GeneralCheck::MojeId::contact_username_availability(_data.handle, _ctx)
    {
    }
};

struct check_contact_birthday
    : GeneralCheck::MojeId::contact_birthday
{
    check_contact_birthday(const LibFred::InfoContactData& _data)
        : GeneralCheck::MojeId::contact_birthday(_data.ssntype, _data.ssn)
    {
    }
};

struct check_contact_birthday_validity
    : GeneralCheck::MojeId::contact_birthday_validity
{
    check_contact_birthday_validity(const LibFred::InfoContactData& _data)
        : GeneralCheck::MojeId::contact_birthday_validity(_data.ssntype, _data.ssn)
    {
    }
};

struct check_contact_vat_id_presence
    : GeneralCheck::MojeId::contact_vat_id_presence
{
    check_contact_vat_id_presence(const LibFred::InfoContactData& _data)
        : GeneralCheck::MojeId::contact_vat_id_presence(_data.ssntype, _data.ssn)
    {
    }
};

struct check_contact_ssn
{
    check_contact_ssn(const LibFred::InfoContactData& _data);
    bool success() const
    {
        return !(birthdate_absent || birthdate_invalid ||
                 vat_id_num_absent || vat_id_num_invalid);
    }
    bool birthdate_absent : 1;
    bool birthdate_invalid : 1;
    bool vat_id_num_absent : 1;
    bool vat_id_num_invalid : 1;
};

} // namespace Fred::Backend::MojeId
} // namespace Fred::Backend
} // namespace Fred

#endif//CHECKERS_HH_1C4A77A46C944F69B5D59D1B75B30116
