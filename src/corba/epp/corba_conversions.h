/*
 * Copyright (C) 2015  CZ.NIC, z.s.p.o.
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
 *  @file
 */

#ifndef CORBA_EPP_CORBA_CONVERSIONS_4505534138350
#define CORBA_EPP_CORBA_CONVERSIONS_4505534138350

#include "src/corba/EPP.hh"

#include "src/epp/request_params.h"
#include "src/epp/contact/contact_info.h"
#include "src/epp/contact/contact_create.h"
#include "src/epp/contact/contact_update.h"
#include "src/epp/contact/contact_check.h"
#include "src/epp/localized_response.h"

#include <boost/optional.hpp>

namespace Corba {

    Epp::ContactCreateInputData unwrap_contact_create_input_data(const char* const handle, const ccReg::ContactChange& c);

    Epp::ContactUpdateInputData unwrap_contact_update_input_data(const char* const handle, const ccReg::ContactChange& c);

    std::vector<std::string> unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles);

    Epp::RequestParams unwrap_epp_request_params(const ccReg::EppParams& _epp_request_params);

    /**
     * Computes disclosability.
     * @param is_set value converted to disclosability
     * @param mode selects conversion method
     * @return true if combination of is_set and mode enables disclosing otherwise false
     */
    bool should_be_disclosed(CORBA::Boolean is_set, ccReg::Disclose mode);

    /**
     * Computes item's new disclose flag value.
     * @param is_set value converted to item's disclosability
     * @param mode selects conversion method
     * @return unset value if flag has to be untached, set on true if item has to be disclosed otherwise set on false
     */
    Optional< bool > get_new_disclose_flag_value(CORBA::Boolean is_set, ccReg::Disclose mode);


    ccReg::Response wrap_response(const Epp::LocalizedSuccessResponse& _input, const std::string& _server_transaction_handle);

    ccReg::EPP::EppError wrap_error(const Epp::LocalizedFailResponse& _input, const std::string& _server_transaction_handle);

    ccReg::Contact wrap_localized_info_contact(const Epp::LocalizedContactInfoOutputData& _input );

    /**
     * Converts database stored disclose flag value to EPP disclose flag with respect to default policy.
     */
    CORBA::Boolean wrap_disclose_flag_to_Boolean(bool is_set);

    /**
     * @returns data ordered the same way as input contact_handles
     */
    ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map< std::string, boost::optional< Epp::LocalizedContactHandleRegistrationObstruction > >& contact_handle_check_results
    );
}

#endif
