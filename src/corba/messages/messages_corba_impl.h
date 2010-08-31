#ifndef MESSAGES_CORBA_IMPL_H_
#define MESSAGES_CORBA_IMPL_H_

#include <vector>
#include <string>

unsigned long long save_file(std::vector<char> & file_buffer
        , const std::string file_name, const std::string file_mime_type
        , const unsigned int file_type );//"application/pdf",6

#endif // MESSAGES_CORBA_IMPL_H_
