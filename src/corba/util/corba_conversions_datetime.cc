#include "corba/util/corba_conversions_datetime.h"

namespace Corba {
    ccReg::DateTimeType wrap_time(ptime in) {
        ccReg::DateTimeType out;

        if (in.is_special()) {
            out.date.day = 0;
            out.date.month = 0;
            out.date.year = 0;
            out.hour = 0;
            out.minute = 0;
            out.second = 0;
        } else {
            out.date.day    = in.date().day();
            out.date.month  = in.date().month();
            out.date.year   = in.date().year();
            out.hour        = in.time_of_day().hours();
            out.minute      = in.time_of_day().minutes();
            out.second      = in.time_of_day().seconds();
        }

        return out;
    }
}
