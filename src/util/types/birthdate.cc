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
 *  @file birthdate.cc
 *  impl of birth day conversion from string to boost date
 */



#include "src/util/types/birthdate.hh"

#include <string>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <stdexcept>

//try recognize birth date in input string and convert to boost date
boost::gregorian::date birthdate_from_string_to_date(std::string birthdate)
{
    using namespace boost::xpressive;

    try
    {

    static sregex re_space = sregex::compile("\\s+");//spaces regex

    //current year, check birthdate in future
    int cy = boost::posix_time::second_clock::local_time().date().year();


    std::string birthdatenospaces
        = regex_replace(birthdate, re_space,std::string(""));//ignore spaces

    smatch match_result;//output of regex_match

    //try yyyy-mm-dd or yyyy/mm/dd
    static sregex re_date_yyyy_mm_dd
        = sregex::compile("^(\\d{4})([/-])(\\d{1,2})\\2(\\d{1,2})$");
    if(regex_match( birthdatenospaces, match_result, re_date_yyyy_mm_dd ))
    {
        return boost::gregorian::from_string(match_result[0]);
    }//if re_date_yyyy_mm_dd

    //try yyyymmdd or ddmmyyyy
    static sregex re_date_dddddddd
        = sregex::compile("^(\\d{2})(\\d{2})(\\d{2})(\\d{2})$");
    if(regex_match( birthdatenospaces, match_result, re_date_dddddddd ))
    {

        int v1_yyyy = boost::lexical_cast<int>(
                    std::string(match_result[1]) + std::string(match_result[2]));
        int v1_mm = boost::lexical_cast<int>(match_result[3]);
        int v1_dd = boost::lexical_cast<int>(match_result[4]);

        if ((v1_yyyy >=1900) && (v1_yyyy <=cy)
                && (v1_mm >= 1) && (v1_mm <= 12)
                && (v1_dd >= 1) && (v1_dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(match_result[0]);
        }

        int v2_yyyy = boost::lexical_cast<int>(
                    std::string(match_result[3]) + std::string(match_result[4]));
        int v2_mm = boost::lexical_cast<int>(match_result[2]);
        int v2_dd = boost::lexical_cast<int>(match_result[1]);

        if ((v2_yyyy >=1900) && (v2_yyyy <=cy)
                && (v2_mm >= 1) && (v2_mm <= 12)
                && (v2_dd >= 1) && (v2_dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(
                std::string(match_result[3])+std::string(match_result[4])+std::string(match_result[2])+std::string(match_result[1])
                );
        }

        throw std::runtime_error("birthdate error: invalid birthdate dddddddd");

    }//if re_date_dddddddd

    //try ddmyyyy
    static sregex re_date_ddddddd
        = sregex::compile("^(\\d{2})(\\d{1})(\\d{4})$");
    if(regex_match( birthdatenospaces, match_result, re_date_ddddddd ))
    {

        int yyyy = boost::lexical_cast<int>(std::string(match_result[3]));
        int mm = boost::lexical_cast<int>(match_result[2]);
        int dd = boost::lexical_cast<int>(match_result[1]);

        if ((yyyy >=1900) && (yyyy <=cy)
                && (mm >= 1) && (mm <= 12)
                && (dd >= 1) && (dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(
                std::string(match_result[3])+"0"+std::string(match_result[2])+std::string(match_result[1])
                );
        }

        throw std::runtime_error("birthdate error: invalid birthdate ddddddd");

    }//if re_date_ddddddd

    //try dd.mm.yyyy or dd/mm/yyyy or dd-mm-yyyy
    static sregex re_date_dd_mm_yyyy
        = sregex::compile("^(\\d{1,2})([\\./-])(\\d{1,2})\\2(\\d{4})$");
    if(regex_match( birthdatenospaces, match_result, re_date_dd_mm_yyyy ))
    {
        int yyyy = boost::lexical_cast<int>(match_result[4]);
        int mm = boost::lexical_cast<int>(match_result[3]);
        int dd = boost::lexical_cast<int>(match_result[1]);

        if ((yyyy >=1900) && (yyyy <=cy)
                && (mm >= 1) && (mm <= 12)
                && (dd >= 1) && (dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(
            str(boost::format("%|04d|%|02d|%|02d|")
            % match_result[4]
            % match_result[3]
            % match_result[1])
            );
        }

        throw std::runtime_error("birthdate error: invalid birthdate dd_mm_yyyy");

    }//if re_date_dd_mm_yyyy

    //try yymmdd or ddmmyy
    static sregex re_date_dddddd
        = sregex::compile("^(\\d{2})(\\d{2})(\\d{2})$");
    if(regex_match( birthdatenospaces, match_result, re_date_dddddd ))
    {
        //yymmdd
        int yy = boost::lexical_cast<int>(match_result[1]);
        int mm = boost::lexical_cast<int>(match_result[2]);
        int dd = boost::lexical_cast<int>(match_result[3]);

        //current year
        int cy1 = cy/100;
        int cy2 = cy%100;

        if(yy > cy2 )
        {
            yy = yy + ((cy1 - 1) * 100);
        }
        else
        {//yy<= cy2
            yy = yy + (cy1 * 100);
        }

        if ((yy >=1900) && (yy <=2010)//for legacy data only
                && (mm >= 1) && (mm <= 12)
                && (dd >= 1) && (dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(
                str(boost::format("%|04d|%|02d|%|02d|")
                % yy % mm % dd)
                );
        }

        //ddmmyy
        yy = boost::lexical_cast<int>(match_result[3]);
        mm = boost::lexical_cast<int>(match_result[2]);
        dd = boost::lexical_cast<int>(match_result[1]);

        if(yy > cy2 )
        {
            yy = yy + ((cy1 - 1) * 100);
        }
        else
        {//yy<= cy2
            yy = yy + (cy1 * 100);
        }

        if ((yy >=1900) && (yy <=2010)//for legacy data only
                && (mm >= 1) && (mm <= 12)
                && (dd >= 1) && (dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(
                str(boost::format("%|04d|%|02d|%|02d|")
                % yy % mm % dd)
                );
        }

        throw std::runtime_error("birthdate error: invalid birthdate dddddd");

    }//if re_date_dddddd


    //try yy_mm_dd or dd_mm_yy
    static sregex re_date_dd_dd_dd
        = sregex::compile("^(\\d{1,2})([\\./-])(\\d{1,2})\\2(\\d{1,2})$");
    if(regex_match( birthdatenospaces, match_result, re_date_dd_dd_dd ))
    {
        //yymmdd
        int yy = boost::lexical_cast<int>(match_result[1]);
        int mm = boost::lexical_cast<int>(match_result[3]);
        int dd = boost::lexical_cast<int>(match_result[4]);

        //current year
        int cy1 = cy/100;
        int cy2 = cy%100;

        if(yy > cy2 )
        {
            yy = yy + ((cy1 - 1) * 100);
        }
        else
        {//yy<= cy2
            yy = yy + (cy1 * 100);
        }

        if ((yy >=1900) && (yy <=2010)//for legacy data only
                && (mm >= 1) && (mm <= 12)
                && (dd >= 1) && (dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(
                str(boost::format("%|04d|%|02d|%|02d|")
                % yy % mm % dd)
                );
        }

        //dd_mm_yy
        yy = boost::lexical_cast<int>(match_result[4]);
        mm = boost::lexical_cast<int>(match_result[3]);
        dd = boost::lexical_cast<int>(match_result[1]);

        if(yy > cy2 )
        {
            yy = yy + ((cy1 - 1) * 100);
        }
        else
        {//yy<= cy2
            yy = yy + (cy1 * 100);
        }

        if ((yy >=1900) && (yy <=2010)//for legacy data only
                && (mm >= 1) && (mm <= 12)
                && (dd >= 1) && (dd <= 31))
        {
            return boost::gregorian::from_undelimited_string(
                str(boost::format("%|04d|%|02d|%|02d|")
                % yy % mm % dd)
                );
        }

        throw std::runtime_error("birthdate error: invalid birthdate dd_dd_dd");

    }//if re_date_dd_dd_dd

    //try yyyymmdd000000
    return  boost::gregorian::from_undelimited_string(birthdatenospaces);

    }//try
    catch(std::exception&)
    {
        return boost::gregorian::date();
    }
}
