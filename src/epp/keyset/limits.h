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

/**
 *  @file
 */

#ifndef LIMITS_H_6FBC5448A8637E00488EEB81C6607506//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define LIMITS_H_6FBC5448A8637E00488EEB81C6607506

namespace Epp {
namespace KeySet {

const unsigned min_number_of_tech_contacts =  1;
const unsigned max_number_of_tech_contacts = 10;

const unsigned min_number_of_dns_keys =  1;
const unsigned max_number_of_dns_keys = 10;

const unsigned min_number_of_ds_records = 0;
const unsigned max_number_of_ds_records = 0;

}//namespace Epp::KeySet
}//namespace Epp

#endif//LIMITS_H_6FBC5448A8637E00488EEB81C6607506
