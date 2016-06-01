#include "src/epp/keyset/check.h"
#include "src/fredlib/keyset/check_keyset.h"

namespace Epp {

namespace {

Nullable< KeySet::HandleCheckResult::Enum > validity_to_check_result(
    Fred::KeySet::HandleState::SyntaxValidity _validity)
{
    switch (_validity)
    {
        case Fred::KeySet::HandleState::invalid: return KeySet::HandleCheckResult::invalid_handle;
        case Fred::KeySet::HandleState::valid: return Nullable< KeySet::HandleCheckResult::Enum >();
    }

    throw std::runtime_error("Invalid Fred::KeySet::HandleState::SyntaxValidity value.");
}

Nullable< KeySet::HandleCheckResult::Enum > keyset_handle_state_to_check_result(
    Fred::KeySet::HandleState::SyntaxValidity _handle_validity,
    Fred::KeySet::HandleState::Registrability _handle_registrability)
{
    switch (_handle_registrability)
    {
        case Fred::KeySet::HandleState::registered           : return KeySet::HandleCheckResult::registered_handle;
        case Fred::KeySet::HandleState::in_protection_period : return KeySet::HandleCheckResult::protected_handle;
        case Fred::KeySet::HandleState::available            : return validity_to_check_result(_handle_validity);
    }

    throw std::runtime_error("Invalid Fred::KeySet::HandleState::Registrability value.");
}

}//namespace Epp::{anonymous}

std::map< std::string, Nullable< KeySet::HandleCheckResult::Enum > > keyset_check(
    Fred::OperationContext &_ctx,
    const std::set< std::string > &_keyset_handles)
{
    typedef std::set< std::string > Handles;
    typedef std::map< std::string, Nullable< KeySet::HandleCheckResult::Enum > > CheckResult;

    CheckResult result;

    for (Handles::const_iterator handle_ptr = _keyset_handles.begin();
         handle_ptr != _keyset_handles.end(); ++handle_ptr)
    {
        result[*handle_ptr] = keyset_handle_state_to_check_result(
            Fred::KeySet::get_handle_syntax_validity(*handle_ptr),
            Fred::KeySet::get_handle_registrability(_ctx, *handle_ptr));
    }

    return result;
}

}//namespace Epp
