/*
 *  Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef GET_PERSONAL_ID_UNION_HH_7CBFA61B4054B6A9A1F59DC2D24B3D4C//date "+%s.%N"|md5sum|tr "[a-f]" "[A-F]"
#define GET_PERSONAL_ID_UNION_HH_7CBFA61B4054B6A9A1F59DC2D24B3D4C

#include "src/backend/epp/contact/contact_ident.hh"
#include "src/libfred/registrable_object/contact/info_contact_data.hh"

namespace Epp {
namespace Contact {
namespace Impl {

LibFred::PersonalIdUnion get_personal_id_union(const ContactIdent& ident);

}//namespace Epp::Contact::Impl
}//namespace Epp::Contact
}//namespace Epp

#endif//GET_PERSONAL_ID_UNION_HH_7CBFA61B4054B6A9A1F59DC2D24B3D4C
