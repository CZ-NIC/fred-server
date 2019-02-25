/*
 * Copyright (C) 2018-2019  CZ.NIC, z. s. p. o.
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
#ifndef LOCK_REQUEST_TYPE_HH_5E65B7E5618F40E4855A1E1199BEE4DF
#define LOCK_REQUEST_TYPE_HH_5E65B7E5618F40E4855A1E1199BEE4DF

namespace Fred {
namespace Backend {
namespace PublicRequest {

struct LockRequestType
{
    enum Enum
    {
        block_transfer,
        block_transfer_and_update,
        unblock_transfer,
        unblock_transfer_and_update
    };
};

} // namespace Fred::Backend::PublicRequest
} // namespace Fred::Backend
} // namespace Fred

#endif
