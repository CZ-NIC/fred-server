/*
 * Copyright (C) 2017  CZ.NIC, z.s.p.o.
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

#ifndef PURPOSE_HH_F9C0DEF8A30E9FD48DF69B3AA5DE34F9//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define PURPOSE_HH_F9C0DEF8A30E9FD48DF69B3AA5DE34F9

namespace Registry {
namespace RecordStatement {

struct Purpose
{
    enum Enum
    {
        private_printout,
        public_printout
    };
};

}//namespace Registry::RecordStatement
}//namespace Registry

#endif//PURPOSE_HH_F9C0DEF8A30E9FD48DF69B3AA5DE34F9
