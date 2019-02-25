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
#ifndef REQUEST_CACHE_HH_B4B02CDD46DC4D8BAE36616834D210A4
#define REQUEST_CACHE_HH_B4B02CDD46DC4D8BAE36616834D210A4

#include "src/deprecated/libfred/requests/request.hh"
#include "src/deprecated/libfred/requests/model_request.hh"
#include <map>
#include <stdexcept>
#include <boost/thread/mutex.hpp>

namespace LibFred {

namespace Logger {

/* Cache used to store requests that are in process. This can speed up
 * closeRequest because necessary attributes can be taken from cache and
 * not from database.
 */
class RequestCache {
  boost::mutex cache_mutex;
	typedef std::map<unsigned long long, ModelRequest> CacheImpl;
	CacheImpl cache;
public:
  /// Exception thrown in case no record is found in cache
  struct NOT_EXISTS : public std::runtime_error
  {
      NOT_EXISTS()
      : std::runtime_error("RequestCache NOT_EXISTS")
      {}
  };
  /// Insert just saved request into cache during createRequest
	void insert(const ModelRequest& mr);
	/// Remove request from cache during closeRequest or throw NOT_EXISTS
	void remove(unsigned long long requestId);
	/// Query cache for request int process during closeRequest  or throw NOT_EXISTS
	const ModelRequest& get(unsigned long long requestId);
	/// Remove all records from cache for cleanup purpose
	void clean();
	/// Query number of records in cache for monitoring purpose
	unsigned count();
};

}

}

#endif /* CACHE_H_ */
