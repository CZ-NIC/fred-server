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

/**
 *  @file
 *  update registrar certification
 */

#ifndef UPDATE_REGISTRAR_CERTIFICATION_HH_4BE6201D6B1B476795A61FD9C4042041
#define UPDATE_REGISTRAR_CERTIFICATION_HH_4BE6201D6B1B476795A61FD9C4042041

#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {
namespace Registrar {

class UpdateRegistrarCertification
{
public:
    UpdateRegistrarCertification(
            unsigned long long _certification_id,
            boost::gregorian::date _valid_until)
    : certification_id_(_certification_id),
      valid_until_(_valid_until)
    {}

    UpdateRegistrarCertification(
            unsigned long long _certification_id,
            int _classification,
            unsigned long long _eval_file_id)
    : certification_id_(_certification_id),
      classification_(_classification),
      eval_file_id_(_eval_file_id)
    {}

    void exec(OperationContext& _ctx);

private:
    unsigned long long certification_id_;
    boost::optional<boost::gregorian::date> valid_until_;
    boost::optional<int> classification_;
    boost::optional<unsigned long long> eval_file_id_;
};

} // namespace Registrar
} // namespace LibFred

#endif
