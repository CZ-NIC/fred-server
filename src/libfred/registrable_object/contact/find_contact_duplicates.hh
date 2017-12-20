#ifndef FIND_CONTACT_DUPLICATES
#define FIND_CONTACT_DUPLICATES

#include "src/libfred/opcontext.hh"
#include "src/util/optional_value.hh"

#include <set>
#include <string>


namespace LibFred {
namespace Contact {

class FindContactDuplicates
{
public:
    FindContactDuplicates();

    FindContactDuplicates& set_registrar(const Optional<std::string>& _registrar_handle);
    FindContactDuplicates& set_exclude_contacts(const std::set<std::string>& _exclude_contacts);
    FindContactDuplicates& set_specific_contact(const std::string& _dest_contact_handle);

    std::set<std::string> exec(LibFred::OperationContext& _ctx);

private:
    Optional<std::string> registrar_handle_;
    std::set<std::string> exclude_contacts_;
    Optional<std::string> specific_contact_handle_;
};

}
}

#endif /*FIND_CONTACT_DUPLICATES*/
