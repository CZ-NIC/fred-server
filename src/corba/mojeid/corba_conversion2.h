#ifndef CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965

#include "src/corba/MojeID2.hh"
#include "src/corba/mojeid/corba_common_conversion2.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/static_assert.hpp>

namespace Registry_MojeID_Date_and_DateTime_conditions
{
BOOST_STATIC_ASSERT(( boost::is_same< Registry::MojeID::Date,     char* >::value ));
BOOST_STATIC_ASSERT(( boost::is_same< Registry::MojeID::DateTime, char* >::value ));
BOOST_STATIC_ASSERT(( boost::is_same< Registry::MojeID::Date,     Registry::MojeID::DateTime >::value ));
}

template < >
struct corba_type_conversion< char* > // Registry::MojeID::Date, Registry::MojeID::DateTime
: corba_type_conversion_base< char* >
{
    static corba_type& set(corba_type &dst, const boost::gregorian::date &src)
    {
        return dst = CORBA::string_dup(boost::gregorian::to_iso_extended_string(src).c_str());
    }
    static boost::gregorian::date& set(boost::gregorian::date &dst, const corba_type &src)
    {
        return dst = boost::gregorian::from_simple_string(src);
    }
    static corba_type& set(corba_type &dst, const std::string &src)
    {
        enum { GREGORIAN_DATE_LENGTH = 10 };
        const std::string str_date = src.substr(0, GREGORIAN_DATE_LENGTH);
        return corba_value(dst).set(boost::gregorian::from_simple_string(str_date));
    }
    static std::string& set(std::string &dst, const corba_type &src)
    {
        return dst = boost::gregorian::to_iso_extended_string(corba_value(src).get< boost::gregorian::date >());
    }
    static corba_type& set(corba_type &dst, const boost::posix_time::ptime &src)
    {
        return dst = CORBA::string_dup(boost::posix_time::to_iso_extended_string(src).c_str());
    }
    static boost::posix_time::ptime& set(boost::posix_time::ptime &dst, const corba_type &src)
    {
        std::string iso_extended = src;//2015-06-24T13:05:03.000123
        enum { T_POS = 10 };
        if ((T_POS <= iso_extended.length()) && (iso_extended[T_POS] == 'T')) {
            iso_extended[T_POS] = ' ';
        }
        return dst = boost::posix_time::time_from_string(iso_extended);
    }
};

#endif//CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965
