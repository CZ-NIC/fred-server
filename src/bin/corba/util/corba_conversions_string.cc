#include "src/bin/corba/util/corba_conversions_string.hh"

#include <stdexcept>

namespace LibFred {
namespace Corba {

CORBA::String_var wrap_string(const std::string& in)
{
    return CORBA::string_dup(in.c_str());
}


std::string unwrap_string(const char* in)
{
    std::string out;

    if (in != NULL)
    {
        out.assign(in);
    }

    return out;
}


std::string unwrap_string_from_const_char_ptr(const char* in)
{
    if (in == NULL)
    {
        throw std::runtime_error("unwrap_string_from_const_char_ptr: in is NULL");
    }
    return std::string(in);
}


CORBA::String_var wrap_const_char_ptr_to_corba_string(const char* in)
{
    if (in == NULL)
    {
        throw std::runtime_error("wrap_const_char_ptr_to_string_member: in is NULL");
    }
    return CORBA::string_dup(in);
}


CORBA::String_var wrap_string_to_corba_string(const std::string& in)
{
    return CORBA::string_dup(in.c_str());
}


} // namespace LibFred::Corba
} // namespace LibFred
