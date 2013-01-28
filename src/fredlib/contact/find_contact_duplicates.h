#ifndef FIND_CONTACT_DUPLICATES
#define FIND_CONTACT_DUPLICATES

#include "fredlib/opcontext.h"
#include "util/types/optional.h"

#include <set>
#include <string>


namespace Fred {
namespace Contact {


class FindAnyContactDuplicates
{
public:
    FindAnyContactDuplicates(const optional_string &_registrar_handle);

    std::set<std::string> exec(Fred::OperationContext &_ctx);


private:
    optional_string registrar_handle_;
};



class FindSpecificContactDuplicates
{
public:
    FindSpecificContactDuplicates(const std::string &_contact_handle);

    std::set<std::string> exec(Fred::OperationContext &_ctx);


private:
    std::string contact_handle_;
};


}
}

#endif /*FIND_CONTACT_DUPLICATES*/
