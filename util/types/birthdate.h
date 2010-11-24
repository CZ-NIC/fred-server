/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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
 *  @file birthdate.h
 *  header of birth day conversion from string to boost date
 */


#ifndef BIRTHDATE_H_
#define BIRTHDATE_H_

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

//try recognize birth date in input string and convert to boost date
boost::gregorian::date birthdate_from_string_to_date(std::string birthdate);

#endif /* BIRTHDATE_H_ */
