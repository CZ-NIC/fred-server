/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
#ifndef REGISTRAR_CERTIFICATION_HH_41E9029ADF8B47A7B604F087420D5518
#define REGISTRAR_CERTIFICATION_HH_41E9029ADF8B47A7B604F087420D5518

#include "libfred/opcontext.hh"
#include <string>
#include <vector>

namespace Fred {
namespace Backend {
namespace Whois {

class RegistrarCertificationData
{
    std::string registrar_handle_;
    short score_;
    unsigned long long evaluation_file_id_;

public:
    RegistrarCertificationData()
        : score_(0), evaluation_file_id_(0)
    {
    }

    RegistrarCertificationData(
            const std::string& registrar_handle,
            short score,
            unsigned long long evaluation_file_id)
        : registrar_handle_(registrar_handle), score_(score), evaluation_file_id_(evaluation_file_id)
    {
    }

    std::string get_registrar_handle() const
    {
        return registrar_handle_;
    }

    short get_registrar_score() const
    {
        return score_;
    }

    unsigned long long get_registrar_evaluation_file_id() const
    {
        return evaluation_file_id_;
    }
};

//list of current registrar certification data
std::vector<RegistrarCertificationData> get_registrar_certifications(
        LibFred::OperationContext& ctx);

} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
