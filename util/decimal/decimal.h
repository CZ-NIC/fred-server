/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
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
 *  @decimal.h
 *  decimal floating point type wrapper
 */

#ifndef DECIMAL_H_
#define DECIMAL_H_

#include <stdexcept>
#include <sstream>
#include <string>
#include <boost/operators.hpp>
#include <boost/lexical_cast.hpp>
#include "util/decimal/mpdecimal-2.2/mpdecimal.h"

class decimal_exception_handler
{
    std::string message_prefix;
    bool rethrow;
public:
    virtual ~decimal_exception_handler(){}

    decimal_exception_handler(const char* _message_prefix
        , bool _rethrow = true)
    : message_prefix(_message_prefix)
    , rethrow(_rethrow)
    {}
    void operator ()()
    {
        try
        {
            throw;
        }
        catch (const std::exception &ex)
        {
            if(rethrow)
            {
                throw std::runtime_error(
                        message_prefix+" std::exception: " + ex.what());
            }
        }
        catch (...)
        {
            if(rethrow)
            {
                throw std::runtime_error(message_prefix+" unknown exception");
            }
        }
    }//operator()
};//decimal_exception_handler

template <class T> std::string decode_status(const T arg)
{
    std::string ret;
    if( arg & MPD_Clamped ) ret += " MPD_Clamped ";
    if( arg & MPD_Conversion_syntax ) ret += " MPD_Conversion_syntax ";
    if( arg & MPD_Division_by_zero ) ret += " MPD_Division_by_zero ";
    if( arg & MPD_Division_impossible ) ret += " MPD_Division_impossible ";
    if( arg & MPD_Division_undefined ) ret += " MPD_Division_undefined ";
    if( arg & MPD_Fpu_error ) ret += " MPD_Fpu_error ";
    if( arg & MPD_Inexact ) ret += " MPD_Inexact ";
    if( arg & MPD_Invalid_context ) ret += " MPD_Invalid_context ";
    if( arg & MPD_Invalid_operation ) ret += " MPD_Invalid_operation ";
    if( arg & MPD_Malloc_error ) ret += " MPD_Malloc_error ";
    if( arg & MPD_Not_implemented ) ret += " MPD_Not_implemented ";
    if( arg & MPD_Overflow ) ret += " MPD_Overflow ";
    if( arg & MPD_Rounded ) ret += " MPD_Rounded ";
    if( arg & MPD_Subnormal ) ret += " MPD_Subnormal ";
    if( arg & MPD_Underflow ) ret += " MPD_Underflow ";
    //if( arg & MPD_Max_status ) ret += " MPD_Max_status ";
    return ret;
}

static std::string string_dump_mpd_context(const mpd_context_t *ctx)
{
    std::stringstream sstr;
    sstr << "mpdecimal mpd_context dump -- precision: " << ctx->prec
            << " max positive exp: " << ctx->emax
            << " min negative exp: " << ctx->emin
            << " status events that should be trapped: " << ctx->traps
            << decode_status(ctx->traps)
            << " status flags: " << ctx->status
            << decode_status(ctx->status)
            << " newtrap: " << ctx->newtrap
            << decode_status(ctx->newtrap)
            << " rounding mode: " << ctx->round
            << " clamp mode: " << ctx->clamp
            << " all functions correctly rounded: " << ctx->allcr
            ;
    return sstr.str();
}

static void
ex_traphandler(const mpd_context_t *ctx, const mpd_t *pvalue)
{
    std::string value;//in pvalue
    if (pvalue == 0)  value += " pvalue is null";//pvalue might be 0
    else if (pvalue->len == 0) value += " pvalue is empty";//pvalue might be empty
    else value += mpd_to_sci(pvalue, 1);//pvalue might have value
    throw std::runtime_error(string_dump_mpd_context(ctx) + " pvalue: " + value );
}


class Decimal
: boost::operators< Decimal >
{
    //postgresql numeric type limit
    static const std::size_t default_precision = 1000;
    void (* traphandler)(const mpd_context_t *, const mpd_t *);
    mpd_context_t ctx;
    mpd_t *pvalue;

    //throw exception from trap handler
    //with context and value
    void check_status_exception(const mpd_context_t *ctx
            , const mpd_t *pvalue, const uint32_t flags) const
    {
        mpd_context_t ctx_copy = *ctx;

        ctx_copy.status |= flags;
        if (flags&ctx->traps)
        {
            ctx_copy.newtrap = (flags&ctx_copy.traps);
            traphandler(&ctx_copy, pvalue);
        }

    }

    void init(const std::size_t precision)
    {
        //throw exception with context and value
        traphandler = ex_traphandler;

        //set default context configuration
        mpd_defaultcontext(&ctx);

        //set traps
        if(mpd_qsettraps(&ctx, MPD_Invalid_context) == 0)
            throw std::runtime_error("MPD_Invalid_context > MPD_Max_status");
        if(mpd_qsettraps(&ctx, MPD_Malloc_error) == 0)
            throw std::runtime_error("MPD_Malloc_error > MPD_Max_status");
        if(mpd_qsettraps(&ctx, MPD_Conversion_syntax) == 0)
            throw std::runtime_error("MPD_Conversion_syntax > MPD_Max_status");
        if(mpd_qsettraps(&ctx, MPD_Invalid_operation) == 0)
            throw std::runtime_error("MPD_Invalid_operation > MPD_Max_status");
        if(mpd_qsettraps(&ctx, MPD_Division_undefined) == 0)
            throw std::runtime_error("MPD_Division_undefined > MPD_Max_status");
        if(mpd_qsettraps(&ctx, MPD_Division_by_zero) == 0)
            throw std::runtime_error("MPD_Division_by_zero > MPD_Max_status");

        //set precision
        if (!mpd_qsetprec(&ctx, precision))
        {
            check_status_exception(&ctx, pvalue, MPD_Invalid_context);
            return;
        }

        //init alloc only once
        static int init_is_set = 0;
        if(init_is_set == 0)
        {
            mpd_setminalloc(MPD_MINALLOC_MAX);
            init_is_set = 1;
        }

    }

    void alloc_pvalue()
    {
        if(pvalue == 0)
        {
            pvalue = mpd_qnew();
            if (pvalue == 0)
            {
                if(mpd_qsettraps(&ctx, MPD_Malloc_error) == 0)
                    throw std::runtime_error("MPD_Malloc_error > MPD_Max_status");

                check_status_exception(&ctx,pvalue, MPD_Malloc_error);
            }
        }
        else
            throw std::runtime_error("alloc_pvalue: pvalue not 0");
    }

public:
    unsigned long get_precision()
    {
        return mpd_getprec(&ctx);
    }

    void set_string(const char *str)
    {
        try
        {
            uint32_t status = 0;
            mpd_qset_string(pvalue, str, &ctx, &status);
            check_status_exception(&ctx, pvalue, status);
        }
        catch(...)
        {
            const std::size_t msg_len = 2048;
            char msg[msg_len]={0};
            snprintf(msg, msg_len
                    ,"set_string(const char *str: %s ) exception"
                    , str);
            decimal_exception_handler(msg, true)();
        }
    }

    bool is_special() const
    {
        return mpd_isspecial(pvalue);
    }

    bool is_nan() const
    {
        return mpd_isnan(pvalue);
    }

    bool is_zero() const
    {
        return mpd_iszero(pvalue);
    }

    bool is_infinite() const
    {
        return mpd_isinfinite(pvalue);
    }

    bool is_negative() const
    {
        return mpd_isnegative(pvalue);
    }
    std::string get_string()
    {
        std::string ret;
        try
        {
           //simple
            char *decstring;
            decstring = mpd_to_sci(pvalue, 1);
            if (decstring)
                ret+=decstring;
        }
        catch(...)
        {
            decimal_exception_handler("get_string() exception", true)();
        }
        return ret;
    }

    //format string fmt syntax is the same as in Python PEP 3101 Standard Format Specifiers
    std::string get_string(const char* fmt)
    {
        std::string ret;
        try
        {
            uint32_t status = 0;
            char * tmp_ret = mpd_qformat(pvalue, fmt, &ctx, &status);
            check_status_exception(&ctx, pvalue, status);
            if(tmp_ret) ret+=tmp_ret;
        }
        catch(...)
        {
            decimal_exception_handler("get_string(const char* fmt) exception", true)();
        }
        return ret;
    }

    Decimal()
    : traphandler(0)
    , pvalue(0)
    {
        try
        {
            init(default_precision);
            alloc_pvalue();
        }
        catch(...)
        {
            decimal_exception_handler("Decimal() ctor exception", true)();
        }
    }

    explicit Decimal(const char *str)
    : traphandler(0)
    , pvalue(0)
    {
        try
        {
            init(default_precision);
            alloc_pvalue();
            set_string(str);
        }
        catch(...)
        {
            decimal_exception_handler("Decimal(const char *str) ctor exception", true)();
        }
    }

    //copy
    Decimal(const Decimal& param )
    : traphandler(0)
    , pvalue(0)
    {
        try
        {
            init(param.ctx.prec);
            ctx = param.ctx;
            pvalue = mpd_qncopy(param.pvalue);
            if (pvalue == 0)
            {
                check_status_exception(&ctx,param.pvalue, MPD_Malloc_error);
            }
        }
        catch(...)
        {
            decimal_exception_handler("Decimal(const Decimal& param ) ctor exception", true)();
        }
    }

    Decimal& operator=(const Decimal& param)
    {
        try
        {
            if (this != &param)
            {
                init(param.ctx.prec);
                ctx = param.ctx;

                if(pvalue != 0)
                {
                    mpd_del(pvalue);
                    pvalue = 0;
                }

                pvalue = mpd_qncopy(param.pvalue);

                if (pvalue == 0)
                {
                    check_status_exception(&ctx,param.pvalue, MPD_Malloc_error);
                }
            }
        }
        catch(...)
        {
            decimal_exception_handler("Decimal& operator=(const Decimal& param) exception", true)();
        }

        return *this;
    }//operator=

    Decimal(const std::size_t precision)
    : traphandler(0)
    , pvalue(0)
    {
        try
        {
            init(precision);
            alloc_pvalue();
        }
        catch(...)
        {
            const unsigned long msg_len = 512;
            char msg[msg_len]={0};
            snprintf(msg, msg_len
                    ,"Decimal(const std::size_t precision: %u) ctor exception"
                    , precision);
            decimal_exception_handler(msg, true)();
        }
    }

    Decimal(const char *str, const std::size_t precision)
    : traphandler(0)
    , pvalue(0)
    {
        try
        {
            init(precision);
            alloc_pvalue();
            set_string(str);
        }
        catch(...)
        {
            const unsigned long msg_len = 2048;
            char msg[msg_len]={0};
            snprintf(msg, msg_len
                    ,"Decimal(const char *str: %s, const std::size_t precision: %u) ctor exception"
                    , str
                    , precision);
            decimal_exception_handler(msg, true)();
        }
    }


    virtual ~Decimal()
    {
        if(pvalue != 0) mpd_del(pvalue);
    }

    bool operator< (const Decimal& x) const
    {
        int c;
        try
        {
            Decimal result;
            uint32_t status = 0;
            c = mpd_qcompare(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
        }
        catch(...)
        {
            decimal_exception_handler("operator< exception", true)();
        }
        return (c == -1);
    }

    bool operator==(const Decimal& x) const
    {
        int c;
        try
        {
            Decimal result;
            uint32_t status = 0;
            c = mpd_qcompare(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
        }
        catch(...)
        {
            decimal_exception_handler("operator== exception", true)();
        }
        return (c == 0);
    }

    Decimal& operator+=(const Decimal& x)
    {
        try
        {
            Decimal result;
            uint32_t status = 0;
            mpd_qadd(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
            *this = result;
        }
        catch(...)
        {
            decimal_exception_handler("operator+= exception", true)();
        }
        return *this;
    }

    Decimal& operator-=(const Decimal& x)
    {
        try
        {
            Decimal result;
            uint32_t status = 0;
            mpd_qsub(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
            *this = result;
        }
        catch(...)
        {
            decimal_exception_handler("operator-= exception", true)();
        }
        return *this;
    }

    Decimal& operator*=(const Decimal& x)
    {
        try
        {
            Decimal result;
            uint32_t status = 0;
            mpd_qmul(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
            *this = result;
        }
        catch(...)
        {
            decimal_exception_handler("operator*= exception", true)();
        }
        return *this;
    }

    Decimal& operator/=(const Decimal& x)
    {
        try
        {
            Decimal result;
            uint32_t status = 0;
            mpd_qdiv(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
            *this = result;
        }
        catch(...)
        {
            decimal_exception_handler("operator/= exception", true)();
        }
        return *this;
    }

    Decimal& integral_division(const Decimal& x)
    {
        try
        {
            Decimal result;
            uint32_t status = 0;
            mpd_qdivint(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
            *this = result;
        }
        catch(...)
        {
            decimal_exception_handler("integral_division exception", true)();
        }
        return *this;
    }

    Decimal& integral_division_remainder(const Decimal& x)
    {
        try
        {
            Decimal result;
            uint32_t status = 0;
            mpd_qrem(result.pvalue, pvalue, x.pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
            *this = result;
        }
        catch(...)
        {
            decimal_exception_handler("integral_division_remainder exception", true)();
        }
        return *this;
    }



    Decimal& round(long precision_places
            , int round_type// MPD_ROUND_* from enum in mpdecimal.h
            )
    {
        try
        {
            int saved_context_round =  mpd_getround(&ctx);
            if(mpd_qsetround(&ctx, round_type) == 0)
                throw std::runtime_error(
                    "Decimal::round unable to set context_newround");
            std::string new_string(this->get_string((std::string(".")
                +boost::lexical_cast<std::string>(precision_places)
                +"f").c_str()));
            if(mpd_qsetround(&ctx, saved_context_round) == 0)
                throw std::runtime_error(
                    "Decimal::round unable to set saved_context_round");
            set_string(new_string.c_str());
        }
        catch(...)
        {
            decimal_exception_handler("round() exception", true)();
        }
        return *this;
    }

    Decimal& abs()
    {
        try
        {
            Decimal result;
            uint32_t status = 0;
            mpd_qabs(result.pvalue, pvalue, &ctx, &status);
            check_status_exception(&ctx,pvalue, status);
            *this = result;
        }
        catch(...)
        {
            decimal_exception_handler("abs() exception", true)();
        }
        return *this;
    }


};//class Decimal
#endif // DECIMAL_H_
