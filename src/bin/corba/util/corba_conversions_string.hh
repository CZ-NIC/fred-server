#ifndef CORBA_CONVERSIONS_STRING_HH_2D28E6D86C3846CB8A87B5EE75629358
#define CORBA_CONVERSIONS_STRING_HH_2D28E6D86C3846CB8A87B5EE75629358

/**
 *  @file
 *  conversions to and from CORBA mapping for string types
 */

#include "util/db/nullable.hh"
#include <omniORB4/CORBA.h>
#include <string>


namespace LibFred {
namespace Corba {

/** DEPRECATED - use wrap_string_to_corba_string instead. */
CORBA::String_var wrap_string(const std::string& in);


std::string unwrap_string(const char* in);


/**
 * Make std::string from servant in string and check for illegal NULL pointer
 * @param in is OMG IDL C++ mapping for in string
 * @return std::string
 * @throws std::exception if in is NULL
 */
std::string unwrap_string_from_const_char_ptr(const char* in);


/**
 * Make CORBA string from C string
 * @param in is input data
 * @return input data allocated by CORBA::string_dup
 * @throws std::exception if in is NULL
 */
CORBA::String_var wrap_const_char_ptr_to_corba_string(const char* in);


/**
 * Make CORBA string from string
 * @param in is input data
 * @return input data allocated by CORBA::string_dup
 */
CORBA::String_var wrap_string_to_corba_string(const std::string& in);


/**
 * Make Nullable CORBA string from optional string.
 * Method get_value() of input value throwing std::exception if value is missing required.
 * @param in is input data
 * @return input data allocated by CORBA::string_dup
 */
template <class OPTIONAL_TYPE>
Nullable<CORBA::String_var> wrap_optional_string_to_nullable_corba_string(const OPTIONAL_TYPE& in)
{
    try
    {
        in.get_value();
    }
    catch (std::exception&)
    {
        return Nullable<CORBA::String_var>();
    }

    return Nullable<CORBA::String_var>(CORBA::string_dup(in.get_value().c_str()));
}


} // namespace LibFred::Corba
} // namespace LibFred

#endif
