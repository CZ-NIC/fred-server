#ifndef MERGE_CONTACT_LOGGER_H__
#define MERGE_CONTACT_LOGGER_H__

#include "src/fredlib/contact/merge_contact.h"
#include "src/fredlib/logger_client.h"


unsigned long long logger_merge_contact_create_request(
        Fred::Logger::LoggerClient &_logger_client,
        const std::string &_src_contact,
        const std::string &_dst_contact);



void logger_merge_contact_close_request_success(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id,
        const Fred::MergeContactOutput &_merge_data);



void logger_merge_contact_close_request_fail(
        Fred::Logger::LoggerClient &_logger_client,
        const unsigned long long _req_id);



#endif /*MERGE_CONTACT_LOGGER_H__*/

