#include "src/epp/keyset/check_keyset.h"
#include "src/fredlib/keyset/check_keyset.h"

namespace Epp {

namespace {

Nullable< Keyset::HandleCheckResult::Enum > validity_to_check_result(
    Fred::Keyset::HandleState::SyntaxValidity _validity)
{
    switch (_validity)
    {
        case Fred::Keyset::HandleState::invalid: return Keyset::HandleCheckResult::invalid_handle;
        case Fred::Keyset::HandleState::valid: return Nullable< Keyset::HandleCheckResult::Enum >();
    }

    throw std::runtime_error("Invalid Fred::Keyset::HandleState::SyntaxValidity value.");
}

Nullable< Keyset::HandleCheckResult::Enum > keyset_handle_state_to_check_result(
    Fred::Keyset::HandleState::SyntaxValidity _handle_validity,
    Fred::Keyset::HandleState::Registrability _handle_registrability)
{
    switch (_handle_registrability)
    {
        case Fred::Keyset::HandleState::registered           : return Keyset::HandleCheckResult::registered_handle;
        case Fred::Keyset::HandleState::in_protection_period : return Keyset::HandleCheckResult::protected_handle;
        case Fred::Keyset::HandleState::available            : return validity_to_check_result(_handle_validity);
    }

    throw std::runtime_error("Invalid Fred::Keyset::HandleState::Registrability value.");
}

}//namespace Epp::{anonymous}

std::map< std::string, Nullable< Keyset::HandleCheckResult::Enum > > keyset_check(
    Fred::OperationContext &_ctx,
    const std::set< std::string > &_keyset_handles)
{
    typedef std::set< std::string > Handles;
    typedef std::map< std::string, Nullable< Keyset::HandleCheckResult::Enum > > CheckResult;

    CheckResult result;

    for (Handles::const_iterator handle_ptr = _keyset_handles.begin();
         handle_ptr != _keyset_handles.end(); ++handle_ptr)
    {
        result[*handle_ptr] = keyset_handle_state_to_check_result(
            Fred::Keyset::get_handle_syntax_validity(*handle_ptr),
            Fred::Keyset::get_handle_registrability(_ctx, *handle_ptr));
    }

    return result;
}

}//namespace Epp
