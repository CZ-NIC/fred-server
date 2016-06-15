#include "src/epp/keyset/dns_key.h"

#include <ctype.h>

namespace Epp {
namespace KeySet {

namespace {

bool is_base64_data_character(char c)
{
    switch (c)
    {
        case '0' ... '9':
        case 'A' ... 'Z':
        case 'a' ... 'z':
        case '+':
        case '/':
            return true;
    }
    return false;
}

bool is_base64_pad_character(char c)
{
    return c == '=';
}

bool is_base64_insignificant_allowed_character(char c)
{
    return ::isspace(c);
}

}//namespace Epp::KeySet::{anonymous}

//DNS key item flags has only 3 allowed values: 0, 256, 257
bool DnsKey::is_flags_correct()const
{
    switch (flags_)
    {
        case   0u:
        case 256u:
        case 257u:
            return true;
    }
    return false;
}

//DNS key item protocol has only 1 allowed value: 3
bool DnsKey::is_protocol_correct()const
{
    return protocol_ == 3u;
}

//DNS key item alg occupies 8 bits => range <0, 255>
bool DnsKey::is_alg_correct()const
{
    return (0u <= alg_) && (alg_ <= 255u);
}

DnsKey::CheckKey::Result DnsKey::check_key()const
{
    unsigned len = 0;
    unsigned pads = 0;
    for (std::string::const_iterator c_ptr = key_.begin(); c_ptr != key_.end(); ++c_ptr) {
        if (is_base64_insignificant_allowed_character(*c_ptr)) {
            continue;
        }
        const bool no_pad_character_yet = (pads == 0);
        if (no_pad_character_yet && is_base64_data_character(*c_ptr)) {
            ++len;
            continue;
        }
        if (is_base64_pad_character(*c_ptr)) {
            static const unsigned max_number_of_pads = 2;
            if (max_number_of_pads <= pads) {
                return CheckKey::bad_char;
            }
            ++len;
            ++pads;
            continue;
        }
        //neither data character nor pad nor space
        return CheckKey::bad_char;
    }
    return (len % 4) == 0 ? CheckKey::ok
                          : CheckKey::bad_length;
}

}//namespace Epp::KeySet
}//namespace Epp
