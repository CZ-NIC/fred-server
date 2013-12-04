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
}

#endif // end of #include guard



