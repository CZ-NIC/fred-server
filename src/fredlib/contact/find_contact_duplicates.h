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
    FindAnyContactDuplicates();

    FindAnyContactDuplicates& set_registrar(const optional_string &_registrar_handle);

    FindAnyContactDuplicates& set_exclude_contacts(const std::set<std::string> &_exclude_contacts);

    std::set<std::string> exec(Fred::OperationContext &_ctx);


private:
    optional_string registrar_handle_;
    std::set<std::string> exclude_contacts_;
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
