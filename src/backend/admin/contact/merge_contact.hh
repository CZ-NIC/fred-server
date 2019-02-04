#ifndef MERGE_CONTACT_HH_3CD295105AA242C78849BEB86FB40A40
#define MERGE_CONTACT_HH_3CD295105AA242C78849BEB86FB40A40

#include "libfred/registrable_object/contact/merge_contact.hh"
#include "src/deprecated/libfred/logger_client.hh"
#include "util/optional_value.hh"


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

    LibFred::MergeContactOutput exec(LibFred::Logger::LoggerClient &_logger_client);

    LibFred::MergeContactOutput exec_dry_run();


private:
    std::string src_contact_handle_;
    std::string dst_contact_handle_;
    Optional<std::string> registrar_;
};


}


#endif /*ADMIN_MERGE_CONTACT_H__*/

