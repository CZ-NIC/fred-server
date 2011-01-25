#ifndef REQUEST_NOTIFIER_H__
#define REQUEST_NOTIFIER_H__

#include <boost/assign/list_of.hpp>
#include <map>

#include "request_notification.h"
#include "map_at.h"


namespace Fred {


template<class RT>
class RequestNotifier
{
public:
    typedef boost::function<void (RequestNotification<RT> &_ntf)> RequestHandler;
    typedef std::map<RT, RequestHandler> RequestHandlerMap;


    RequestNotifier(const RequestHandlerMap &_handlers)
        : handlers_(_handlers)
    {
    }


    template<class TSender>
    void notify(const unsigned long long &_request_id, const TSender &_sender)
    {
        this->notify(_request_id, _sender, DisableCondition<RT>());
    }


    template<class TSender, class TCond>
    void notify_if(const unsigned long long &_request_id, const TSender &_sender,
                   const TCond &_cond)
    {
        RequestNotification<RT> ntf(_request_id);
        RT rtype = get_request_type(_request_id, ntf.get_object_type());

        ntf.set_request_type(rtype);

        if (!_cond(ntf)) {
            LOGGER(PACKAGE).debug("notification canceled: condition was not met");
            return;
        }

        try {
            map_at(handlers_, rtype)(ntf);
        }
        catch (std::out_of_range &_ex) {
            throw std::runtime_error(str(boost::format("not notification"
                            " handler for request type %1%") % rtype));
        }

        _sender.send(ntf);
    }


private:

    template<class T>
    struct DisableCondition
    {
        bool operator()(const RequestNotification<T> &_ntf) const
        {
            return true;
        }
    };

    RT get_request_type(const unsigned long long &_request_id,
                        const unsigned short &_object_type);

    RequestHandlerMap   handlers_;
};


}


#endif /*REQUEST_NOTIFIER_H__*/

