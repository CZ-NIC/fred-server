#include "src/epp/keyset/info_keyset.h"

#include "src/epp/epp_response_failure.h"
#include "src/epp/epp_result_code.h"
#include "src/epp/epp_result_failure.h"
#include "src/epp/exception.h"
#include "src/fredlib/keyset/info_keyset.h"
#include "src/fredlib/object_state/get_object_states.h"
#include "src/fredlib/registrar/info_registrar.h"

#include <string>
#include <vector>

namespace Epp {
namespace Keyset {

InfoKeysetOutputData info_keyset(
        Fred::OperationContext& _ctx,
        const std::string& _keyset_handle,
        unsigned long long _registrar_id)
{
    try
    {
        InfoKeysetOutputData result;
        static const char* const utc_timezone = "UTC";
        const Fred::InfoKeysetData data =
            Fred::InfoKeysetByHandle(_keyset_handle).exec(_ctx, utc_timezone).info_keyset_data;
        result.handle = data.handle;
        result.roid = data.roid;
        result.sponsoring_registrar_handle = data.sponsoring_registrar_handle;
        result.creating_registrar_handle = data.create_registrar_handle;
        result.last_update_registrar_handle = data.update_registrar_handle;
        {

            typedef std::vector<Fred::ObjectStateData> ObjectStatesData;

            ObjectStatesData keyset_states_data = Fred::GetObjectStates(data.id).exec(_ctx);
            for (ObjectStatesData::const_iterator data_ptr = keyset_states_data.begin();
                 data_ptr != keyset_states_data.end(); ++data_ptr)
            {
                result.states.insert(
                        Conversion::Enums::from_db_handle<Fred::Object_State>(
                                data_ptr->
                                state_name));
            }
        }
        result.crdate = data.creation_time;
        result.last_update = data.update_time;
        result.last_transfer = data.transfer_time;
        // show object authinfopw only to sponsoring registrar
        if (Fred::InfoRegistrarByHandle(data.sponsoring_registrar_handle).exec(_ctx).info_registrar_data.id ==
            _registrar_id)
        {
            result.authinfopw = data.authinfopw;
        }
        // result.ds_records = ... // Fred::InfoKeysetData doesn't contain any ds record informations
        {

            typedef std::vector<Fred::DnsKey> FredDnsKeys;
            for (FredDnsKeys::const_iterator data_ptr = data.dns_keys.begin();
                 data_ptr != data.dns_keys.end(); ++data_ptr)
            {
                result.dns_keys.insert(
                        Keyset::DnsKey(
                                data_ptr->get_flags(),
                                data_ptr->get_protocol(),
                                data_ptr->get_alg(),
                                data_ptr->get_key()));
            }
        }
        {

            typedef std::vector<Fred::ObjectIdHandlePair> FredObjectIdHandle;
            for (FredObjectIdHandle::const_iterator data_ptr = data.tech_contacts.begin();
                 data_ptr != data.tech_contacts.end(); ++data_ptr)
            {
                result.tech_contacts.insert(data_ptr->handle);
            }
        }
        return result;
    }
    catch (const Fred::InfoKeysetByHandle::Exception& e)
    {
        if (e.is_set_unknown_handle())
        {
            throw EppResponseFailure(EppResultFailure(EppResultCode::object_does_not_exist));
        }
        throw;
    }
}


} // namespace Epp::Keyset
} // namespace Epp
