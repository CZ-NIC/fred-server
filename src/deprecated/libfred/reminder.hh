/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#ifndef REMINDER_HH_F21022EA59C544D5924D2228DDC4A8DF
#define REMINDER_HH_F21022EA59C544D5924D2228DDC4A8DF

#include <boost/date_time/gregorian/gregorian.hpp>
#include <stdexcept>

#include "libfred/mailer.hh"


namespace LibFred {


struct ReminderError : public std::runtime_error
{
    ReminderError(const std::string &_str) : std::runtime_error(_str) { }
};


void run_reminder(Mailer::Manager *_mailer,
                  const boost::gregorian::date &_date
                    = boost::gregorian::day_clock::universal_day());


}


#endif /*REMINDER_H__*/

