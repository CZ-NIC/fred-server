/*
 * Copyright (C) 2016  CZ.NIC, z.s.p.o.
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

#ifndef EXCEPTION_AGGREGATE_PARAM_ERRORS_H_E07FF065CEB94BB6BF86C7D103D03B3F
#define EXCEPTION_AGGREGATE_PARAM_ERRORS_H_E07FF065CEB94BB6BF86C7D103D03B3F

#include "src/epp/error.h"

#include <set>

namespace Epp {

class AggregatedParamErrors
{
public:
    AggregatedParamErrors& add(const Error& _new_error)
    {
        param_errors_.insert(_new_error);
        return *this;
    }

    std::set<Error> get() const
    {
        return param_errors_;
    }

    bool is_empty() const
    {
        return param_errors_.empty();
    }

private:
    std::set<Error> param_errors_;
};

class ParameterValuePolicyError
{
public:
    ParameterValuePolicyError& add(const Error& _new_error)
    {
        param_errors_.insert(_new_error);
        return *this;
    }

    ParameterValuePolicyError& add(const std::set<Error>& _new_errors)
    {
        param_errors_.insert(_new_errors.begin(), _new_errors.end());
        return *this;
    }

    std::set<Error> get() const
    {
        return param_errors_;
    }

    bool is_empty() const
    {
        return param_errors_.empty();
    }

private:
    std::set<Error> param_errors_;

};

class ParameterValueRangeError {
public:
    ParameterValueRangeError& add(const Error& _new_error)
    {
        param_errors_.insert(_new_error);
        return *this;
    }

    std::set<Error> get() const
    {
        return param_errors_;
    }

    bool is_empty() const
    {
        return param_errors_.empty();
    }

private:
    std::set<Error> param_errors_;

};

class ParameterValueSyntaxError
{
public:
    ParameterValueSyntaxError& add(const Error& _new_error)
    {
        param_errors_.insert(_new_error);
        return *this;
    }

    std::set<Error> get() const
    {
        return param_errors_;
    }

    bool is_empty() const
    {
        return param_errors_.empty();
    }

private:
    std::set<Error> param_errors_;

};

class RequiredSpecificParameterMissing
{
public:
    RequiredSpecificParameterMissing& add(const Error& _new_error)
    {
        param_errors_.insert(_new_error);
        return *this;
    }

    std::set<Error> get() const
    {
        return param_errors_;
    }

    bool is_empty() const
    {
        return param_errors_.empty();
    }

private:
    std::set<Error> param_errors_;

};

} // namespace Epp

#endif
