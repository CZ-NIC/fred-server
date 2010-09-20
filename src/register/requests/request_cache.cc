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

#include "request_cache.h"

void
Register::Logger::RequestCache::insert(const ModelRequest& mr)
{
	// TODO: lock before this call, cache is shared resource
	cache[mr.getId()] = mr;
}

void
Register::Logger::RequestCache::remove(unsigned long long requestId)
throw (NOT_EXISTS)
{
	// TODO: lock before this call, cache is shared resource
	CacheImpl::const_iterator m = cache.find(requestId);
	if (m == cache.end()) throw NOT_EXISTS();
	cache.erase(requestId);
}

const ModelRequest&
Register::Logger::RequestCache::get(unsigned long long requestId)
throw (NOT_EXISTS)
{
	// TODO: lock before this call, cache is shared resource
	CacheImpl::const_iterator m = cache.find(requestId);
	if (m == cache.end()) throw NOT_EXISTS();
	return m->second;
}

void
Register::Logger::RequestCache::clean()
{
	// TODO: lock before this call, cache is shared resource
	cache.clear();
}

unsigned
Register::Logger::RequestCache::count()
{
	// TODO: lock before this call, cache is shared resource
	return cache.size();
}
