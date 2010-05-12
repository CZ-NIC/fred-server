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

/**
 *  @file concurrent_queue.h
 *  concurrent_queue class template
 */


#ifndef CONCURRENT_QUEUE_H_
#define CONCURRENT_QUEUE_H_

#include <boost/thread.hpp>
#include <boost/version.hpp>
#include <boost/thread/barrier.hpp>
#include <queue>

/**
 * \class concurrent_queue
 * \brief Template queue class with internal locking
 */

template<typename Data>
class concurrent_queue
{
private:
    std::queue<Data> the_queue;
    mutable boost::mutex the_mutex;

#if ( BOOST_VERSION > 103401 )
    boost::condition_variable
#else
    boost::condition
#endif
        the_condition_variable;
public:
    void push(Data const& data)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        the_queue.push(data);
        lock.unlock();
        the_condition_variable.notify_one();
    }

    std::size_t size()
    {
        boost::mutex::scoped_lock lock(the_mutex);
        std::size_t ret = the_queue.size();
        lock.unlock();
        the_condition_variable.notify_one();
        return ret;
    }


    bool empty() const
    {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_queue.empty();
    }

    bool try_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        if(the_queue.empty())
        {
            return false;
        }

        popped_value=the_queue.front();
        the_queue.pop();
        return true;
    }

    void wait_and_pop(Data& popped_value)
    {
        boost::mutex::scoped_lock lock(the_mutex);
        while(the_queue.empty())
        {
            the_condition_variable.wait(lock);
        }

        popped_value=the_queue.front();
        the_queue.pop();
    }
};//concurrent_queue

#endif //CONCURRENT_QUEUE_H_
