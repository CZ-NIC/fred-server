#ifndef DATETIME_COMMON_
#define DATETIME_COMMON_

#include "common.h"

namespace Database {

/*
 * date time element used for math operators
 */

#define DATETIME_ELEMENT(_name, _type) \
SIMPLE_WRAPPED_TYPE(_name, _type, 0)

DATETIME_ELEMENT(Days, unsigned)
DATETIME_ELEMENT(Months, unsigned)
DATETIME_ELEMENT(Years, unsigned)
DATETIME_ELEMENT(Hours, unsigned)
DATETIME_ELEMENT(Minutes, unsigned)
DATETIME_ELEMENT(Seconds, unsigned)


/*
 * special values definition
 */

/* special values for constructing date/time */
enum DateTimeSpecial {
  NOW,         //< actual local date
  NEG_INF,     //< minus infinity
  POS_INF,      //< plus infinity
  NOW_UTC       //< actual UTC date
};


/* special values for construction intervals */
enum DateTimeIntervalSpecial {
  NONE,        //< no special value
  DAY,         //< specific day
  INTERVAL,    //< specific interval
  LAST_HOUR,   //< constant for hour +- offset from/to actual time
  LAST_DAY,    //< constant for day +- offset from/to actual time
  LAST_WEEK,   //< constant for week +- offset from/to actual time
  LAST_MONTH,  //< constant for month +- offset from/to actual time
  LAST_YEAR,   //< constant for year +- offset from/to actual time
  PAST_HOUR,   //< constant for hour +- offset until/since actual time
  PAST_DAY,    //< constant for day +- offset until/since actual time
  PAST_WEEK,   //< constant for week +- offset until/since actual time
  PAST_MONTH,  //< constant for month +- offset until/since actual time
  PAST_YEAR    //< constant for year +- offset until/since actual time
};

}

#endif

