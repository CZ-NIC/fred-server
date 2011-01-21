#ifndef REQUEST_NOTIFIER_H__
#define REQUEST_NOTIFIER_H__

#include <boost/assign/list_of.hpp>
#include <map>

#include "request_notification.h"
#include "map_at.h"


namespace Fred {


class RequestNotifier
{
public:
    typedef boost::function<void (RequestNotification &_ntf)> RequestHandler;
    typedef std::map<unsigned int, RequestHandler> RequestHandlerMap;


    RequestNotifier(const RequestHandlerMap &_handlers)
        : handlers_(_handlers)
    {
    }


    template<class TSender>
    void notify(const unsigned long long &_request_id, const TSender &_sender)
    {
        RequestNotification ntf(_request_id);
        unsigned int rtype = static_cast<unsigned int>(get_request_type(
                    _request_id, ntf.get_object_type()));

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
    unsigned int get_request_type(const unsigned long long &_request_id,
                                  const unsigned short &_object_type);

    RequestHandlerMap   handlers_;
};


}


#endif /*REQUEST_NOTIFIER_H__*/

