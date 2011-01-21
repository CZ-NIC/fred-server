#ifndef MOJEID_NOTIFIER_H__
#define MOJEID_NOTIFIER_H__

#include "fredlib/notifier/request_notifier.h"
#include "fredlib/notifier/request_ntf_handler_contact.h"

#include <boost/assign/list_of.hpp>
#include <map>


namespace Fred {


enum RequestType {
    CREATE_CONTACT,
    CREATE_DOMAIN,
    CREATE_NSSET,
    CREATE_KEYSET,
    UPDATE_CONTACT,
    UPDATE_DOMAIN,
    UPDATE_NSSET,
    UPDATE_KEYSET,
    TRANSFER_CONTACT,
    TRANSFER_DOMAIN,
    TRANSFER_NSSET,
    TRANSFER_KEYSET,
    DELETE_CONTACT,
    DELETE_DOMAIN,
    DELETE_NSSET,
    DELETE_KEYSET,
    RENEW_DOMAIN
};



unsigned int RequestNotifier::get_request_type(const unsigned long long &_request_id,
                                    const unsigned short &_object_type)
{
    Database::Connection conn = Database::Manager::acquire();

    Database::Result act_info = conn.exec_params(
            "SELECT oh.historyid, oh.trdate, oh.update FROM object_history oh"
            " JOIN history h ON h.id = oh.historyid"
            " WHERE h.request_id = $1::integer",
            Database::query_param_list(_request_id));

    if (act_info.size() != 1) {
        throw std::runtime_error("no such request in object history");
    }

    unsigned long long hid = act_info[0][0];
    Database::Result prev_info = conn.exec_params(
            "SELECT oh.historyid, oh.trdate, oh.update FROM object_history oh"
            " JOIN history h ON h.id = oh.historyid"
            " WHERE h.next = $1::integer",
            Database::query_param_list(hid));

    if (prev_info.size() != 1) {
        switch (_object_type) {
            case 1: return CREATE_CONTACT;
            case 2: return CREATE_NSSET;
            case 3: return CREATE_DOMAIN;
            case 4: return CREATE_KEYSET;
        }
    }

    ptime p_tr = prev_info[0][1];
    ptime a_tr = act_info[0][1];
    ptime p_up = prev_info[0][2];
    ptime a_up = act_info[0][2];

    if (p_tr != a_tr) {
        switch (_object_type) {
            case 1: return TRANSFER_CONTACT;
            case 2: return TRANSFER_NSSET;
            case 3: return TRANSFER_DOMAIN;
            case 4: return TRANSFER_KEYSET;
        }
    }
    else if (p_up != a_up) {
        switch (_object_type) {
            case 1: return UPDATE_CONTACT;
            case 2: return UPDATE_NSSET;
            case 3: return UPDATE_DOMAIN;
            case 4: return UPDATE_KEYSET;
        }
    }
    throw std::runtime_error("unknown request type");
}


RequestNotifier::RequestHandlerMap init_handlers()
{
    RequestNotifier::RequestHandlerMap handlers = boost::assign::map_list_of
            (static_cast<unsigned int>(CREATE_CONTACT), request_contact_common)
            (static_cast<unsigned int>(TRANSFER_CONTACT), request_contact_common)
            (static_cast<unsigned int>(UPDATE_CONTACT), request_contact_update);
    return handlers;
}


RequestNotifier create_request_notifier_mojeid()
{
    static RequestNotifier notifier(init_handlers());
    return notifier;
}


}


#endif /*MOJEID_NOTIFIER_H__*/

