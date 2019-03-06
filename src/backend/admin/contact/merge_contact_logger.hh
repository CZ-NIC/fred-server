/*
 * Copyright (C) 2013-2019  CZ.NIC, z. s. p. o.
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
#ifndef MERGE_CONTACT_LOGGER_HH_B9BCF17767FA4A008A433FCC2C549D0F
#define MERGE_CONTACT_LOGGER_HH_B9BCF17767FA4A008A433FCC2C549D0F

#include "libfred/registrable_object/contact/merge_contact.hh"
#include "src/deprecated/libfred/logger_client.hh"


unsigned long long logger_merge_contact_create_request(
        LibFred::Logger::LoggerClient &_logger_client,
        const std::string &_src_contact,
        const std::string &_dst_contact);



void logger_merge_contact_close_request_success(
        LibFred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const LibFred::MergeContactOutput &_merge_data);



void logger_merge_contact_close_request_fail(
        LibFred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id);



#endif /*MERGE_CONTACT_LOGGER_H__*/

