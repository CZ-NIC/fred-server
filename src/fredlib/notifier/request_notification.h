#ifndef REQUEST_NOTIFICATION_H__
#define REQUEST_NOTIFICATION_H__

#include <vector>
#include <string>
#include "fredlib/common_diff.h"


namespace Fred {


template<class RT>
class RequestNotification
{
public:
    typedef std::vector<unsigned long long> RecipientList;


    RequestNotification(const unsigned long long &_request_id)
        : request_id_(_request_id),
          object_id_(0)
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Result info = conn.exec_params(
                "SELECT oreg.id, oreg.name, oreg.type, h1.id AS act, h2.id AS prev"
                " FROM history h1 LEFT join history h2 ON h1.id = h2.next"
                " JOIN object_history oh ON oh.historyid = h1.id"
                " JOIN object_registry oreg ON oreg.id = oh.id"
                " WHERE h1.request_id = $1::integer",
                Database::query_param_list(_request_id));

        if (info.size() != 1) {
            throw std::runtime_error("create notification: no such request");
        }

        object_id_       = static_cast<unsigned long long>(info[0][0]);
        object_handle_   = static_cast<std::string>(info[0][1]);
        object_type_     = static_cast<unsigned int>(info[0][2]);
        object_hid_act_  = info[0][3].isnull() ? 0 : static_cast<unsigned long long>(info[0][3]);
        object_hid_prev_ = info[0][4].isnull() ? 0 : static_cast<unsigned long long>(info[0][4]);
    }

    void add_recipient(const unsigned long long &_contact_hid)
    {
        recipients_.push_back(_contact_hid);
    }

    const RecipientList& get_recipients() const
    {
        return recipients_;
    }

    void set_object_changes(const ChangesMap &_changes)
    {
        object_changes_ = _changes;
    }

    const ChangesMap& get_object_changes() const
    {
        return object_changes_;
    }

    const unsigned short& get_object_type() const
    {
        return object_type_;
    }

    const unsigned long long& get_object_id() const
    {
        return object_id_;
    }

    const std::string& get_object_handle() const
    {
        return object_handle_;
    }

    const unsigned long long& get_request_id() const
    {
        return request_id_;
    }

    const unsigned long long& get_object_hid_act() const
    {
        return object_hid_act_;
    }

    const unsigned long long& get_object_hid_prev() const
    {
        return object_hid_prev_;
    }


    const RT& get_request_type() const
    {
        return request_type_;
    }

    void set_request_type(const RT &_type)
    {
        request_type_ = _type;
    }


private:
    unsigned long long request_id_;
    unsigned long long object_id_;
    std::string        object_handle_;
    unsigned short     object_type_;
    unsigned long long object_hid_act_;
    unsigned long long object_hid_prev_;
    ChangesMap         object_changes_;

    RT                 request_type_;

    /* history ids of contacts */
    RecipientList recipients_;
};


}


#endif /*REQUEST_NOTIFICATION_H__*/

