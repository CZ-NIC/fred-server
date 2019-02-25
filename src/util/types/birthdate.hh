/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file birthdate.h
 *  header of birth day conversion from string to boost date
 */


#ifndef BIRTHDATE_HH_BE36FEFEFE5D47FDBEB617AC67BC878E
#define BIRTHDATE_HH_BE36FEFEFE5D47FDBEB617AC67BC878E

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>

//try recognize birth date in input string and convert to boost date
boost::gregorian::date birthdate_from_string_to_date(std::string birthdate);

#endif /* BIRTHDATE_H_ */
