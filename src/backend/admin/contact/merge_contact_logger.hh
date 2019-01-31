#ifndef MERGE_CONTACT_LOGGER_HH_B9BCF17767FA4A008A433FCC2C549D0F
#define MERGE_CONTACT_LOGGER_HH_B9BCF17767FA4A008A433FCC2C549D0F

#include "libfred/registrable_object/contact/merge_contact.hh"
#include "src/deprecated/libfred/logger_client.hh"


unsigned long long logger_merge_contact_create_request(
        LibFred::Logger::LoggerClient &_logger_client,
        const std::string &_src_contact,
        const std::string &_dst_contact);



void logger_merge_contact_close_request_success(
        LibFred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const LibFred::MergeContactOutput &_merge_data);



void logger_merge_contact_close_request_fail(
        LibFred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id);



#endif /*MERGE_CONTACT_LOGGER_H__*/

