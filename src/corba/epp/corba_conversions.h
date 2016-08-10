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
#include "src/epp/contact/contact_change.h"
#include "src/epp/localized_response.h"

namespace Corba {

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactCreateInputData &dst);

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactUpdateInputData &dst);

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactChange &dst);

    std::vector<std::string> unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles);

    Epp::RequestParams unwrap_epp_request_params(const ccReg::EppParams& _epp_request_params);


    ccReg::Response wrap_response(const Epp::LocalizedSuccessResponse& _input, const std::string& _server_transaction_handle);

    ccReg::EPP::EppError wrap_error(const Epp::LocalizedFailResponse& _input, const std::string& _server_transaction_handle);

    void wrap_LocalizedContactInfoOutputData(const Epp::LocalizedContactInfoOutputData &src, ccReg::Contact &dst);

    /**
     * @returns data ordered the same way as input contact_handles
     */
    ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map< std::string, boost::optional< Epp::LocalizedContactHandleRegistrationObstruction > >& contact_handle_check_results
    );
}

#endif
