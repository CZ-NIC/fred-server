#include "src/corba/epp/corba_conversions.h"

#include "src/corba/EPP.hh"
#include "src/epp/error.h"
#include "src/epp/param.h"
#include "src/epp/contact/contact_create.h"

#include "src/corba/epp/epp_legacy_compatibility.h"
#include "src/corba/util/corba_conversions_string.h"

#include "util/corba_conversion.h"
#include "util/db/nullable.h"
#include "util/map_at.h"
#include "util/optional_value.h"
#include "src/old_utils/util.h" // for convert_rfc3339_timestamp()

#include <string>
#include <boost/algorithm/string/trim.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Corba {

    std::vector<std::string> unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles) {
        std::vector<std::string> result;

        result.reserve(handles.length());

        for(CORBA::ULong i = 0; i < handles.length(); ++i) {
            result.push_back(Corba::unwrap_string_from_const_char_ptr(handles[i]));
        }

        return result;
    }

    Epp::RequestParams unwrap_EppParams(const ccReg::EppParams &_epp_request_params) {
        Epp::RequestParams result;
        CorbaConversion::unwrap_int(_epp_request_params.loginID, result.session_id);
        result.client_transaction_id = Corba::unwrap_string(_epp_request_params.clTRID);
        unsigned long long log_request_id;
        CorbaConversion::unwrap_int(_epp_request_params.requestID, log_request_id);
        if (log_request_id != 0) {
            result.log_request_id = log_request_id;
        }
        return result;
    }

    struct ExceptionInvalidParam {};

    /**
     * @throws ExceptionInvalidParam
     */
    ccReg::ParamError wrap_param_error(Epp::Param::Enum _param) {

        switch(_param) {
            case Epp::Param::poll_msg_id:            return ccReg::poll_msgID;
            case Epp::Param::contact_handle:         return ccReg::contact_handle;
            case Epp::Param::contact_cc:             return ccReg::contact_cc;
            case Epp::Param::nsset_handle:           return ccReg::nsset_handle;
            case Epp::Param::nsset_tech:             return ccReg::nsset_tech;
            case Epp::Param::nsset_dns_name:         return ccReg::nsset_dns_name;
            case Epp::Param::nsset_dns_addr:         return ccReg::nsset_dns_addr;
            case Epp::Param::nsset_dns_name_add:     return ccReg::nsset_dns_name_add;
            case Epp::Param::nsset_dns_name_rem:     return ccReg::nsset_dns_name_rem;
            case Epp::Param::nsset_tech_add:         return ccReg::nsset_tech_add;
            case Epp::Param::nsset_tech_rem:         return ccReg::nsset_tech_rem;
            case Epp::Param::domain_fqdn:            return ccReg::domain_fqdn;
            case Epp::Param::domain_registrant:      return ccReg::domain_registrant;
            case Epp::Param::domain_nsset:           return ccReg::domain_nsset;
            case Epp::Param::domain_keyset:          return ccReg::domain_keyset;
            case Epp::Param::domain_period:          return ccReg::domain_period;
            case Epp::Param::domain_admin:           return ccReg::domain_admin;
            case Epp::Param::domain_tmpcontact:      return ccReg::domain_tmpcontact;
            case Epp::Param::domain_ext_val_date:    return ccReg::domain_ext_valDate;
            case Epp::Param::domain_ext_val_date_missing: return ccReg::domain_ext_valDate_missing;
            case Epp::Param::domain_cur_exp_date:    return ccReg::domain_curExpDate;
            case Epp::Param::domain_admin_add:       return ccReg::domain_admin_add;
            case Epp::Param::domain_admin_rem:       return ccReg::domain_admin_rem;
            case Epp::Param::keyset_handle:          return ccReg::keyset_handle;
            case Epp::Param::keyset_tech:            return ccReg::keyset_tech;
            case Epp::Param::keyset_dsrecord:        return ccReg::keyset_dsrecord;
            case Epp::Param::keyset_dsrecord_add:    return ccReg::keyset_dsrecord_add;
            case Epp::Param::keyset_dsrecord_rem:    return ccReg::keyset_dsrecord_rem;
            case Epp::Param::keyset_tech_add:        return ccReg::keyset_tech_add;
            case Epp::Param::keyset_tech_rem:        return ccReg::keyset_tech_rem;
            case Epp::Param::registrar_autor:        return ccReg::registrar_autor;
            case Epp::Param::keyset_dnskey:          return ccReg::keyset_dnskey;
            case Epp::Param::keyset_dnskey_add:      return ccReg::keyset_dnskey_add;
            case Epp::Param::keyset_dnskey_rem:      return ccReg::keyset_dnskey_rem;
        };

        throw ExceptionInvalidParam();
    }

    ccReg::Response wrap_response(const Epp::LocalizedSuccessResponse& _input, const std::string& _server_transaction_handle) {
        ccReg::Response result;

        CorbaConversion::int_to_int(to_description_db_id(_input.response), result.code);
        result.svTRID = Corba::wrap_string_to_corba_string(_server_transaction_handle);
        result.msg = Corba::wrap_string_to_corba_string(_input.localized_msg);

        return result;
    }

    ccReg::EPP::EppError wrap_error(const Epp::LocalizedFailResponse& _input, const std::string& _server_transaction_handle) {
        ccReg::EPP::EppError result;

        CorbaConversion::int_to_int(to_description_db_id(_input.response), result.errCode);
        result.svTRID = Corba::wrap_string_to_corba_string(_server_transaction_handle);
        result.errMsg = Corba::wrap_string_to_corba_string(_input.localized_msg);

        const std::set<Epp::Error>::size_type size = _input.errors.size();
        result.errorList.length(size);

        int i = 0;
        for(std::set<Epp::LocalizedError>::const_iterator it = _input.errors.begin();
            it != _input.errors.end();
            ++it, ++i
        ) {
            result.errorList[i].code = Corba::wrap_param_error(it->param);
            CorbaConversion::int_to_int(it->position, result.errorList[i].position);
            result.errorList[i].reason = Corba::wrap_string_to_corba_string(it->localized_reason_description);
        }

        return result;
    }

    namespace {

    bool is_contact_change_string_meaning_to_delete(const char *value)
    {
        return value[0] == '\b';
    }

    bool is_contact_change_string_meaning_not_to_touch(const char *value)
    {
        return value[0] == '\0';
    }

    boost::optional< Nullable< std::string > > convert_contact_update_or_delete_string(const char *src)
    {
        const bool src_has_special_meaning_to_delete = is_contact_change_string_meaning_to_delete(src);
        if (src_has_special_meaning_to_delete) {
            return Nullable< std::string >();
        }
        const bool src_has_special_meaning_not_to_touch = is_contact_change_string_meaning_not_to_touch(src);
        if (src_has_special_meaning_not_to_touch) {
            return boost::optional< Nullable< std::string > >();
        }
        const std::string value_to_set = boost::trim_copy(Corba::unwrap_string(src));
        const bool value_to_set_means_not_to_touch = value_to_set.empty();
        if (value_to_set_means_not_to_touch) {
            return boost::optional< Nullable< std::string > >();
        }
        return Nullable< std::string >(value_to_set);
    }

    boost::optional< std::string > convert_contact_update_string(const char *src)
    {
        const bool src_has_special_meaning_not_to_touch = is_contact_change_string_meaning_not_to_touch(src);
        if (src_has_special_meaning_not_to_touch) {
            return boost::optional< std::string >();
        }
        const std::string value_to_set = boost::trim_copy(Corba::unwrap_string(src));
        const bool value_to_set_means_not_to_touch = value_to_set.empty();
        return value_to_set_means_not_to_touch ? boost::optional< std::string >()
                                               : value_to_set;
    }

    Epp::ContactDisclose convert_ContactChange_to_ContactDisclose(
            const ccReg::ContactChange &src,
            Epp::ContactDisclose::Flag::Enum meaning)
    {
        Epp::ContactDisclose result(meaning);
        if (CorbaConversion::int_to_int< bool >(src.DiscloseName)) {
            result.add< Epp::ContactDisclose::Item::name >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseOrganization)) {
            result.add< Epp::ContactDisclose::Item::organization >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseAddress)) {
            result.add< Epp::ContactDisclose::Item::address >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseTelephone)) {
            result.add< Epp::ContactDisclose::Item::telephone >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseFax)) {
            result.add< Epp::ContactDisclose::Item::fax >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseEmail)) {
            result.add< Epp::ContactDisclose::Item::email >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseVAT)) {
            result.add< Epp::ContactDisclose::Item::vat >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseIdent)) {
            result.add< Epp::ContactDisclose::Item::ident >();
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseNotifyEmail)) {
            result.add< Epp::ContactDisclose::Item::notify_email >();
        }
        return result;
    }

    boost::optional< Epp::ContactDisclose > unwrap_ContactChange_to_ContactDisclose(const ccReg::ContactChange &src)
    {
        switch (src.DiscloseFlag)
        {
            case ccReg::DISCL_EMPTY:
                return boost::optional< Epp::ContactDisclose >();
            case ccReg::DISCL_HIDE:
                return convert_ContactChange_to_ContactDisclose(src, Epp::ContactDisclose::Flag::hide);
            case ccReg::DISCL_DISPLAY:
                return convert_ContactChange_to_ContactDisclose(src, Epp::ContactDisclose::Flag::disclose);
        }
        throw std::runtime_error("Invalid DiscloseFlag value;");
    }

    Nullable< Epp::ContactChange::IdentType::Enum > unwrap_identtyp(ccReg::identtyp type)
    {
        switch (type)
        {
            case ccReg::EMPTY:    return Nullable< Epp::ContactChange::IdentType::Enum >();
            case ccReg::OP:       return Epp::ContactChange::IdentType::op;
            case ccReg::PASS:     return Epp::ContactChange::IdentType::pass;
            case ccReg::ICO:      return Epp::ContactChange::IdentType::ico;
            case ccReg::MPSV:     return Epp::ContactChange::IdentType::mpsv;
            case ccReg::BIRTHDAY: return Epp::ContactChange::IdentType::birthday;
        }
        throw std::runtime_error("Invalid identtyp value.");
    }

    }//namespace Corba::{anonymous}

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactChange &dst)
    {
        dst.name              = convert_contact_update_or_delete_string(src.Name);
        dst.organization      = convert_contact_update_or_delete_string(src.Organization);
        for (unsigned idx = 0; idx < src.Streets.length(); ++idx) {
            dst.streets.push_back(convert_contact_update_or_delete_string(src.Streets[idx]));
        }
        dst.city              = convert_contact_update_or_delete_string(src.City);
        dst.state_or_province = convert_contact_update_or_delete_string(src.StateOrProvince);
        dst.postal_code       = convert_contact_update_or_delete_string(src.PostalCode);
        dst.country_code      = convert_contact_update_string(src.CC);
        dst.telephone         = convert_contact_update_or_delete_string(src.Telephone);
        dst.fax               = convert_contact_update_or_delete_string(src.Fax);
        dst.email             = convert_contact_update_or_delete_string(src.Email);
        dst.notify_email      = convert_contact_update_or_delete_string(src.NotifyEmail);
        dst.vat               = convert_contact_update_or_delete_string(src.VAT);
        dst.ident             = convert_contact_update_or_delete_string(src.ident);
        dst.ident_type        = unwrap_identtyp(src.identtype);
        dst.auth_info_pw      = convert_contact_update_or_delete_string(src.AuthInfoPw);
        dst.disclose          = unwrap_ContactChange_to_ContactDisclose(src);
    }

    static std::string formatTime(const boost::posix_time::ptime& tm) {
        char buffer[100];
        convert_rfc3339_timestamp(buffer, sizeof(buffer), boost::posix_time::to_iso_extended_string(tm).c_str());
        return buffer;
    }

    
    namespace {

    template < Epp::ContactDisclose::Item::Enum ITEM >
    CORBA::Boolean presents(const Epp::ContactDisclose &src)
    {
        return CorbaConversion::int_to_int< CORBA::Boolean >(src.presents< ITEM >());
    }

    ccReg::identtyp wrap_identtyp(const Nullable< Epp::LocalizedContactInfoOutputData::IdentType::Enum > &type)
    {
        if (type.isnull()) {
            return ccReg::EMPTY;
        }
        switch (type.get_value())
        {
            case Epp::LocalizedContactInfoOutputData::IdentType::op:       return ccReg::OP;
            case Epp::LocalizedContactInfoOutputData::IdentType::pass:     return ccReg::PASS;
            case Epp::LocalizedContactInfoOutputData::IdentType::ico:      return ccReg::ICO;
            case Epp::LocalizedContactInfoOutputData::IdentType::mpsv:     return ccReg::MPSV;
            case Epp::LocalizedContactInfoOutputData::IdentType::birthday: return ccReg::BIRTHDAY;
        }
        throw std::runtime_error("Invalid Epp::LocalizedContactInfoOutputData::IdentType::Enum value.");
    }

    }//namespace Corba::{anonymous}

    void wrap_LocalizedContactInfoOutputData(const Epp::LocalizedContactInfoOutputData &src, ccReg::Contact &dst)
    {
        dst.handle = wrap_string_to_corba_string(src.handle);
        dst.ROID = wrap_string_to_corba_string(src.roid);
        dst.ClID = wrap_string_to_corba_string(src.sponsoring_registrar_handle);
        dst.CrID = wrap_string_to_corba_string(src.creating_registrar_handle);
        // XXX IDL nonsense
        dst.UpID = wrap_string_to_corba_string(src.last_update_registrar_handle.get_value_or(std::string()));

        dst.stat.length(src.localized_external_states.size());
        unsigned long idx = 0;
        for (std::map< std::string, std::string >::const_iterator value_text_ptr = src.localized_external_states.begin();
            value_text_ptr != src.localized_external_states.end(); ++value_text_ptr, ++idx)
        {
            dst.stat[idx].value = wrap_string_to_corba_string(value_text_ptr->first);
            dst.stat[idx].text  = wrap_string_to_corba_string(value_text_ptr->second);
        }

        dst.CrDate = wrap_string_to_corba_string(formatTime(src.crdate));
        // XXX IDL nonsense
        dst.UpDate = wrap_string_to_corba_string(
            src.last_update.isnull() ? std::string()
                                     : formatTime(src.last_update.get_value()));
        // XXX IDL nonsense
        dst.TrDate = wrap_string_to_corba_string(
            src.last_transfer.isnull() ? std::string()
                                       : formatTime(src.last_transfer.get_value()));
        dst.Name = wrap_string_to_corba_string(src.name.get_value_or_default());
        dst.Organization = wrap_string_to_corba_string(src.organization.get_value_or_default());

        const unsigned number_of_streets =
            !src.street3.isnull() && !src.street3.get_value().empty()
                ? 3
                : !src.street2.isnull() && !src.street2.get_value().empty()
                    ? 2
                    : !src.street1.isnull() && !src.street1.get_value().empty()
                        ? 1
                        : 0;
        dst.Streets.length(number_of_streets);
        if (0 < number_of_streets) {
            dst.Streets[0] = Corba::wrap_string_to_corba_string(src.street1.get_value_or_default());
        }
        if (1 < number_of_streets) {
            dst.Streets[1] = Corba::wrap_string_to_corba_string(src.street2.get_value_or_default());
        }
        if (2 < number_of_streets) {
            dst.Streets[2] = Corba::wrap_string_to_corba_string(src.street3.get_value_or_default());
        }

        dst.City = Corba::wrap_string_to_corba_string(src.city.get_value_or_default());
        dst.StateOrProvince = Corba::wrap_string_to_corba_string(src.state_or_province.get_value_or_default());
        dst.PostalCode = Corba::wrap_string_to_corba_string(src.postal_code.get_value_or_default());
        dst.CountryCode = Corba::wrap_string_to_corba_string(src.country_code.get_value_or_default());
        dst.Telephone = Corba::wrap_string_to_corba_string(src.telephone.get_value_or_default());
        dst.Fax = Corba::wrap_string_to_corba_string(src.fax.get_value_or_default());
        dst.Email = Corba::wrap_string_to_corba_string(src.email.get_value_or_default());
        dst.NotifyEmail = Corba::wrap_string_to_corba_string(src.notify_email.get_value_or_default());
        dst.VAT = Corba::wrap_string_to_corba_string(src.VAT.get_value_or_default());
        dst.ident = Corba::wrap_string_to_corba_string(src.ident.get_value_or_default());
        dst.identtype = Corba::wrap_identtyp(src.identtype);
        dst.AuthInfoPw = Corba::wrap_string_to_corba_string(src.auth_info_pw.get_value_or_default());

        if (!src.disclose.is_initialized()) {
            dst.DiscloseFlag           = ccReg::DISCL_EMPTY;
            dst.DiscloseName           = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseOrganization   = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseAddress        = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseTelephone      = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseFax            = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseEmail          = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseVAT            = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseIdent          = CorbaConversion::int_to_int< CORBA::Boolean >(false);
            dst.DiscloseNotifyEmail    = CorbaConversion::int_to_int< CORBA::Boolean >(false);
        }
        else {
            dst.DiscloseFlag         = src.disclose->does_present_item_mean_to_disclose() ? ccReg::DISCL_DISPLAY
                                                                                          : ccReg::DISCL_HIDE;
            dst.DiscloseName         = presents< Epp::ContactDisclose::Item::name         >(*src.disclose);
            dst.DiscloseOrganization = presents< Epp::ContactDisclose::Item::organization >(*src.disclose);
            dst.DiscloseAddress      = presents< Epp::ContactDisclose::Item::address      >(*src.disclose);
            dst.DiscloseTelephone    = presents< Epp::ContactDisclose::Item::telephone    >(*src.disclose);
            dst.DiscloseFax          = presents< Epp::ContactDisclose::Item::fax          >(*src.disclose);
            dst.DiscloseEmail        = presents< Epp::ContactDisclose::Item::email        >(*src.disclose);
            dst.DiscloseVAT          = presents< Epp::ContactDisclose::Item::vat          >(*src.disclose);
            dst.DiscloseIdent        = presents< Epp::ContactDisclose::Item::ident        >(*src.disclose);
            dst.DiscloseNotifyEmail  = presents< Epp::ContactDisclose::Item::notify_email >(*src.disclose);
        }
    }

    static ccReg::CheckAvail wrap_contact_handle_check_result(const boost::optional< Epp::LocalizedContactHandleRegistrationObstruction >& _obstruction) {

        if (!_obstruction.is_initialized()) {
            return ccReg::NotExist;
        }

        switch (_obstruction.get().state) {
            case Epp::ContactHandleRegistrationObstruction::invalid_handle      : return ccReg::BadFormat;
            case Epp::ContactHandleRegistrationObstruction::protected_handle    : return ccReg::DelPeriod; // XXX oh my
            case Epp::ContactHandleRegistrationObstruction::registered_handle   : return ccReg::Exist;
        }

        throw std::runtime_error("unknown_contact_state");
    }

    /**
     * @returns check results in the same order as input handles
     */
    ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map< std::string, boost::optional< Epp::LocalizedContactHandleRegistrationObstruction > >& contact_handle_check_results
    ) {
        ccReg::CheckResp result;
        result.length(contact_handles.size());

        CORBA::ULong i = 0;
        for(
            std::vector<std::string>::const_iterator it = contact_handles.begin();
            it != contact_handles.end();
            ++it, ++i
        ) {
            const boost::optional< Epp::LocalizedContactHandleRegistrationObstruction > check_result = map_at(contact_handle_check_results, *it);

            result[i].avail = wrap_contact_handle_check_result(check_result);
            result[i].reason = Corba::wrap_string_to_corba_string(check_result.is_initialized() ? check_result.get().description : "");
        }

        return result;
    }

    void wrap_Epp_LocalizedStates(const Epp::LocalizedStates &_src, ccReg::Status &_dst)
    {
        if (_src.descriptions.empty()) {
            _dst.length(1);
            _dst[0].value = "ok";
            _dst[0].text = _src.ok_state_description.c_str();
            return;
        }
        _dst.length(_src.descriptions.size());
        ::size_t idx = 0;
        for (Epp::LocalizedStates::Descriptions::const_iterator state_ptr = _src.descriptions.begin();
             state_ptr != _src.descriptions.end(); ++state_ptr, ++idx)
        {
            _dst[idx].value = Conversion::Enums::to_db_handle(state_ptr->first).c_str();
            _dst[idx].text = state_ptr->second.c_str();
        }
    }

    static void wrap_Epp_KeysetInfoData_DnsKeys(const Epp::KeysetInfoData::DnsKeys &_src, ccReg::DNSKey &_dst)
    {
        _dst.length(_src.size());
        ::size_t idx = 0;
        for (Epp::KeysetInfoData::DnsKeys::const_iterator data_ptr = _src.begin();
             data_ptr != _src.end(); ++data_ptr, ++idx)
        {
            CorbaConversion::wrap_int(data_ptr->get_flags(),    _dst[idx].flags);
            CorbaConversion::wrap_int(data_ptr->get_protocol(), _dst[idx].protocol);
            CorbaConversion::wrap_int(data_ptr->get_alg(),      _dst[idx].alg);
            _dst[idx].key = data_ptr->get_key().c_str();
        }
    }

    void wrap_Epp_KeysetInfoData_TechContacts(const Epp::KeysetInfoData::TechContacts &_src, ccReg::TechContact &_dst)
    {
        _dst.length(_src.size());
        ::size_t idx = 0;
        for (Epp::KeysetInfoData::TechContacts::const_iterator data_ptr = _src.begin();
             data_ptr != _src.end(); ++data_ptr, ++idx)
        {
            _dst[idx] = data_ptr->c_str();
        }
    }

    void wrap_Epp_LocalizedKeysetInfoData(const Epp::LocalizedKeysetInfoData &_src, ccReg::KeySet &_dst)
    {
        _dst.handle = _src.handle.c_str();
        _dst.ROID = _src.roid.c_str();
        _dst.ClID = _src.sponsoring_registrar_handle.c_str();
        _dst.CrID = _src.creating_registrar_handle.c_str();
        _dst.UpID = wrap_Nullable_string_to_string(_src.last_update_registrar_handle).c_str();
        wrap_Epp_LocalizedStates(_src.states, _dst.stat);
        _dst.CrDate = wrap_boost_posix_time_ptime_to_string(_src.crdate).c_str();
        _dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_update).c_str();
        _dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_transfer).c_str();
        _dst.AuthInfoPw = wrap_Nullable_string_to_string(_src.auth_info_pw).c_str();
        _dst.dsrec.length(0); // has to be empty
        wrap_Epp_KeysetInfoData_DnsKeys(_src.dns_keys, _dst.dnsk);
        wrap_Epp_KeysetInfoData_TechContacts(_src.tech_contacts, _dst.tech);
    }
}//namespace Corba
