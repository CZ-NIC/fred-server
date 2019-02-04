/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "src/backend/epp/keyset/dns_key.hh"
#include "libfred/registrable_object/keyset/check_dns_key.hh"

namespace Epp {
namespace Keyset {

namespace {

bool is_base64_data_character(char c)
{
    switch (c)
    {
        // '0' ... '9'
        case '0': // 1.
        case '1': // 2.
        case '2': // 3.
        case '3': // 4.
        case '4': // 5.
        case '5': // 6.
        case '6': // 7.
        case '7': // 8.
        case '8': // 9.
        case '9': // 10.

        // 'A' ... 'Z'
        case 'A': // 11.
        case 'B': // 12.
        case 'C': // 13.
        case 'D': // 14.
        case 'E': // 15.
        case 'F': // 16.
        case 'G': // 17.
        case 'H': // 18.
        case 'I': // 19.
        case 'J': // 20.
        case 'K': // 21.
        case 'L': // 22.
        case 'M': // 23.
        case 'N': // 24.
        case 'O': // 25.
        case 'P': // 26.
        case 'Q': // 27.
        case 'R': // 28.
        case 'S': // 29.
        case 'T': // 30.
        case 'U': // 31.
        case 'V': // 32.
        case 'W': // 33.
        case 'X': // 34.
        case 'Y': // 35.
        case 'Z': // 36.

        // 'a' ... 'z'
        case 'a': // 37.
        case 'b': // 38.
        case 'c': // 39.
        case 'd': // 40.
        case 'e': // 41.
        case 'f': // 42.
        case 'g': // 43.
        case 'h': // 44.
        case 'i': // 45.
        case 'j': // 46.
        case 'k': // 47.
        case 'l': // 48.
        case 'm': // 49.
        case 'n': // 50.
        case 'o': // 51.
        case 'p': // 52.
        case 'q': // 53.
        case 'r': // 54.
        case 's': // 55.
        case 't': // 56.
        case 'u': // 57.
        case 'v': // 58.
        case 'w': // 59.
        case 'x': // 60.
        case 'y': // 61.
        case 'z': // 62.

        // '+', '/'
        case '+': // 63.
        case '/': // 64.
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


} // namespace Epp::Keyset::{anonymous}

// DNS key item flags has only 3 allowed values: 0, 256, 257
bool DnsKey::is_flags_correct() const
{
    switch (flags_)
    {
        case 0u:
        case 256u:
        case 257u:
            return true;
    }
    return false;
}


// DNS key item protocol has only 1 allowed value: 3
bool DnsKey::is_protocol_correct() const
{
    return protocol_ == 3u;
}


DnsKey::AlgValidator::AlgValidator(LibFred::OperationContext& _ctx)
    : ctx_(_ctx)
{
}


bool DnsKey::AlgValidator::is_alg_correct(const DnsKey& _dns_key)
{
    const unsigned short alg = _dns_key.get_alg();
    const AlgNumberToIsCorrect::const_iterator alg_correctness_ptr = alg_correctness_.find(alg);
    const bool alg_correctness_is_known = alg_correctness_ptr != alg_correctness_.end();
    if (alg_correctness_is_known)
    {
        return alg_correctness_ptr->second;
    }
    const bool alg_is_correct =
        LibFred::DnsSec::get_algorithm_usability(ctx_, alg) == LibFred::DnsSec::Algorithm::usable;
    alg_correctness_.insert(std::make_pair(alg, alg_is_correct));
    return alg_is_correct;
}


DnsKey::CheckKey::Result DnsKey::check_key() const
{
    unsigned len = 0;
    unsigned pads = 0;
    for (std::string::const_iterator c_ptr = key_.begin(); c_ptr != key_.end(); ++c_ptr)
    {
        if (is_base64_insignificant_allowed_character(*c_ptr))
        {
            continue;
        }
        const bool no_pad_character_yet = (pads == 0);
        if (no_pad_character_yet && is_base64_data_character(*c_ptr))
        {
            ++len;
            continue;
        }
        if (is_base64_pad_character(*c_ptr))
        {
            static const unsigned max_number_of_pads = 2;
            if (max_number_of_pads <= pads)
            {
                return CheckKey::bad_char;
            }
            ++len;
            ++pads;
            continue;
        }
        // neither data character nor pad nor space
        return CheckKey::bad_char;
    }
    return (len % 4) == 0 ? CheckKey::ok
                          : CheckKey::bad_length;
}


} // namespace Epp::Keyset
} // namespace Epp
