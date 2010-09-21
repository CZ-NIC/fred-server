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


#ifndef CACHE_H_
#define CACHE_H_

#include "request.h"
#include <map>
#include <boost/thread/mutex.hpp>

namespace Register {

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
  struct NOT_EXISTS {};
  /// Insert just saved request into cache during createRequest
	void insert(const ModelRequest& mr);
	/// Remove request from cache during closeRequest
	void remove(unsigned long long requestId) throw (NOT_EXISTS);
	/// Query cache for request int process during closeRequest
	const ModelRequest& get(unsigned long long requestId) throw (NOT_EXISTS);
	/// Remove all records from cache for cleanup purpose
	void clean();
	/// Query number of records in cache for monitoring purpose
	unsigned count();
};

}

}

#endif /* CACHE_H_ */