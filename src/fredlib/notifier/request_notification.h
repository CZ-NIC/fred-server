#ifndef REQUEST_NOTIFICATION_H__
#define REQUEST_NOTIFICATION_H__

#include <vector>
#include <string>
#include "fredlib/common_diff.h"


namespace Fred {


const std::string CMD_CONTACT_CREATE = "ContactCreate";
const std::string CMD_CONTACT_UPDATE = "ContactUpdate";
const std::string CMD_CONTACT_TRANSFER = "ContactTransfer";



class RequestNotification
{
public:
    typedef std::vector<unsigned long long> RecipientList;


    RequestNotification(const unsigned long long &_request_id);


    void add_recipient(const unsigned long long &_contact_hid)
    {
        recipients_.push_back(_contact_hid);
    }


    const RecipientList& get_recipients() const
    {
        return recipients_;
    }


    void set_object_changes(const ChangesMap& _object_changes)
    {
        object_changes_ = _object_changes;
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


    const std::string& get_request_type() const
    {
        return request_type_;
    }


    const std::string get_ticket_id() const
    {
        return Util::make_svtrid(request_id_);
    }


    const std::string get_registrar_info() const
    {
        return registrar_info_; 
    }


    void set_template_name(const std::string &_template_name)
    {
        template_name_ = _template_name;
    }


    const std::string& get_template_name() const
    {
        return template_name_;
    }


    void save_relation(const unsigned long long &_msg_id) const;



private:
    void load_request_info();

    void load_registrar_info(const unsigned long long _reg_id);


    unsigned long long request_id_;
    std::string        request_type_;
    std::string        registrar_info_;
    unsigned long long object_id_;
    std::string        object_handle_;
    unsigned short     object_type_;
    unsigned long long object_hid_act_;
    unsigned long long object_hid_prev_;
    ChangesMap         object_changes_;

    /* history ids of contacts */
    RecipientList      recipients_;
    std::string        template_name_;
};


}


#endif /*REQUEST_NOTIFICATION_H__*/

