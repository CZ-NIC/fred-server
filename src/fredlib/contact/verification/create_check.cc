/*
 * Copyright (C) 2013  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file create_check.cc
 *  create contact check
 */

#include <boost/assign/list_of.hpp>
#include <boost/algorithm/string/join.hpp>
/* TODO - FIXME - only temporary for uuid mockup */
#include  <cstdlib>

#include "fredlib/contact/verification/create_check.h"
#include "fredlib/contact/verification/enum_check_status.h"

namespace Fred
{
    CreateContactCheck::CreateContactCheck(
        const std::string& _contact_handle,
        const std::string& _testsuite_name
    ) :
        contact_handle_(_contact_handle),
        testsuite_name_(_testsuite_name)
    { }

    CreateContactCheck::CreateContactCheck(
        const std::string&  _contact_handle,
        const std::string&  _testsuite_name,
        Optional<long long> _logd_request_id
    ) :
        contact_handle_(_contact_handle),
        testsuite_name_(_testsuite_name),
        logd_request_id_(
            ( _logd_request_id.isset() )
                ?
                Nullable<long long>( _logd_request_id.get_value() )
                :
                Nullable<long long>()
        )
    { }

    CreateContactCheck& CreateContactCheck::set_logd_request_id(long long _logd_request_id) {
        logd_request_id_ = _logd_request_id;
        return *this;
    }

    std::string CreateContactCheck::exec(OperationContext& _ctx) {
        /* TODO - FIXME
           temporary ugliness - waiting for boost 1.42 to use boost::uuid
           refer to ticket #9148 for complete saga of UUID handles

           after 1.42 is available just
            - change handle generating command to lowercase
            - change function generate to operator()
            - include these:
              #include <boost/uuid/uuid.hpp>            // uuid class
              #include <boost/uuid/uuid_generators.hpp> // generators
              #include <boost/uuid/uuid_io.hpp>         // streaming operators etc.
         */

        struct BOOST { struct UUIDS { struct RANDOM_GENERATOR {
            static std::string generate() {
                srand(time(NULL));
                std::vector<unsigned char> bytes;

                // generate random 128bits = 16 bytes
                for (int i = 0; i < 16; ++i) {
                    bytes.push_back( rand()%256 );
                }
                /* some specific uuid rules
                 * http://www.cryptosys.net/pki/Uuid.c.html
                 */
                bytes.at(6) = static_cast<char>(0x40 | (bytes.at(6) & 0xf));
                bytes.at(8) = static_cast<char>(0x80 | (bytes.at(8) & 0x3f));

                // buffer for hex representation of one byte + terminating zero
                char hex_rep[3];

                // converting raw bytes to hex string representation
                std::string result;
                for (std::vector<unsigned char>::iterator it = bytes.begin(); it != bytes.end(); ++it) {
                    sprintf(hex_rep,"%02x",*it);
                    // conversion target is hhhh - so in case it gets wrong just cut off the tail
                    hex_rep[2] = 0;
                    result += hex_rep;
                }

                // hyphens for canonical form
                result.insert(8, "-");
                result.insert(13, "-");
                result.insert(18, "-");
                result.insert(23, "-");

                return result;
            }
        }; }; };

        /* end of temporary ugliness - please cut and replace between ASAP*/

        std::string handle = boost::lexical_cast<std::string>(BOOST::UUIDS::RANDOM_GENERATOR::generate());

        try {
            _ctx.get_conn().exec_params(
                "INSERT INTO contact_check ( "
                "   handle,"
                "   contact_history_id,"
                "   enum_contact_testsuite_id,"
                "   enum_contact_check_status_id,"
                "   logd_request_id"
                ")"
                "VALUES ("
                "   $1::uuid,"
                "   (SELECT historyid"
                "       FROM object_registry"
                "       WHERE name=$2::varchar),"
                "   (SELECT id FROM enum_contact_testsuite WHERE name=$3::varchar),"
                "   (SELECT id FROM enum_contact_check_status WHERE name=$4::varchar),"
                "   $5::bigint"
                ");",
                Database::query_param_list
                    (handle)
                    (contact_handle_)
                    (testsuite_name_)
                    (Fred::ContactCheckStatus::ENQUEUED)
                    (logd_request_id_)
            );
        } catch(const std::exception& _exc) {

            std::string what_string(_exc.what());

            if(what_string.find("fk_contact_check_contact_history_id") != std::string::npos) {
                throw ExceptionUnknownContactHandle();
            }

            if(what_string.find("contact_check_fk_Enum_contact_testsuite_id") != std::string::npos) {
                throw ExceptionUnknownTestsuiteName();
            }

            if(what_string.find("fk_contact_check_contact_history_id") != std::string::npos) {
                throw ExceptionCheckAlreadyExists();
            }

            // problem was elsewhere so let it propagate
            throw;
        }

        return handle;
    }

    std::ostream& operator<<(std::ostream& os, const CreateContactCheck& i) {
        os << "#CreateContactCheck"
            << " contact_handle_: "  << i.contact_handle_
            << " testsuite_name_: "  << i.testsuite_name_
            << " logd_request_id_: " << i.logd_request_id_.print_quoted();

        return os;
    }

    std::string CreateContactCheck::to_string() const {
        std::stringstream ss;
        ss << *this;
        return ss.str();
    }

} // namespace Fred
