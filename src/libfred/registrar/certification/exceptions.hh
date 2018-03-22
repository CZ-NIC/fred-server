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

#ifndef EXCEPTIONS_HH_6E652B2F6EEF4081BF3CD3561D3FB0EE
#define EXCEPTIONS_HH_6E652B2F6EEF4081BF3CD3561D3FB0EE

#include "src/util/base_exception.hh"

struct WrongIntervalOrder : std::exception
{
    virtual const char* what() const noexcept
    {
        return "date from is later than date to";
    }
};

struct OverlappingRange : std::exception
{
    virtual const char* what() const noexcept
    {
        return "range of certification overlaps with the other one";
    }
};

struct CertificationInPast : std::exception
{
    virtual const char* what() const noexcept
    {
        return "end date is earlier than current date";
    }
};

struct CertificationExtension : std::exception
{
    virtual const char* what() const noexcept
    {
        return "end date is later than latter value";
    }
};

struct ScoreOutOfRange : std::exception
{
    virtual const char* what() const noexcept
    {
        return "certification got score out of bound";
    }
};

struct RegistrarNotFound : std::exception
{
    virtual const char* what() const noexcept
    {
        return "registrar with this id doesn't exist";
    }
};

#endif
