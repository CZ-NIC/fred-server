#include "mojeid_notifier.h"


namespace Fred {


template<>
RequestType MojeIDRequestNotifier::get_request_type(
        const unsigned long long &_request_id,
        const unsigned short &_object_type) const
{
    if (_object_type != 1) {
        throw std::runtime_error("unsuported object-request type");
    }
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
        if (_object_type == 1) {
            return CREATE_CONTACT;
        }
    }

    ptime p_tr = prev_info[0][1];
    ptime a_tr = act_info[0][1];
    ptime p_up = prev_info[0][2];
    ptime a_up = act_info[0][2];

    if (p_tr != a_tr) {
        if (_object_type == 1) {
            return TRANSFER_CONTACT;
        }
    }
    else if (p_up != a_up) {
        if (_object_type == 1) {
            return UPDATE_CONTACT;
        }
    }
    throw std::runtime_error("unknown request type");
}


//template<class TSender>
//std::string MojeIDRequestNotifier::get_template(
//        const RequestType &_request_type, const TSender &_sender) const
//{
//}


MojeIDRequestNotifier::RequestHandlerMap init_handlers()
{
    MojeIDRequestNotifier::RequestHandlerMap handlers = boost::assign::map_list_of
            (CREATE_CONTACT, request_contact_create<RequestType>)
            (TRANSFER_CONTACT, request_contact_transfer<RequestType>)
            (UPDATE_CONTACT, request_contact_update<RequestType>);
    return handlers;
}


MojeIDRequestNotifier create_request_notifier_mojeid()
{
    static MojeIDRequestNotifier notifier(init_handlers());
    return notifier;
}

}

