/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file random_data_generator.h
 *  RandomDataGenerator class using boost
 */


#ifndef RANDOM_DATA_GENERATOR_HH_94BF03B65AC44513BB65FE985FA1311A
#define RANDOM_DATA_GENERATOR_HH_94BF03B65AC44513BB65FE985FA1311A

#include <boost/random/mersenne_twister.hpp>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/variate_generator.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string.hpp>
#include <exception>
#include <string>
#include <queue>
#include <sys/time.h>
#include <time.h>


/**
 * \class RandomDataGenerator
 * \brief pseudo-random data generator
 */

class RandomDataGenerator //data generator is actually pseudo-random
{
    //typedef for a random number generator
    //boost::mt19937, boost::ecuyer1988, boost::minstd_rand
    typedef boost::mt19937 base_generator_t;

    //uniform random number distribution of values in given range
    typedef boost::uniform_int<> int_distribution_t;
    typedef boost::uniform_real<> real_distribution_t;

    unsigned seed_;
    base_generator_t rng;
    int_distribution_t gen_letter52;//both case letters
    int_distribution_t gen_numletter10;//any number
    int_distribution_t gen_letter62;//both case letters or any number
    int_distribution_t gen_num1_5;//number 1-5
    int_distribution_t gen_num1_6;//number 1-6
    int_distribution_t gen_int;//signed integer
    real_distribution_t gen_real;//signed double
    int_distribution_t gen_time;//from 1.1.1990 to the end of unix time (19.01.2038 04:14:07 (CET))
    boost::variate_generator<base_generator_t, int_distribution_t > letter52;
    boost::variate_generator<base_generator_t, int_distribution_t > numletter10;
    boost::variate_generator<base_generator_t, int_distribution_t > letter62;
    boost::variate_generator<base_generator_t, int_distribution_t > num1_5;
    boost::variate_generator<base_generator_t, int_distribution_t > num1_6;
    boost::variate_generator<base_generator_t, int_distribution_t > gint;
    boost::variate_generator<base_generator_t, real_distribution_t > greal;
    boost::variate_generator<base_generator_t, int_distribution_t > gtime;

public:
    //ctor
    RandomDataGenerator(unsigned seed = 0)
        : seed_(seed ? seed : msseed())
          , rng(seed_)

          //ranges definitions
        , gen_letter52(0,51)
        , gen_numletter10(0,9)
        , gen_letter62(0,62)
        , gen_num1_5(1,5)
        , gen_num1_6(1,6)
        , gen_int(std::numeric_limits<int>::min(), std::numeric_limits<int>::max())
        , gen_real(std::numeric_limits<double>::min(), std::numeric_limits<double>::max())
        , gen_time(631148400, 2147483647)


          //generator instances
        , letter52(rng, gen_letter52)
        , numletter10(rng, gen_numletter10)
        , letter62(rng,gen_letter62)
        , num1_5(rng, gen_num1_5)
        , num1_6(rng, gen_num1_6)
        , gint(rng, gen_int)
        , greal(rng, gen_real)
        , gtime(rng, gen_time)
    {
        //std::cout << "RandomDataGenerator using seed: " << seed_ << std::endl;
    }

    ///used seed getter
    unsigned get_seed() const
    {
        return seed_;
    }

    ///generate some letter A-Z a-z
    char xletter()
    {
        unsigned rnumber = letter52();
        char ret = rnumber < 26
                ? static_cast<char>(rnumber + 65) //A-Z
                : static_cast<char>(rnumber + 71) ; //a-z
        return ret;
    }

    ///generate some letter 0-9
    char xnumletter()
    {
        return static_cast<char>(numletter10() + 48);
    }

    ///generate some letter A-Z a-z 0-9
    char xnletter()
    {
        unsigned rnumber = letter62();
        char ret = rnumber < 26
            ? static_cast<char>(rnumber + 65) //A-Z
            : ( rnumber < 52
                ? static_cast<char>(rnumber + 71) //a-z
                : static_cast<char>(rnumber + 48) //0-9
              );
        return ret;
    }


    ///generate some string of given length
    std::string xstring(std::size_t length)
    {
        std::string ret;
        ret.reserve(length);
        for(std::size_t i = 0; i < length; ++i)
            ret.push_back(xletter());
        return ret;
    }

    ///generate some string of given length
    std::string xnumstring(std::size_t length)
    {
        std::string ret;
        ret.reserve(length);
        for(std::size_t i = 0; i < length; ++i)
            ret.push_back(xnumletter());
        return ret;
    }

    ///generate some string of given length
    std::string xnstring(std::size_t length)
    {
        std::string ret;
        ret.reserve(length);
        for(std::size_t i = 0; i < length; ++i)
            ret.push_back(xnletter());
        return ret;
    }


    ///generate signed integer 1-5
    int xnum1_5()
    {
        return num1_5();
    }

    ///generate signed integer 1-6
    int xnum1_6()
    {
        return num1_6();
    }

    ///generate some signed integer
    int xint()
    {
        return gint();
    }

    ///generate some unsigned integer
    unsigned xuint()
    {
        return (1U << std::numeric_limits<int>::digits) + gint();
    }

    unsigned long msseed()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_usec + tv.tv_sec;
    }
   ///generate some signed real number
   double xreal()
   {
       return greal();
   }
   ///generate some unix time from 1.1.1990 to the end of unix time (19.01.2038 04:14:07 (CET))
   time_t xtime()
   {
       return gtime();
   }
   ///generate some posix time from 1.1.1990 to the end of unix time (19.01.2038 04:14:07 (CET))
   boost::posix_time::ptime xptime()
   {
       return boost::posix_time::from_time_t(gtime());
   }

   ///generate some gregorian date from 1.1.1990 to the end of unix time (19.01.2038)
   boost::gregorian::date xdate()
   {
       return xptime().date();
   }

};//class RandomDataGenerator


#endif
