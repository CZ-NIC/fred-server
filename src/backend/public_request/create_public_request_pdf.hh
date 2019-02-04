/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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

#ifndef CREATE_PUBLIC_REQUEST_PDF_HH_3F9408F88CAC4028980B1973D4BE1E9A
#define CREATE_PUBLIC_REQUEST_PDF_HH_3F9408F88CAC4028980B1973D4BE1E9A

#include "src/backend/buffer.hh"
#include "src/backend/public_request/language.hh"
#include "src/backend/public_request/object_type.hh"
#include "src/deprecated/libfred/documents.hh"
#include "libfred/opcontext.hh"
#include "util/optional_value.hh"

#include <string>

namespace Fred {
namespace Backend {
namespace PublicRequest {

Fred::Backend::Buffer create_public_request_pdf(
        unsigned long long public_request_id,
        Language::Enum lang,
        std::shared_ptr<LibFred::Document::Manager> manager);

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
