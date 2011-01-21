#ifndef REQUEST_NOTIFICATION_H__
#define REQUEST_NOTIFICATION_H__

#include <vector>
#include <string>
#include "fredlib/common_diff.h"


namespace Fred {


class RequestNotification
{
public:
    RequestNotification(const unsigned long long &_request_id);

    void add_recipient(const unsigned long long &_contact_id)
    {
        recipients_.push_back(_contact_id);
    }

    void set_object_changes(const ChangesMap &_changes)
    {
        object_changes_ = _changes;
    }

    const unsigned short& get_object_type() const
    {
        return object_type_;
    }

    const unsigned long long& get_object_id() const
    {
        return object_id_;
    }

    const unsigned long long& get_request_id() const
    {
        return request_id_;
    }


private:
    unsigned long long request_id_;
    unsigned long long object_id_;
    std::string        object_handle_;
    unsigned short     object_type_;
    ChangesMap         object_changes_;

    std::vector<unsigned long long> recipients_;
};


}


#endif /*REQUEST_NOTIFICATION_H__*/

