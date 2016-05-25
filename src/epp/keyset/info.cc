#include "src/epp/keyset/info.h"

#include "src/epp/exception.h"

#include "src/fredlib/object_state/get_object_state_descriptions.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/keyset/info_keyset.h"

namespace Epp {

KeysetInfoData keyset_info(Fred::OperationContext &_ctx,
                           const std::string &_keyset_handle,
                           unsigned long long _registrar_id)
{
}

}
