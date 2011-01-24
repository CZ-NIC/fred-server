#ifndef REQUEST_NTF_HANDLER_CONTACT_H__
#define REQUEST_NTF_HANDLER_CONTACT_H__

#include "request_notification.h"
#include "fredlib/contact_diff.h"


namespace Fred {


void request_contact_create(RequestNotification &_ntf)
{
    _ntf.add_recipient(_ntf.get_object_hid_act());
}


void request_contact_update(RequestNotification &_ntf)
{
    _ntf.add_recipient(_ntf.get_object_hid_prev());
    _ntf.add_recipient(_ntf.get_object_hid_act());

    std::auto_ptr<Contact::Manager> cm(Contact::Manager::create(DBDisconnectPtr(0), true));
    _ntf.set_object_changes(
            Contact::diff(cm.get(), _ntf.get_object_hid_prev(), _ntf.get_object_hid_act()));
}


void request_contact_transfer(RequestNotification &_ntf)
{
    _ntf.add_recipient(_ntf.get_object_hid_act());
}


}


#endif /*REQUEST_NTF_HANDLER_CONTACT_H__*/

