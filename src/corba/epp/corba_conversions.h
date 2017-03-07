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


#include <vector>
#include <boost/optional.hpp>

#include "src/epp/nsset/nsset_dns_host_input.h"
#include "src/corba/EPP.hh"
#include "src/epp/request_params.h"
#include "src/epp/localized_response.h"
#include "src/epp/contact/contact_info.h"
#include "src/epp/contact/contact_create.h"
#include "src/epp/contact/contact_update.h"
#include "src/epp/contact/contact_check.h"
#include "src/epp/localized_response.h"
#include "src/epp/keyset/localized_info.h"
#include "src/epp/keyset/localized_check.h"
#include "src/epp/nsset/nsset_check.h"
#include "src/epp/nsset/nsset_info.h"
#include "src/epp/nsset/nsset_delete.h"
#include "src/epp/domain/domain_registration_time.h"
#include "src/epp/domain/domain_enum_validation.h"

namespace Corba {

    std::vector<std::string> unwrap_ccreg_techcontacts_to_vector_string(const ccReg::TechContact & in);

    std::vector<Epp::DNShostInput> unwrap_ccreg_dnshosts_to_vector_dnshosts(const ccReg::DNSHost& in);

    Epp::ContactCreateInputData unwrap_contact_create_input_data(const char* const handle, const ccReg::ContactChange& c);

    /**
     * Unwrapper for attributes which can be empty with special meaning
     *
     * @param _src string to be unwrapped, should not be NULL
     *
     * @return Optional() if input string empty, else unwrapped input
     */
    Optional<std::string> unwrap_string_for_change_to_Optional_string_no_trim(const char* _src);
    Optional<std::string> unwrap_string_for_change_to_Optional_string(const char* _src);

    /**
     * Unwrapper for attributes which can be empty with special meaning and can have control char with special meaning
     *
     * @param _src string to be unwrapped, should not be NULL
     *
     * @return Optional() if input string empty, empty string if input contains special control char, unwrapped input in other cases
     */
    Optional<std::string> unwrap_string_for_change_or_remove_to_Optional_string_no_trim(const char* _src);
    Optional<std::string> unwrap_string_for_change_or_remove_to_Optional_string(const char* _src);

    /**
     * Unwrapper for attributes which can be empty with special meaning and can have control char with special meaning
     *
     * @param _src string to be unwrapped, should not be NULL
     *
     * @return empty string if input string empty, Optinal(Nullable()) if input ocntains special control char, unwrapped input in other cases
     */
    Optional<Nullable<std::string> > unwrap_string_for_change_or_remove_to_Optional_Nullable_string_no_trim(const char* _src);

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactChange &dst);

    std::vector<std::string> unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles);

    std::vector< std::string > unwrap_TechContact_to_vector_string(const ccReg::TechContact &_tech_contacts);

    std::vector< Epp::KeySet::DsRecord > unwrap_ccReg_DSRecord_to_vector_Epp_KeySet_DsRecord(
        const ccReg::DSRecord &_ds_records);

    void unwrap_ccReg_DSRecord_str(const ccReg::DSRecord_str &_src, Epp::KeySet::DsRecord &_dst);

    std::vector< Epp::KeySet::DnsKey > unwrap_ccReg_DNSKey_to_vector_Epp_KeySet_DnsKey(
        const ccReg::DNSKey &_dns_keys);

    void unwrap_ccReg_DNSKey_str(const ccReg::DNSKey_str &_src, Epp::KeySet::DnsKey &_dst);

    Epp::RequestParams unwrap_EppParams(const ccReg::EppParams& _epp_request_params);

    boost::optional<short> unwrap_tech_check_level(CORBA::Short level);


    ccReg::Response wrap_response(const Epp::LocalizedSuccessResponse& _input, const std::string& _server_transaction_handle);

    void wrap_Epp_LocalizedSuccessResponse(const Epp::LocalizedSuccessResponse &_src,
                                           const std::string &_server_transaction_handle,
                                           ccReg::Response &_dst);

    ccReg::EPP::EppError wrap_error(const Epp::LocalizedFailResponse& _input, const std::string& _server_transaction_handle);

    void wrap_LocalizedContactInfoOutputData(const Epp::LocalizedContactInfoOutputData &src, ccReg::Contact &dst);

    ccReg::NSSet wrap_localized_info_nsset(const Epp::LocalizedNssetInfoOutputData& _input );

    /**
     * @returns data ordered the same way as input contact_handles
     */
    ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map<std::string, boost::optional< Epp::LocalizedContactHandleRegistrationObstruction > >& contact_handle_check_results
    );

    /**
     * @returns data ordered the same way as input handles
     */
    void wrap_Epp_KeySet_Localized_HandlesCheck_Results(
        const std::vector< std::string > &handles,
        const Epp::KeySet::Localized::HandlesCheck::Results &check_results,
        ccReg::CheckResp &dst);

    void wrap_Epp_LocalizedStates(const Epp::LocalizedStates &_src, ccReg::Status &_dst);

    void wrap_Epp_KeysetInfoData_TechContacts(const Epp::KeysetInfoData::TechContacts &_src, ccReg::TechContact &_dst);

    void wrap_Epp_KeySet_Localized_InfoData(const Epp::KeySet::Localized::InfoData &_src, ccReg::KeySet &_dst);

    /**
     * @returns data ordered the same way as input nsset_handles
     */
    ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& nsset_handles,
        const std::map<std::string, boost::optional<Epp::LocalizedNssetHandleRegistrationObstruction> >& nsset_handle_check_results
    );

    /**
     * length of domain registration period
     */
    Epp::DomainRegistrationTime unwrap_domain_registration_period(const ccReg::Period_str& period);

    /**
     * domain administrative contacts unwrapper
     */
    std::vector<std::string> unwrap_ccreg_admincontacts_to_vector_string(const ccReg::AdminContact & in);

    /**
     * ENUM validation list unwrapper
     */
    std::vector<Epp::ENUMValidationExtension> unwrap_enum_validation_extension(const ccReg::ExtensionList& ext);
}

#endif