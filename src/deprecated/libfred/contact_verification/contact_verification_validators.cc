/*
 * Copyright (C) 2012-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/libfred/contact_verification/contact_verification_validators.hh"

namespace LibFred {
namespace Contact {
namespace Verification {


ContactValidator create_default_contact_validator()
{
    ContactValidator tmp;
    tmp.add_checker(contact_checker_name);
    tmp.add_checker(contact_checker_address_required);
    tmp.add_checker(contact_checker_email_format);
    tmp.add_checker(contact_checker_email_required);
    tmp.add_checker(contact_checker_phone_format);
    tmp.add_checker(contact_checker_notify_email_format);
    tmp.add_checker(contact_checker_fax_format);
    return tmp;
}


ContactValidator create_conditional_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_email_unique);
    tmp.add_checker(contact_checker_phone_required);
    tmp.add_checker(contact_checker_phone_unique);
    return tmp;
}


ContactValidator create_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    tmp.add_checker(contact_checker_email_unique);
    return tmp;
}


ContactValidator create_finish_identification_validator()
{
    ContactValidator tmp = create_default_contact_validator();
    return tmp;
}


}
}
}

