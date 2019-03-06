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
/**
 *  @file concurrent_queue.h
 *  concurrent_queue class template
 */


#ifndef CONCURRENT_QUEUE_HH_C52DEFE6B3F04F6D9C4B5829923F7990
#define CONCURRENT_QUEUE_HH_C52DEFE6B3F04F6D9C4B5829923F7990

#include <boost/thread.hpp>
#include <boost/version.hpp>
#include <boost/thread/barrier.hpp>
#include <queue>

#include <stdexcept>

/**
 * \class concurrent_queue
 * \brief Template queue class with internal locking
 */

template <typename T>
class concurrent_queue
{
public:
    using Type = T;
    class UnguardedAccess
    {
    public:
        UnguardedAccess() = delete;
        void push(const Type& data)
        {
            queue_.push(data);
        }
        auto size()const
        {
            return queue_.size();
        }
        bool empty()const
        {
            return queue_.empty();
        }
        auto pop()
        {
            return queue_.pop();
        }
    private:
        explicit UnguardedAccess(concurrent_queue& queue)
            : queue_(queue)
        { }
        concurrent_queue& queue_;
        friend class concurrent_queue;
    };
    class GuardedAccess
    {
    public:
        GuardedAccess() = delete;
        void push(const Type& data)
        {
            boost::mutex::scoped_lock guard(queue_.the_mutex);
            queue_.push(data);
        }
        auto size()const
        {
            boost::mutex::scoped_lock guard(queue_.the_mutex);
            return queue_.size();
        }
        bool empty()const
        {
            boost::mutex::scoped_lock guard(queue_.the_mutex);
            return queue_.empty();
        }
        auto pop()
        {
            boost::mutex::scoped_lock guard(queue_.the_mutex);
            return queue_.pop();
        }
    private:
        explicit GuardedAccess(concurrent_queue& queue)
            : queue_(queue)
        { }
        concurrent_queue& queue_;
        friend class concurrent_queue;
    };
    GuardedAccess guarded_access() { return GuardedAccess(*this); }
    UnguardedAccess unguarded_access() { return UnguardedAccess(*this); }
private:
    void push(const Type& data)
    {
        the_queue.push(data);
    }
    auto size()const
    {
        return the_queue.size();
    }
    bool empty()const
    {
        return the_queue.empty();
    }
    auto pop()
    {
        if (the_queue.empty())
        {
            std::runtime_error("queue is empty");
        }
        const auto front = the_queue.front();
        the_queue.pop();
        return front;
    }
    std::queue<Type> the_queue;
    mutable boost::mutex the_mutex;
};

#endif
