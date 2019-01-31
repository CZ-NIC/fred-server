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

