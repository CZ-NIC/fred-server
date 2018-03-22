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
 *  create registrar certification
 */

#ifndef CREATE_REGISTRAR_CERTIFICATION_HH_BDE927161AD44CEFAEDD7C35AE6E8C28
#define CREATE_REGISTRAR_CERTIFICATION_HH_BDE927161AD44CEFAEDD7C35AE6E8C28

#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {
namespace Registrar {

class CreateRegistrarCertification
{
public:
    CreateRegistrarCertification(
            const unsigned long long _registrar_id,
            const boost::gregorian::date& _valid_from,
            const boost::gregorian::date& _valid_until,
            const int _classification,
            const unsigned long long _eval_file_id)
    : registrar_id_(_registrar_id),
      valid_from_(_valid_from),
      valid_until_(_valid_until),
      classification_(_classification),
      eval_file_id_(_eval_file_id)
    {}

    unsigned long long exec(OperationContext& _ctx);

private:
    unsigned long long registrar_id_;
    boost::gregorian::date valid_from_;
    boost::gregorian::date valid_until_;
    int classification_;
    unsigned long long eval_file_id_;

};

} // namespace Registrar
} // namespace Fred

#endif
