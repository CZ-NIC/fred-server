/*
 * Copyright (C) 2018  CZ.NIC, z.s.p.o.
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
 *  @file
 *  exceptions
 */

#ifndef EXCEPIONS_HH_DC47802337AF4A2493F20AC3E084CA84
#define EXCEPIONS_HH_DC47802337AF4A2493F20AC3E084CA84

#include <exception>

struct WrongIntervalOrder : std::exception
{
    virtual const char* what() const noexcept
    {
        return "date from is later than date to";
    }
};

struct IntervalIntersection : std::exception
{
    virtual const char* what() const noexcept
    {
        return "new membership starts when old one is active";
    }
};

struct MembershipStartChange : std::exception
{
    virtual const char* what() const noexcept
    {
        return "membership starting date must not be changed";
    }
};

struct WrongMembershipEnd : std::exception
{
    virtual const char* what() const noexcept
    {
        return "membership infiniteness must not be altered";
    }
};

struct WrongRegistrar : std::exception
{
    virtual const char* what() const noexcept
    {
        return "this membership has different registrar";
    }
};

struct WrongGroup : std::exception
{
    virtual const char* what() const noexcept
    {
        return "this membership has different registrar group";
    }
};

struct MembershipNotFound : std::exception
{
    virtual const char* what() const noexcept
    {
        return "no active membership with given registrar and group found";
    }
};

#endif
