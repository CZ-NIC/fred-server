#ifndef REMINDER_H__
#define REMINDER_H__

#include <boost/date_time/gregorian/gregorian.hpp>
#include <stdexcept>

#include "mailer.h"


namespace Fred {


struct ReminderError : public std::runtime_error
{
    ReminderError(const std::string &_str) : std::runtime_error(_str) { }
};


void run_reminder(Mailer::Manager *_mailer,
                  const boost::gregorian::date &_date
                    = boost::gregorian::day_clock::universal_day());


}


#endif /*REMINDER_H__*/

