/*
 * Copyright (C) 2011-2019  CZ.NIC, z. s. p. o.
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
#ifndef CONCURRENT_SET_HH_B0AD23D144A3450D89ADA61291ADAF31
#define CONCURRENT_SET_HH_B0AD23D144A3450D89ADA61291ADAF31

#include <map>
#include <boost/thread.hpp>

template <class T>
class concurrent_set {

typedef typename std::set<T> set_type;
typedef typename set_type::size_type size_type;
typedef typename set_type::iterator iterator;

private:
    set_type aset;
    mutable boost::mutex lock;

public:
    concurrent_set() : aset() { }

    std::pair<iterator, bool>
    insert(const T &elem) {
        boost::mutex::scoped_lock guard(lock);
        return aset.insert(elem);
    }
    bool empty() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.empty();
    }
    size_type size() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.size();
    }
    iterator insert(iterator, const T &x) {
        boost::mutex::scoped_lock guard(lock);
        return aset.insert(x);
    }
    void erase(iterator position) {
        boost::mutex::scoped_lock guard(lock);
        aset.erase(position);
    }
    size_type erase(const T& x) {
        boost::mutex::scoped_lock guard(lock);
        return aset.erase(x);
    }
    void clear() {
        boost::mutex::scoped_lock guard(lock);
        aset.clear();
    }
    iterator begin() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.begin();
    }
    iterator end() const {
        boost::mutex::scoped_lock guard(lock);
        return aset.end();
    }
};

#endif /*CONCURRENT_SET_H_*/
