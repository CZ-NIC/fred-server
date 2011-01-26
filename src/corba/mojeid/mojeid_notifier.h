#ifndef MOJEID_NOTIFIER_H__
#define MOJEID_NOTIFIER_H__

#include "fredlib/notifier/request_notifier.h"
#include "fredlib/notifier/request_ntf_handler_contact.h"
#include "fredlib/notifier/request_ntf_sender_email.h"

#include <boost/assign/list_of.hpp>
#include <map>


namespace Fred {


enum RequestType {
    CREATE_CONTACT,
    UPDATE_CONTACT,
    TRANSFER_CONTACT,
    DELETE_CONTACT,
};


typedef RequestNotifier<RequestType> MojeIDRequestNotifier;


struct IsRequestType
{
    IsRequestType(const RequestType &_rt) : type(_rt) { }

    bool operator()(const RequestNotification<RequestType> &_ntf) const
    {
        if (_ntf.get_request_type() == type) {
            return true;
        }
        else {
            return false;
        }
    }

    RequestType type;
};


MojeIDRequestNotifier create_request_notifier_mojeid();


}


#endif /*MOJEID_NOTIFIER_H__*/

