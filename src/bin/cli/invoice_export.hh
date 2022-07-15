/*
 * Copyright (C) 2022  CZ.NIC, z. s. p. o.
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

#ifndef INVOICE_EXPORT_HH_0A79B09FCC63406EAC11FBB832F3FF27
#define INVOICE_EXPORT_HH_0A79B09FCC63406EAC11FBB832F3FF27

#include "src/bin/cli/fileman_params.hh"
#include "src/bin/cli/messenger_params.hh"
#include "src/bin/cli/secretary_params.hh"

namespace Admin {

void invoice_export(
            const MessengerArgs& _messenger_args,
            const FilemanArgs& _fileman_args,
            const SecretaryArgs& _secretary_args,
            bool _invoice_dont_send,
            unsigned long long _invoice_id,
            bool _debug_context);

void invoice_export_list(
            const FilemanArgs& _fileman_args,
            int limit,
            unsigned long long _invoice_id);

} // namespace Admin

#endif
