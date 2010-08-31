#include <vector>
#include <string>

#include "messages_corba_impl.h"
#include "corba_wrapper_decl.h"
#include "common.h"
#include "file_manager_client.h"

unsigned long long save_file(std::vector<char>& file_buffer
        , const std::string file_name, const std::string file_mime_type
        , const unsigned int file_type )
{
    FileManagerClient fm_client(CorbaContainer::get_instance()->getNS());

    unsigned long long file_id
            = fm_client.upload(file_buffer,file_name,file_mime_type,file_type);

    return file_id;
}
