/*
 * Copyright (C) 2015-2019  CZ.NIC, z. s. p. o.
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
/**
 *  @file
 *  header of safe data storage implementation
 */

#ifndef SAFE_DATA_STORAGE_HH_788C0297D4FD4B119C93044CBD21B3DB
#define SAFE_DATA_STORAGE_HH_788C0297D4FD4B119C93044CBD21B3DB

#include <map>
#include <stdexcept>
#include <boost/thread/mutex.hpp>
#include <boost/thread/once.hpp>
#include <boost/thread/locks.hpp>

/**
 * Add exclusive access protection to the singleton object instance of given type.
 * @tparam OBJECT_TYPE type of protected object
 */
template < typename OBJECT_TYPE >
class guarded_singleton
{
public:
    typedef OBJECT_TYPE object_type;     ///< used for type without exclusive access protection
    typedef object_type under_protection;///< type with exclusive access protection

    /// Starts to guard object instance.
    guarded_singleton()
    :   singleton_(get()),
        guard_(singleton_)
    { }

    /**
     * Makes guarded object accessible.
     * @return pointer to singleton object instance
     */
    under_protection* operator->() { return singleton_; }

    /**
     * Makes guarded object accessible.
     * @return pointer to non-modifiable part of singleton object instance
     */
    const under_protection* operator->()const { return singleton_; }
private:
    typedef boost::mutex                    mutex_type;
    typedef boost::lock_guard< mutex_type > guard_type;
    class lockable:public mutex_type
    {
    public:
        operator under_protection*() { return &object; }
        operator const under_protection*()const { return &object; }
    private:
        object_type object;
    };
    static void init_me()
    {
        static lockable singleton_instance;
        singleton_ptr_ = &singleton_instance;
    }
    static lockable& get()
    {
        boost::call_once(im_uninitialized_, init_me);
        return *singleton_ptr_;
    }
    lockable                &singleton_;
    guard_type               guard_;
    static lockable         *singleton_ptr_;
    static boost::once_flag  im_uninitialized_;
};

template < typename GUARDED_TYPE >
typename guarded_singleton< GUARDED_TYPE >::lockable *guarded_singleton< GUARDED_TYPE >::singleton_ptr_;

template < typename GUARDED_TYPE >
boost::once_flag guarded_singleton< GUARDED_TYPE >::im_uninitialized_ = BOOST_ONCE_INIT;

/**
 * Data storage class without exclusive access control.
 * @tparam KEY type of identification
 * @tparam VALUE type of stored data
 */
template < typename KEY, typename VALUE >
class data_storage
{
public:
    typedef KEY   key_type;
    typedef VALUE value_type;
    /// Thread-safe variant of data_storage.
    typedef guarded_singleton< data_storage > safe;

    class key_already_used:public std::runtime_error
    {
    public:
        key_already_used(const std::string &_msg):std::runtime_error(_msg) { }
    };

    class data_not_found:public std::runtime_error
    {
    public:
        data_not_found(const std::string &_msg):std::runtime_error(_msg) { }
    };

    /**
     * Stores data identified by given key.
     * @param _key data record identification
     * @param _value stored data
     * @throw key_already_used in case of data record already exists
     */
    void store(const key_type &_key, const value_type &_value)
    {
        typename storage_type::const_iterator data_ptr = storage_.find(_key);
        if (data_ptr != storage_.end()) {
            throw key_already_used("key already used");
        }
        storage_.insert(std::make_pair(_key, _value));
    }

    /**
     * Return data identified by given key.
     * @param _key data record identification
     * @return stored data
     * @throw data_not_found in case of data record doesn't exist
     */
    value_type get(const key_type &_key)const
    {
        typename storage_type::const_iterator data_ptr = storage_.find(_key);
        if (data_ptr != storage_.end()) {
            return data_ptr->second;
        }
        throw data_not_found("data not found");
    }

    /**
     * Release data record identified by given key.
     * @param _key data record identification
     * @throw data_not_found in case of data record doesn't exist
     */
    void release(const key_type &_key)
    {
        typename storage_type::iterator data_ptr = storage_.find(_key);
        if (data_ptr == storage_.end()) {
            throw data_not_found("data not found");
        }
        storage_.erase(data_ptr);
    }
private:
    typedef std::map< key_type, value_type > storage_type;
    storage_type storage_;
};

#endif
