/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/contact/contact_change.hh"

#include "src/backend/epp/contact/util.hh"

namespace Epp {
namespace Contact {

ContactChange ContactChange::get_trimmed_copy()const
{
    ContactChange dst;
    dst.name = trim(this->name);
    dst.organization = trim(this->organization);
    dst.address = trim(this->address);
    dst.mailing_address = trim(this->mailing_address);
    dst.telephone = trim(this->telephone);
    dst.fax = trim(this->fax);
    dst.email = trim(this->email);
    dst.notify_email = trim(this->notify_email);
    dst.vat = trim(this->vat);
    dst.ident = trim(this->ident);
    dst.authinfopw = this->authinfopw;
    dst.disclose = this->disclose;
    return dst;
}

} // namespace Epp::Contact
} // namespace Epp
