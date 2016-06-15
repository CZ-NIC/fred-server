#include "src/epp/keyset/update.h"
#include "src/epp/keyset/limits.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/parameter_errors.h"
#include "src/epp/reason.h"

#include "src/fredlib/registrar/info_registrar.h"

#include "src/fredlib/keyset/update_keyset.h"
#include "src/fredlib/keyset/info_keyset.h"

#include "src/fredlib/contact/check_contact.h"

#include <map>
#include <set>
#include <ctype.h>

namespace Epp {

namespace {

Fred::InfoKeysetData check_keyset_handle(const std::string &_keyset_handle,
                                         unsigned long long _registrar_id,
                                         Fred::OperationContext &_ctx,
                                         std::string &_callers_registrar_handle)
{
    try {
        const Fred::InfoRegistrarData callers_registrar =
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data;
        const Fred::InfoKeysetData result =
            Fred::InfoKeysetByHandle(_keyset_handle).set_lock().exec(_ctx).info_keyset_data;
        if (callers_registrar.system.get_value_or(false) ||
            (result.sponsoring_registrar_handle == callers_registrar.handle))
        {
            _callers_registrar_handle = callers_registrar.handle;
            return result;
        }
        ParameterErrors param_errors;
        param_errors.add_scalar_parameter_error(Param::registrar_autor, Reason::registrar_autor);
        _ctx.get_log().info("check_keyset_handle failure: registrar not authorized for this operation");
        throw param_errors;
    }
    catch (const Fred::InfoKeysetByHandle::Exception &e) {
        if (e.is_set_unknown_handle()) {
            ParameterErrors param_errors;
            param_errors.add_scalar_parameter_error(Param::keyset_handle, Reason::keyset_notexist);
            _ctx.get_log().info("check_keyset_handle failure: keyset not found");
            throw param_errors;
        }
        _ctx.get_log().error("check_keyset_handle failure: unexpected error has occurred");
        throw;
    }
    catch (const Fred::InfoRegistrarById::Exception &e) {
        if (e.is_set_unknown_registrar_id()) {
            _ctx.get_log().error("check_keyset_handle failure: registrar id not found");
            throw;
        }
        _ctx.get_log().error("check_keyset_handle failure: unexpected error has occurred in "
                                                          "InfoRegistrarById operation");
        throw;
    }
    catch (...) {
        _ctx.get_log().error("check_keyset_handle failure: unexpected exception was throwing");
        throw;
    }
}

typedef std::set< std::string > Handles;
Handles get_tech_contact_handles(const std::vector< Fred::ObjectIdHandlePair > &_id_handles)
{
    typedef std::vector< Fred::ObjectIdHandlePair > IdHandlePairs;
    Handles result;
    for (IdHandlePairs::const_iterator id_handle_ptr = _id_handles.begin();
         id_handle_ptr != _id_handles.end(); ++id_handle_ptr)
    {
        result.insert(id_handle_ptr->handle);
    }
    return result;
}

typedef bool Presents;
Presents copy_error(Param::Enum _param, Reason::Enum _reason,
                    unsigned short _src_idx, unsigned short _dst_idx,
                    ParameterErrors &_param_errors)
{
    if (_param_errors.has_vector_parameter_error_at(_param, _src_idx, _reason)) {
        _param_errors.add_vector_parameter_error(_param, _dst_idx, _reason);
        return true;
    }
    return false;
}

typedef bool Success;

Success check_tech_contacts(const std::vector< std::string > &_tech_contacts_add,
                            const std::vector< std::string > &_tech_contacts_rem,
                            const Fred::InfoKeysetData &_keyset_data,
                            Fred::OperationContext &_ctx,
                            ParameterErrors &_param_errors)
{
    if (_tech_contacts_add.empty() && _tech_contacts_rem.empty()) {//nothing to do
        return true;
    }

    Success check_result = true;

    if (KeySet::max_number_of_tech_contacts < _tech_contacts_add.size()) {
        _param_errors.add_scalar_parameter_error(Param::keyset_tech, Reason::techadmin_limit);
        check_result = false;
    }

    const Handles current_tech_contacts = get_tech_contact_handles(_keyset_data.tech_contacts);

    if (current_tech_contacts.size() < _tech_contacts_rem.size()) {
        _param_errors.add_scalar_parameter_error(Param::keyset_tech_rem, Reason::can_not_remove_tech);
        check_result = false;
    }

    //prevents detailed checking of too long _tech_contacts_add or _tech_contacts_rem lists
    if (!check_result) {
        return false;
    }

    typedef std::map< std::string, unsigned short > HandleIndex;
    HandleIndex unique_handles;
    unsigned short idx = 0;
    unsigned short to_add_count = 0;
    for (std::vector< std::string >::const_iterator handle_ptr = _tech_contacts_add.begin();
         handle_ptr != _tech_contacts_add.end(); ++handle_ptr, ++idx)
    {
        const HandleIndex::const_iterator handle_index_ptr = unique_handles.find(*handle_ptr);
        if (handle_index_ptr != unique_handles.end()) {//a duplicate handle
            _param_errors.add_vector_parameter_error(Param::keyset_tech_add, idx, Reason::duplicity_contact);
            copy_error(Param::keyset_tech_add, Reason::tech_notexist,
                       handle_index_ptr->second, idx, _param_errors);
            copy_error(Param::keyset_tech_add, Reason::tech_exist,
                       handle_index_ptr->second, idx, _param_errors);
            check_result = false;
            continue;
        }
        //unique handle
        unique_handles.insert(std::make_pair(*handle_ptr, idx));
        if (0 < current_tech_contacts.count(*handle_ptr)) {
            _param_errors.add_vector_parameter_error(Param::keyset_tech_add, idx, Reason::tech_exist);
            check_result = false;
            continue;
        }
        switch (Fred::Contact::get_handle_registrability(_ctx, *handle_ptr))
        {
            case Fred::ContactHandleState::Registrability::registered:
                ++to_add_count;
                break;
            case Fred::ContactHandleState::Registrability::available:
            case Fred::ContactHandleState::Registrability::in_protection_period:
                _param_errors.add_vector_parameter_error(Param::keyset_tech_add, idx, Reason::tech_notexist);
                check_result = false;
                break;
        }
    }

    unique_handles.clear();
    idx = 0;
    unsigned short to_rem_count = 0;
    for (std::vector< std::string >::const_iterator handle_ptr = _tech_contacts_rem.begin();
         handle_ptr != _tech_contacts_rem.end(); ++handle_ptr, ++idx)
    {
        const HandleIndex::const_iterator handle_index_ptr = unique_handles.find(*handle_ptr);
        if (handle_index_ptr != unique_handles.end()) {//a duplicate handle
            _param_errors.add_vector_parameter_error(Param::keyset_tech_rem, idx, Reason::duplicity_contact);
            copy_error(Param::keyset_tech_rem, Reason::tech_notexist,
                       handle_index_ptr->second, idx, _param_errors);
            check_result = false;
            continue;
        }
        //unique handle
        unique_handles.insert(std::make_pair(*handle_ptr, idx));
        if (current_tech_contacts.count(*handle_ptr) == 0) {
            _param_errors.add_vector_parameter_error(Param::keyset_tech_rem, idx, Reason::tech_notexist);
            check_result = false;
        }
        else {
            ++to_rem_count;
        }
    }
    unique_handles.clear();

    if ((current_tech_contacts.size() + to_add_count) < to_rem_count) {
        _param_errors.add_scalar_parameter_error(Param::keyset_tech_rem, Reason::can_not_remove_tech);
        return false;
    }
    const unsigned short number_of_tech_contacts = current_tech_contacts.size() + to_add_count - to_rem_count;
    if (number_of_tech_contacts < KeySet::min_number_of_tech_contacts) {
        _param_errors.add_scalar_parameter_error(Param::keyset_tech_rem, Reason::can_not_remove_tech);
        return false;
    }
    if (KeySet::max_number_of_tech_contacts < number_of_tech_contacts) {
        _param_errors.add_scalar_parameter_error(Param::keyset_tech, Reason::techadmin_limit);
        return false;
    }

    return check_result;
}

template < unsigned MIN_NUMBER_OF_DS_RECORDS, unsigned MAX_NUMBER_OF_DS_RECORDS >
Success check_ds_records(const std::vector< KeySet::DsRecord >&, const std::vector< KeySet::DsRecord >&,
                         const Fred::InfoKeysetData&, Fred::OperationContext&, ParameterErrors&);

//specialization for requirement of no DS records
template < >
Success check_ds_records< 0, 0 >(const std::vector< KeySet::DsRecord > &_ds_records_add,
                                 const std::vector< KeySet::DsRecord > &_ds_records_rem,
                                 const Fred::InfoKeysetData&, Fred::OperationContext&,
                                 ParameterErrors &_param_errors)
{
    if (_ds_records_add.empty() && _ds_records_rem.empty()) {
        return true;
    }
    _param_errors.add_scalar_parameter_error(Param::keyset_dsrecord, Reason::dsrecord_limit);
    return false;
}

typedef std::set< KeySet::DnsKey > DnsKeys;
DnsKeys get_dns_keys(const std::vector< Fred::DnsKey > &_dns_keys)
{
    DnsKeys result;
    for (std::vector< Fred::DnsKey >::const_iterator dns_key_ptr = _dns_keys.begin();
         dns_key_ptr != _dns_keys.end(); ++dns_key_ptr)
    {
        result.insert(KeySet::DnsKey(dns_key_ptr->get_flags(),
                                     dns_key_ptr->get_protocol(),
                                     dns_key_ptr->get_alg(),
                                     dns_key_ptr->get_key()));
    }
    return result;
}

Success check_dns_keys(const std::vector< KeySet::DnsKey > &_dns_keys_add,
                       const std::vector< KeySet::DnsKey > &_dns_keys_rem,
                       const Fred::InfoKeysetData &_keyset_data,
                       Fred::OperationContext &_ctx,
                       ParameterErrors &_param_errors)
{
    if (_dns_keys_add.empty() && _dns_keys_rem.empty()) {//nothing to do
        return true;
    }

    Success check_result = true;

    if (KeySet::max_number_of_dns_keys < _dns_keys_add.size()) {
        _param_errors.add_scalar_parameter_error(Param::keyset_dnskey, Reason::dnskey_limit);
        check_result = false;
    }

    const DnsKeys current_dns_keys = get_dns_keys(_keyset_data.dns_keys);

    if (current_dns_keys.size() < _dns_keys_rem.size()) {
        _param_errors.add_scalar_parameter_error(Param::keyset_dnskey, Reason::no_dnskey_dsrecord);
        check_result = false;
    }

    //prevents detailed checking of too long _tech_contacts_add or _tech_contacts_rem lists
    if (!check_result) {
        return false;
    }

    typedef std::map< KeySet::DnsKey, unsigned short > DnsKeyIndex;
    DnsKeyIndex unique_dns_keys;
    unsigned short idx = 0;
    unsigned short to_add_count = 0;
    for (std::vector< KeySet::DnsKey >::const_iterator dns_key_ptr = _dns_keys_add.begin();
         dns_key_ptr != _dns_keys_add.end(); ++dns_key_ptr, ++idx)
    {
        const DnsKeyIndex::const_iterator dns_key_index_ptr = unique_dns_keys.find(*dns_key_ptr);
        if (dns_key_index_ptr != unique_dns_keys.end()) {//a duplicate dns_key
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey_add, idx, Reason::duplicity_dnskey);
            copy_error(Param::keyset_dnskey_add, Reason::dnskey_exist,
                       dns_key_index_ptr->second, idx, _param_errors);
            copy_error(Param::keyset_dnskey_add, Reason::dnskey_bad_flags,
                       dns_key_index_ptr->second, idx, _param_errors);
            copy_error(Param::keyset_dnskey_add, Reason::dnskey_bad_protocol,
                       dns_key_index_ptr->second, idx, _param_errors);
            copy_error(Param::keyset_dnskey_add, Reason::dnskey_bad_alg,
                       dns_key_index_ptr->second, idx, _param_errors);
            copy_error(Param::keyset_dnskey_add, Reason::dnskey_bad_key_char,
                       dns_key_index_ptr->second, idx, _param_errors);
            copy_error(Param::keyset_dnskey_add, Reason::dnskey_bad_key_len,
                       dns_key_index_ptr->second, idx, _param_errors);
            check_result = false;
            continue;
        }

        //unique DNS key
        unique_dns_keys.insert(std::make_pair(*dns_key_ptr, idx));
        if (0 < current_dns_keys.count(*dns_key_ptr)) {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey_add, idx, Reason::dnskey_exist);
            check_result = false;
            continue;
        }

        bool is_dns_key_correct = true;

        if (!dns_key_ptr->is_flags_correct())
        {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey_add, idx, Reason::dnskey_bad_flags);
            is_dns_key_correct = false;
        }

        if (dns_key_ptr->get_protocol() != 3u)
        {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey_add, idx, Reason::dnskey_bad_protocol);
            is_dns_key_correct = false;
        }

        if ((dns_key_ptr->get_alg() < 0u) || (255u < dns_key_ptr->get_alg()))
        {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey_add, idx, Reason::dnskey_bad_alg);
            is_dns_key_correct = false;
        }

        switch (dns_key_ptr->check_key())
        {
            case KeySet::DnsKey::CheckKey::ok:
                break;
            case KeySet::DnsKey::CheckKey::bad_char:
                _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::dnskey_bad_key_char);
                is_dns_key_correct = false;
                break;
            case KeySet::DnsKey::CheckKey::bad_length:
                _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::dnskey_bad_key_len);
                is_dns_key_correct = false;
                break;
        }
        if (!is_dns_key_correct) {
            check_result = false;
            continue;
        }

        ++to_add_count;
    }

    unique_dns_keys.clear();
    idx = 0;
    unsigned short to_rem_count = 0;
    for (std::vector< KeySet::DnsKey >::const_iterator dns_key_ptr = _dns_keys_rem.begin();
         dns_key_ptr != _dns_keys_rem.end(); ++dns_key_ptr, ++idx)
    {
        const DnsKeyIndex::const_iterator dns_key_index_ptr = unique_dns_keys.find(*dns_key_ptr);
        if (dns_key_index_ptr != unique_dns_keys.end()) {//a duplicate DNS key
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey_rem, idx, Reason::duplicity_dnskey);
            copy_error(Param::keyset_dnskey_rem, Reason::dnskey_notexist,
                       dns_key_index_ptr->second, idx, _param_errors);
            check_result = false;
            continue;
        }
        //unique DNS key
        unique_dns_keys.insert(std::make_pair(*dns_key_ptr, idx));
        if (current_dns_keys.count(*dns_key_ptr) == 0) {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey_rem, idx, Reason::dnskey_notexist);
            check_result = false;
            continue;
        }
        ++to_rem_count;
    }
    unique_dns_keys.clear();

    if ((current_dns_keys.size() + to_add_count) < to_rem_count) {
        _param_errors.add_scalar_parameter_error(Param::keyset_dnskey, Reason::no_dnskey_dsrecord);
        return false;
    }
    const unsigned short number_of_dns_keys = current_dns_keys.size() + to_add_count - to_rem_count;
    if (number_of_dns_keys < KeySet::min_number_of_dns_keys) {
        _param_errors.add_scalar_parameter_error(Param::keyset_dnskey, Reason::no_dnskey_dsrecord);
        return false;
    }
    if (KeySet::max_number_of_dns_keys < number_of_dns_keys) {
        _param_errors.add_scalar_parameter_error(Param::keyset_dnskey, Reason::dnskey_limit);
        return false;
    }

    return check_result;
}

std::vector< Fred::DnsKey > to_fred(const std::vector< KeySet::DnsKey > &_dns_keys)
{
    std::vector< Fred::DnsKey > result;
    for (std::vector< KeySet::DnsKey >::const_iterator dns_key_ptr = _dns_keys.begin();
         dns_key_ptr != _dns_keys.end(); ++dns_key_ptr)
    {
        result.push_back(Fred::DnsKey(dns_key_ptr->get_flags(),
                                      dns_key_ptr->get_protocol(),
                                      dns_key_ptr->get_alg(),
                                      dns_key_ptr->get_key()));
    }
    return result;
}

}//namespace Epp::{anonymous}

KeysetUpdateResult keyset_update(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts_add,
    const std::vector< std::string > &_tech_contacts_rem,
    const std::vector< KeySet::DsRecord > &_ds_records_add,
    const std::vector< KeySet::DsRecord > &_ds_records_rem,
    const std::vector< KeySet::DnsKey > &_dns_keys_add,
    const std::vector< KeySet::DnsKey > &_dns_keys_rem,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        throw AuthErrorServerClosingConnection();
    }

    KeysetUpdateResult result;
    std::string callers_registrar_handle;
    {
        const Fred::InfoKeysetData keyset_data = check_keyset_handle(_keyset_handle,
                                                                     _registrar_id,
                                                                     _ctx,
                                                                     callers_registrar_handle);
        ParameterErrors param_errors;

        if (!check_tech_contacts(_tech_contacts_add, _tech_contacts_rem, keyset_data, _ctx, param_errors)) {
            _ctx.get_log().info("check_tech_contacts failure");
        }

        if (!check_ds_records< KeySet::min_number_of_ds_records,
                               KeySet::max_number_of_ds_records >(_ds_records_add, _ds_records_rem,
                                                                  keyset_data, _ctx, param_errors))
        {
            _ctx.get_log().info("check_ds_records failure");
        }

        if (!check_dns_keys(_dns_keys_add, _dns_keys_rem, keyset_data, _ctx, param_errors)) {
            _ctx.get_log().info("check_dns_keys failure");
        }

        if (!param_errors.is_empty()) {
            throw param_errors;
        }
        result.id = keyset_data.id;
    }

    try {
        const std::vector< Fred::DnsKey > dns_keys_add = to_fred(_dns_keys_add);
        const std::vector< Fred::DnsKey > dns_keys_rem = to_fred(_dns_keys_rem);
        result.update_history_id = Fred::UpdateKeyset(
            _keyset_handle,
            callers_registrar_handle,
            Optional< std::string >(),//do not change sponsoring registrar
            _auth_info_pw,
            _tech_contacts_add,
            _tech_contacts_rem,
            dns_keys_add,
            dns_keys_rem,
            _logd_request_id).exec(_ctx);
        return result;
    }
    catch (const Fred::UpdateKeyset::Exception &e) {
        //general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_keyset_handle()) {
            _ctx.get_log().warning("unknown keyset handle");
        }
        if (e.is_set_unknown_registrar_handle()) {
            _ctx.get_log().warning("unknown registrar handle");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle()) {
            _ctx.get_log().warning("unknown technical contact handle");
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle()) {
            _ctx.get_log().warning("duplicate technical contact handle");
        }
        if (e.is_set_vector_of_unassigned_technical_contact_handle()) {
            _ctx.get_log().warning("unassigned technical contact handle");
        }
        if (e.is_set_vector_of_already_set_dns_key()) {
            _ctx.get_log().warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unassigned_dns_key()) {
            _ctx.get_log().warning("unassigned dns key");
        }
        throw;
    }
}

}
