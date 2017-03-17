#include "tests/interfaces/epp/domain/fixture.h"
#include "src/fredlib/object/object_id_handle_pair.h"

#include <string>
#include <vector>

namespace Test {

std::vector<std::string> vector_of_Fred_ObjectIdHandlePair_to_vector_of_string(const std::vector<Fred::ObjectIdHandlePair>& admin_contacts) {
    std::vector<std::string> admin;
    for (
        std::vector<Fred::ObjectIdHandlePair>::const_iterator object_id_handle_pair = admin_contacts.begin();
        object_id_handle_pair != admin_contacts.end();
        ++object_id_handle_pair
    ) {
        admin.push_back(object_id_handle_pair->handle);
    }
    return admin;
}

} // namespace Test
