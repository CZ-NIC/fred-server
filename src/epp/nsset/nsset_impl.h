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

#ifndef EPP_NSSET_IMPL_H_9e6a8e492e314c5993aec42a9eb4af7d
#define EPP_NSSET_IMPL_H_9e6a8e492e314c5993aec42a9eb4af7d

#include <boost/asio/ip/address.hpp>

namespace Epp {

    bool is_unspecified_ip_addr(const boost::asio::ip::address& ipaddr);

}

#endif
