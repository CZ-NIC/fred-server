#ifndef ADMIN_MERGE_CONTACT_H__
#define ADMIN_MERGE_CONTACT_H__

#include "src/fredlib/contact/merge_contact.h"
#include "src/fredlib/logger_client.h"
#include "util/optional_value.h"


namespace Admin {


class MergeContact
{
public:
    MergeContact(
            const std::string &_src_contact_handle,
            const std::string &_dst_contact_handle);

    MergeContact(
            const std::string &_src_contact_handle,
            const std::string &_dst_contact_handle,
            const Optional<std::string> &_registrar);

    Fred::MergeContactOutput exec(Fred::Logger::LoggerClient &_logger_client);

    Fred::MergeContactOutput exec_dry_run();


private:
    std::string src_contact_handle_;
    std::string dst_contact_handle_;
    Optional<std::string> registrar_;
};


}


#endif /*ADMIN_MERGE_CONTACT_H__*/

