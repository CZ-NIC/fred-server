#include "request_ntf_handler_contact.h"

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
    _ntf.set_object_changes(Contact::diff(cm.get(), _ntf.get_object_hid_prev(), _ntf.get_object_hid_act()));
}


void request_contact_transfer(RequestNotification &_ntf)
{
    _ntf.add_recipient(_ntf.get_object_hid_act());
}


}

