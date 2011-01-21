#ifndef REQUEST_NTF_HANDLER_CONTACT_H__
#define REQUEST_NTF_HANDLER_CONTACT_H__

#include "request_notification.h"
#include "fredlib/contact_diff.h"


namespace Fred {


void request_contact_common(RequestNotification &_ntf)
{
    _ntf.add_recipient(_ntf.get_object_id());
}



void request_contact_update(RequestNotification &_ntf)
{
    request_contact_common(_ntf);
    std::auto_ptr<Contact::Manager> cm(Contact::Manager::create(DBDisconnectPtr(0), true));
    _ntf.set_object_changes(Contact::diff_last_history(cm.get(), _ntf.get_object_id()));
}


}


#endif /*REQUEST_NTF_HANDLER_CONTACT_H__*/

