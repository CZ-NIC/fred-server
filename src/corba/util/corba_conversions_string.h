#ifndef CORBA_UTIL_CORBA_CONVERSIONS_STRING_245658434125
#define CORBA_UTIL_CORBA_CONVERSIONS_STRING_245658434125

/**
 *  @file
 *  conversions to and from CORBA mapping for string types
 */

#include <omniORB4/CORBA.h>
#include <string>


namespace Corba {
    CORBA::String_member wrap_string(const std::string& in);
    std::string unwrap_string(const CORBA::String_member& in);

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

}

#endif // end of #include guard



