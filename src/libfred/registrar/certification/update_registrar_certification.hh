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

#ifndef UPDATE_REGISTRAR_CERTIFICATION_H_
#define UPDATE_REGISTRAR_CERTIFICATION_H_

#include "src/libfred/opcontext.hh"

#include <boost/date_time/posix_time/posix_time.hpp>

namespace LibFred {

class UpdateRegistrarCertification
{
private:
    unsigned long long m_certification_id;
    boost::gregorian::date m_valid_until;
    int m_classification;
    unsigned long long m_eval_file_id;
    bool m_valid_until_set;

public:
    UpdateRegistrarCertification(
            const unsigned long long _certification_id,
            const boost::gregorian::date _valid_until)
    : m_certification_id(_certification_id),
      m_valid_until(_valid_until),
      m_valid_until_set(true)
    {}

    UpdateRegistrarCertification(
            const unsigned long long _certification_id,
            const int _classification,
            const unsigned long long _eval_file_id)
    : m_certification_id(_certification_id),
      m_classification(_classification),
      m_eval_file_id(_eval_file_id),
      m_valid_until_set(false)
    {}

    void exec(OperationContext& ctx);
}; // UpdateRegistrarCertification

} // namespace Fred

#endif // UPDATE_REGISTRAR_CERTIFICATION_H_
