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

#ifndef CREATE_REGISTRAR_CERTIFICATION_H_
#define CREATE_REGISTRAR_CERTIFICATION_H_

#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {

class CreateRegistrarCertification
{
private:
    unsigned long long m_registrar_id;
    boost::gregorian::date m_valid_from;
    boost::gregorian::date m_valid_until;
    int m_classification;
    unsigned long long m_eval_file_id;

public:
    CreateRegistrarCertification(
            const unsigned long long _registrar_id,
            const boost::gregorian::date& _valid_from,
            const boost::gregorian::date& _valid_until,
            const int _classification,
            const unsigned long long _eval_file_id)
    : m_registrar_id(_registrar_id),
      m_valid_from(_valid_from),
      m_valid_until(_valid_until),
      m_classification(_classification),
      m_eval_file_id(_eval_file_id)
    {}

    unsigned long long exec(OperationContext& ctx);

}; // CreateRegistrarCertification

} // namespace Fred

#endif // CREATE_REGISTRAR_CERTIFICATION_H_
