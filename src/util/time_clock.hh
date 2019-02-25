/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef TIME_CLOCK_HH_930B2A4B106847E9873B166F5C8936A9
#define TIME_CLOCK_HH_930B2A4B106847E9873B166F5C8936A9

#include <ctime>
#include <string>
#include <iostream>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/function.hpp>

//callback for implementation of the message print
typedef boost::function<void (const std::string& msg)> MessagePrint;

struct cerr_print
{
    void operator()(const std::string& msg)
    {
        std::cerr << msg << std::endl;
    }
};

struct cout_print
{
    void operator()(const std::string& msg)
    {
        std::cout << msg << std::endl;
    }
};

class ElapsedTime
{
    boost::posix_time::ptime start_;
    std::string label_;
    MessagePrint mprint_;
public:
    ElapsedTime()
    : start_(boost::posix_time::microsec_clock::universal_time())
    , label_("elapsed time: ")
    , mprint_(cout_print())
    {}
    ElapsedTime(const std::string& label)
    : start_(boost::posix_time::microsec_clock::universal_time())
    , label_(label)
    , mprint_(cout_print())
    {}
    ElapsedTime(const std::string& label, MessagePrint mprint)
    : start_(boost::posix_time::microsec_clock::universal_time())
    , label_(label)
    , mprint_(mprint)
    {}

    ~ElapsedTime()
    {
        mprint_(label_
                + boost::posix_time::to_iso_string(
                boost::posix_time::microsec_clock::universal_time()
                - start_));
    }
};//class ElapsedTime
													

class TimeStamp
{
public:
    static std::string microsec()
    {
        std::string time_string(
           boost::posix_time::to_iso_string(
                   boost::posix_time::microsec_clock::universal_time()));
           boost::algorithm::erase_all(time_string,",");
           boost::algorithm::erase_all(time_string,".");
           boost::algorithm::erase_all(time_string,"T");
        return time_string;
    }
};//class TimeStamp


class TimeClock {
protected:
  double start_;
  double stop_;

public:
  void reset() {
    start_ = stop_ = 0;
  }

  void start() {
    start_ = clock();
  }

  void stop() {
    stop_ = clock();
  }

  double time() const {
    return ((double)(stop_ - start_))/(CLOCKS_PER_SEC/1000);
  }
};



#endif /*TIME_CLOCK_H_*/
