#include "src/corba/util/corba_conversions_datetime.h"
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

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

    ccReg::DateType wrap_date(date in) {
        ccReg::DateType out;

        if (in.is_special()) {
            out.day = 0;
            out.month = 0;
            out.year = 0;
        } else {
            out.day    = in.day();
            out.month  = in.month();
            out.year   = in.year();
        }

        return out;
    }

    CORBA::String_var wrap_date_to_corba_string(boost::gregorian::date in)
    {
        if(in.is_special())
        {
            throw std::runtime_error("wrap_date_to_corba_string: invalid date");
        }
        return CORBA::string_dup(boost::gregorian::to_iso_extended_string(in).c_str());
    }

    CORBA::String_var wrap_ptime_to_corba_string(boost::posix_time::ptime in)
    {
        if(in.is_special())
        {
            throw std::runtime_error("wrap_ptime_to_corba_string: invalid posix time");
        }
        return CORBA::string_dup(boost::posix_time::to_iso_extended_string(in).c_str());
    }

}
