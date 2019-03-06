/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef FILTER_IT_HH_AA110E6014DB45A783CF3AF9CD2891F4
#define FILTER_IT_HH_AA110E6014DB45A783CF3AF9CD2891F4

#include <vector>
#include "src/util/db/query/filter.hh"

namespace Database {
namespace Filters {

class Iterator {
public:
	Iterator(std::vector<Filter *>& fl) :
	flist(fl), it(fl.begin()) {
	}
	void first() {
		it = flist.begin();
	}
	void next() {
		if (!isDone())
		it++;
	}
	bool isDone() {
		return it == flist.end();
	}
	Filter* get() {
		return isDone() ? NULL : *it;
	}
private:
	std::vector<Filter *>& flist;
	std::vector<Filter *>::iterator it;
};

}
}

#endif /*FILTER_IT_H_*/
