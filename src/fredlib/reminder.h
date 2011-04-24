#ifndef REMINDER_H__
#define REMINDER_H__

#include <boost/date_time/gregorian/gregorian.hpp>

namespace Fred {


void run_reminder(const boost::gregorian::date &_date
                    = boost::gregorian::day_clock::universal_day());


}


#endif /*REMINDER_H__*/

