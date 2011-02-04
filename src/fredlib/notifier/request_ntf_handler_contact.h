#ifndef REQUEST_NTF_HANDLER_CONTACT_H__
#define REQUEST_NTF_HANDLER_CONTACT_H__

#include "request_notification.h"
#include "fredlib/contact_diff.h"


namespace Fred {


void request_contact_create(RequestNotification &_ntf);

void request_contact_update(RequestNotification &_ntf);

void request_contact_transfer(RequestNotification &_ntf);


}


#endif /*REQUEST_NTF_HANDLER_CONTACT_H__*/

