#ifndef CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965

#include "src/corba/MojeID2.hh"
#include "src/corba/mojeid/corba_common_conversion2.h"

#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Corba {
namespace Conversion {

inline boost::gregorian::date& convert(const std::string &from, boost::gregorian::date &into)
{
    enum { GREGORIAN_DATE_LENGTH = 10 };
    const std::string str_date = from.substr(0, GREGORIAN_DATE_LENGTH);
    return into = boost::gregorian::from_simple_string(str_date);
}

template < >
struct from_into< Registry::MojeID::Date, boost::gregorian::date >
: from_into_base< Registry::MojeID::Date, boost::gregorian::date >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const
    {
        return dst = boost::gregorian::from_simple_string(src);
    }
};

template < >
struct into_from< Registry::MojeID::Date, boost::gregorian::date >
: into_from_base< Registry::MojeID::Date, boost::gregorian::date >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        return dst = CORBA::string_dup(boost::gregorian::to_iso_extended_string(src).c_str());
    }
};

template < >
struct from_into< Registry::MojeID::DateTime, boost::posix_time::ptime >
: from_into_base< Registry::MojeID::DateTime, boost::posix_time::ptime >
{
    dst_value_ref operator()(src_value src, dst_value_ref dst)const
    {
        std::string iso_extended = src;//2015-06-24T13:05:03.000123
        enum { T_POS = 10 };
        if ((T_POS <= iso_extended.length()) && (iso_extended[T_POS] == 'T')) {
            iso_extended[T_POS] = ' ';
        }
        return dst = boost::posix_time::time_from_string(iso_extended);
    }
};

template < >
struct into_from< Registry::MojeID::DateTime, boost::posix_time::ptime >
: into_from_base< Registry::MojeID::DateTime, boost::posix_time::ptime >
{
    dst_value_ref operator()(dst_value_ref dst, src_value src)const
    {
        return dst = CORBA::string_dup(boost::posix_time::to_iso_extended_string(src).c_str());
    }
};

}//Corba::Conversion
}//Corba

#endif//CORBA_CONVERSION2_H_909714E7B8BE860E87D51B0685D54965
