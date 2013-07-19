#ifndef CORBA_AUTO_GARBAGED_LIST
#define CORBA_AUTO_GARBAGED_LIST

#include <vector>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/thread/thread.hpp>
#include "util/log/logger.h"
#include "util/log/context.h"
#include "util/corba_wrapper_decl.h"


template<typename T>
class CorbaAutoGarbagedList
{
private:
    typedef boost::shared_ptr<T> value_type_ptr;
    typedef std::vector<value_type_ptr> value_type_ptr_list;

    std::string name_;
    boost::posix_time::time_duration object_max_idle_interval_;
    boost::posix_time::time_duration scavenger_thread_interval_;
    bool scavenger_thread_active_;
    boost::thread scavenger_thread_;

    boost::mutex access_mutex_;
    value_type_ptr_list data_;


public:
    CorbaAutoGarbagedList(
            const std::string &_name,
            const boost::posix_time::time_duration &_scavenger_thread_interval,
            const boost::posix_time::time_duration &_object_max_idle_interval)
        : name_(_name),
          object_max_idle_interval_(_object_max_idle_interval),
          scavenger_thread_interval_(_scavenger_thread_interval),
          scavenger_thread_active_(true),
          scavenger_thread_(boost::bind(&CorbaAutoGarbagedList::scavenger, this))
    {
    }

    ~CorbaAutoGarbagedList()
    {
        scavenger_thread_active_ = false;
        scavenger_thread_.join();
    }


    void push_back(const value_type_ptr &_v)
    {
        boost::lock_guard<boost::mutex> lock(access_mutex_);
        data_.push_back(_v);
    }


    void scavenger()
    {
        while (scavenger_thread_active_)
        {
            try {
                boost::this_thread::sleep(scavenger_thread_interval_);

                Logging::Context ctx(name_);
                LOGGER(PACKAGE).debug("AutoGarbageList::scavenger: iteration started");
                {
                    boost::lock_guard<boost::mutex> lock(access_mutex_);

                    LOGGER(PACKAGE).debug(boost::format("AutoGarbageList::scavenger: size=%1%") % data_.size());
                    typename value_type_ptr_list::iterator it = data_.begin();
                    while (it != data_.end())
                    {
                        if ((boost::posix_time::second_clock::local_time() - (*it)->get_last_used()) > object_max_idle_interval_)
                        {
                            (*it)->close();
                        }

                        if ((*it)->is_closed())
                        {
                            LOGGER(PACKAGE).debug(boost::format("AutoGarbageList::scavenger: erasing object (last_used=%1%)") % (*it)->get_last_used());
                            value_type_ptr del = *it;
                            it = data_.erase(it);
                            LOGGER(PACKAGE).debug("AutoGarbageList::scavenger: object erased from list");
                            /* corba object deactivation */
                            PortableServer::ObjectId_var id = CorbaContainer::get_instance()->root_poa->servant_to_id(del.get());
                            CorbaContainer::get_instance()->root_poa->deactivate_object(id);
                            LOGGER(PACKAGE).debug("AutoGarbageList::scavenger: object deactivated");
                        }
                        else
                        {
                            it++;
                        }
                    }
                }
                LOGGER(PACKAGE).debug(boost::format("AutoGarbageList::scavenger: iteration finished"
                            " (next at %1%)") % (boost::posix_time::second_clock::local_time() + scavenger_thread_interval_));
            }
            catch (std::exception &ex)
            {
                try
                {
                    LOGGER(PACKAGE).error(std::string("AutoGarbageList::scavenger: iteration run failed (") + ex.what() + std::string(")"));
                }
                catch (...) { }
            }
            catch (...)
            {
                try
                {
                    LOGGER(PACKAGE).error("AutoGarbageList::scavenger: iteration run failed");
                }
                catch (...) { }
            }
        }

    }

};



#endif /*CORBA_AUTO_GARBAGED_LIST*/
