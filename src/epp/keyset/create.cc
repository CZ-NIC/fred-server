#include "src/epp/keyset/create.h"
#include "src/epp/keyset/limits.h"

#include "src/epp/error.h"
#include "src/epp/exception.h"
#include "src/epp/parameter_errors.h"
#include "src/epp/reason.h"

#include "src/fredlib/registrar/info_registrar.h"

#include "src/fredlib/keyset/create_keyset.h"
#include "src/fredlib/keyset/check_keyset.h"

#include "src/fredlib/contact/check_contact.h"

#include <map>

namespace Epp {

namespace {

typedef bool Success;

Success check_keyset_handle(const std::string &_keyset_handle,
                            Fred::OperationContext &_ctx,
                            ParameterErrors &_param_errors)
{
    switch (Fred::KeySet::get_handle_syntax_validity(_keyset_handle))
    {
        case Fred::KeySet::HandleState::valid:
            switch (Fred::KeySet::get_handle_registrability(_ctx, _keyset_handle))
            {
                case Fred::KeySet::HandleState::registered:
                    _param_errors.add_scalar_parameter_error(Param::keyset_handle, Reason::existing);
                    return false;
                case Fred::KeySet::HandleState::in_protection_period:
                    _param_errors.add_scalar_parameter_error(Param::keyset_handle, Reason::protected_period);
                    return false;
                case Fred::KeySet::HandleState::available:
                    return true;
            }
            throw std::runtime_error("unexpected keyset handle registrability value");
        case Fred::KeySet::HandleState::invalid:
            _param_errors.add_scalar_parameter_error(Param::keyset_handle, Reason::bad_format_keyset_handle);
            return false;
    }
    throw std::runtime_error("unexpected keyset handle syntax validity value");
}

Success check_tech_contacts(const std::vector< std::string > &_tech_contacts,
                            Fred::OperationContext &_ctx,
                            ParameterErrors &_param_errors)
{
    if (_tech_contacts.size() < KeySet::min_number_of_tech_contacts) {
        _param_errors.add_scalar_parameter_error(Param::keyset_tech, Reason::technical_contact_not_registered);
        return false;
    }
    if (KeySet::max_number_of_tech_contacts < _tech_contacts.size()) {
        _param_errors.add_scalar_parameter_error(Param::keyset_tech, Reason::techadmin_limit);
        return false;
    }

    Success existing_tech_contacts = true;
    typedef std::map< std::string, unsigned short > HandleIndex;
    HandleIndex unique_handles;
    unsigned short idx = 0;
    for (std::vector< std::string >::const_iterator handle_ptr = _tech_contacts.begin();
         handle_ptr != _tech_contacts.end(); ++handle_ptr, ++idx)
    {
        const HandleIndex::const_iterator handle_index_ptr = unique_handles.find(*handle_ptr);
        if (handle_index_ptr != unique_handles.end()) {//duplicate handle
            _param_errors.add_vector_parameter_error(Param::keyset_tech, idx, Reason::duplicated_contact);
            if (_param_errors.has_vector_parameter_error_at(Param::keyset_tech,
                                                            handle_index_ptr->second,
                                                            Reason::technical_contact_not_registered))
            {
                _param_errors.add_vector_parameter_error(Param::keyset_tech, idx, Reason::technical_contact_not_registered);
            }
            existing_tech_contacts = false;
        }
        else {//unique handle
            unique_handles.insert(std::make_pair(*handle_ptr, idx));
            switch (Fred::Contact::get_handle_registrability(_ctx, *handle_ptr))
            {
                case Fred::ContactHandleState::Registrability::registered:
                    break;
                case Fred::ContactHandleState::Registrability::available:
                case Fred::ContactHandleState::Registrability::in_protection_period:
                    _param_errors.add_vector_parameter_error(Param::keyset_tech, idx, Reason::technical_contact_not_registered);
                    existing_tech_contacts = false;
                    break;
            }
        }
    }
    return existing_tech_contacts;
}

template < unsigned MIN_NUMBER_OF_DS_RECORDS, unsigned MAX_NUMBER_OF_DS_RECORDS >
Success check_ds_records(const std::vector< KeySet::DsRecord >&, ParameterErrors&);

//specialization for requirement of no ds_record
template < >
Success check_ds_records< 0, 0 >(const std::vector< KeySet::DsRecord > &_ds_records, ParameterErrors &_param_errors)
{
    if (_ds_records.empty()) {
        return true;
    }
    _param_errors.add_scalar_parameter_error(Param::keyset_dsrecord, Reason::dsrecord_limit);
    return false;
}

Success check_dns_keys(const std::vector< KeySet::DnsKey > &_dns_keys,
                       Fred::OperationContext &_ctx,
                       ParameterErrors &_param_errors)
{
    if (_dns_keys.size() < KeySet::min_number_of_dns_keys) {
        _param_errors.add_scalar_parameter_error(Param::keyset_dnskey, Reason::no_dnskey);
        return false;
    }
    if (KeySet::max_number_of_dns_keys < _dns_keys.size()) {
        _param_errors.add_scalar_parameter_error(Param::keyset_dnskey, Reason::dnskey_limit);
        return false;
    }

    typedef std::map< KeySet::DnsKey, unsigned short > DnsKeyIndex;
    DnsKeyIndex unique_dns_keys;
    Success correct = true;
    unsigned short idx = 0;
    for (std::vector< KeySet::DnsKey >::const_iterator dns_key_ptr = _dns_keys.begin();
         dns_key_ptr != _dns_keys.end(); ++dns_key_ptr, ++idx)
    {
        //check DNS key uniqueness
        const DnsKeyIndex::const_iterator dns_key_index_ptr = unique_dns_keys.find(*dns_key_ptr);
        if (dns_key_index_ptr != unique_dns_keys.end()) {//duplicate DNS key
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::duplicated_dnskey);
            static const Param::Enum param = Param::keyset_dnskey;
            static const Reason::Enum reasons[] =
            {
                Reason::dnskey_bad_flags,
                Reason::dnskey_bad_protocol,
                Reason::dnskey_bad_alg,
                Reason::dnskey_bad_key_char,
                Reason::dnskey_bad_key_len
            };
            static const std::size_t number_of_reasons = sizeof(reasons) / sizeof(*reasons);
            static const Reason::Enum *const reasons_end = reasons + number_of_reasons;
            for (const Reason::Enum *reason_ptr = reasons; reason_ptr < reasons_end; ++reason_ptr) {
                const Reason::Enum reason = *reason_ptr;
                const unsigned short index_of_first_occurrence = dns_key_index_ptr->second;
                //the duplicate DNS key has the same errors as the first
                if (_param_errors.has_vector_parameter_error_at(param, index_of_first_occurrence, reason)) {
                    _param_errors.add_vector_parameter_error(param, idx, reason);
                }
            }
            correct = false;
            continue;
        }

        //unique DNS key
        unique_dns_keys.insert(std::make_pair(*dns_key_ptr, idx));

        if (!dns_key_ptr->is_flags_correct())
        {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::dnskey_bad_flags);
            correct = false;
        }

        if (!dns_key_ptr->is_protocol_correct())
        {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::dnskey_bad_protocol);
            correct = false;
        }

        if (!dns_key_ptr->is_alg_correct())
        {
            _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::dnskey_bad_alg);
            correct = false;
        }

        switch (dns_key_ptr->check_key())
        {
            case KeySet::DnsKey::CheckKey::ok:
                break;
            case KeySet::DnsKey::CheckKey::bad_char:
                _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::dnskey_bad_key_char);
                correct = false;
                break;
            case KeySet::DnsKey::CheckKey::bad_length:
                _param_errors.add_vector_parameter_error(Param::keyset_dnskey, idx, Reason::dnskey_bad_key_len);
                correct = false;
                break;
        }
    }
    return correct;
}

}//namespace Epp::{anonymous}

KeysetCreateResult keyset_create(
    Fred::OperationContext &_ctx,
    const std::string &_keyset_handle,
    const Optional< std::string > &_auth_info_pw,
    const std::vector< std::string > &_tech_contacts,
    const std::vector< KeySet::DsRecord > &_ds_records,
    const std::vector< KeySet::DnsKey > &_dns_keys,
    unsigned long long _registrar_id,
    const Optional< unsigned long long > &_logd_request_id)
{
    static const unsigned long long invalid_registrar_id = 0;
    if (_registrar_id == invalid_registrar_id) {
        throw AuthErrorServerClosingConnection();
    }

    {
        ParameterErrors param_errors;

        if (!check_keyset_handle(_keyset_handle, _ctx, param_errors)) {
            _ctx.get_log().info("check_keyset_handle failure");
        }
        if (!check_tech_contacts(_tech_contacts, _ctx, param_errors)) {
            _ctx.get_log().info("check_tech_contacts failure");
        }
        if (!check_ds_records< KeySet::min_number_of_ds_records,
                               KeySet::max_number_of_ds_records >(_ds_records, param_errors)) {
            _ctx.get_log().info("check_ds_records failure");
        }
        if (!check_dns_keys(_dns_keys, _ctx, param_errors)) {
            _ctx.get_log().info("check_dns_keys failure");
        }

        if (!param_errors.is_empty()) {
            throw param_errors;
        }
    }

    try {
        std::vector< Fred::DnsKey > dns_keys;
        for (std::vector< KeySet::DnsKey >::const_iterator dns_key_ptr = _dns_keys.begin();
             dns_key_ptr != _dns_keys.end(); ++dns_key_ptr)
        {
            dns_keys.push_back(Fred::DnsKey(dns_key_ptr->get_flags(),
                                            dns_key_ptr->get_protocol(),
                                            dns_key_ptr->get_alg(),
                                            dns_key_ptr->get_key()));
        }
        const Fred::CreateKeyset::Result op_result = Fred::CreateKeyset(
            _keyset_handle,
            Fred::InfoRegistrarById(_registrar_id).exec(_ctx).info_registrar_data.handle,
            _auth_info_pw,
            dns_keys,
            _tech_contacts).exec(_ctx, _logd_request_id, "UTC");

        KeysetCreateResult result;
        result.id = op_result.create_object_result.object_id,
        result.create_history_id = op_result.create_object_result.history_id,
        result.crdate = op_result.creation_time;
        return result;
    }
    catch (const Fred::CreateKeyset::Exception &e) {
        //general errors (possibly but not NECESSARILLY caused by input data) signalizing unknown/bigger problems have priority
        if (e.is_set_unknown_registrar_handle()) {
            _ctx.get_log().warning("unknown registrar handle");
        }
        if (e.is_set_vector_of_already_set_dns_key()) {
            _ctx.get_log().warning("duplicate dns key");
        }
        if (e.is_set_vector_of_unknown_technical_contact_handle()) {
            _ctx.get_log().warning("unknown technical contact handle");
        }
        if (e.is_set_vector_of_already_set_technical_contact_handle()) {
            _ctx.get_log().warning("duplicate technical contact handle");
        }
        throw;
    }
}

}
