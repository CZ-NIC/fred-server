#include "src/corba/util/corba_conversions_string.h"
#include <stdexcept>

namespace Corba {
    CORBA::String_member wrap_string(const std::string& in) {
        return CORBA::string_dup(in.c_str());
    }

    std::string unwrap_string(const CORBA::String_member& in) {
        std::string out;

        const char * raw = static_cast<const char*>(in);
        if(raw != NULL) {
            out.assign(raw);
        }

        return out;
    }

    std::string unwrap_string_from_const_char_ptr(const char* in)
    {
        if(in == 0)
        {
            throw std::runtime_error("unwrap_string_from_const_char_ptr: in is NULL");
        }
        return std::string(in);
    }
}
