#include "src/corba/epp/corba_conversions.h"

#include "src/corba/EPP.hh"
#include "src/epp/error.h"
#include "src/epp/contact/ident_type.h"
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

    static Nullable<Epp::IdentType::Enum> unwrap_ident_type(ccReg::identtyp ssntype) {
        switch(ssntype) {
            case ccReg::OP:         return Epp::IdentType::identity_card;
            case ccReg::PASS:       return Epp::IdentType::passport;
            case ccReg::ICO:        return Epp::IdentType::organization_identification;
            case ccReg::MPSV:       return Epp::IdentType::social_security_number;
            case ccReg::BIRTHDAY:   return Epp::IdentType::birthday;
            default:                return Nullable<Epp::IdentType::Enum>();
        };

        return Nullable<Epp::IdentType::Enum>();
    }

    std::vector<std::string> unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles) {
        std::vector<std::string> result;

        result.reserve(handles.length());

        for(CORBA::ULong i = 0; i < handles.length(); ++i) {
            result.push_back(Corba::unwrap_string_from_const_char_ptr(handles[i]));
        }

        return result;
    }

    Epp::RequestParams unwrap_epp_request_params(const ccReg::EppParams& _epp_request_params) {

        return Epp::RequestParams(
            CorbaConversion::int_to_int<unsigned long long>(_epp_request_params.loginID),
            Corba::unwrap_string(_epp_request_params.clTRID),
            _epp_request_params.requestID == 0
                ?   Optional<unsigned long long>()
                :   Optional<unsigned long long>(CorbaConversion::int_to_int<unsigned long long>(_epp_request_params.requestID))
        );
    }

    struct ExceptionInvalidIdentType {};

    /**
     * @throws ExceptionInvalidIdentType
     */

    static ccReg::identtyp wrap_ident_type(Nullable<Epp::IdentType::Enum> ident) {
        if(ident.isnull()) {
            return ccReg::EMPTY;
        }

        switch(ident.get_value()) {
            case Epp::IdentType::identity_card:                 return ccReg::OP;
            case Epp::IdentType::passport:                      return ccReg::PASS;
            case Epp::IdentType::organization_identification:   return ccReg::ICO;
            case Epp::IdentType::social_security_number:        return ccReg::MPSV;
            case Epp::IdentType::birthday:                      return ccReg::BIRTHDAY;
            default:                                            throw ExceptionInvalidIdentType();
        };

        throw ExceptionInvalidIdentType();
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

    struct StreetAddressPart {
        std::string street_line_1;
        std::string street_line_2;
        std::string street_line_3;

        StreetAddressPart(
            const std::string& _street_line_1,
            const std::string& _street_line_2,
            const std::string& _street_line_3
        ) :
            street_line_1(_street_line_1),
            street_line_2(_street_line_2),
            street_line_3(_street_line_3)
        { }
    };

    static StreetAddressPart unwrap_streets(const ccReg::Lists& _corba_streets) {
        return StreetAddressPart(
            (_corba_streets.length() > 0
                ?   _corba_streets[0].in() != NULL ? Corba::unwrap_string(_corba_streets[0].in()) : ""
                :   ""
            ),
            (_corba_streets.length() > 1
                ?   _corba_streets[1].in() != NULL ? Corba::unwrap_string(_corba_streets[1].in()) : ""
                :   ""
            ),
            (_corba_streets.length() > 2
                ?   _corba_streets[2].in() != NULL ? Corba::unwrap_string(_corba_streets[2].in()) : ""
                :   ""
            )
        );
    }

    namespace {

    void set_disclosed_items(const ccReg::ContactChange &data, std::set< Epp::ContactDisclose::Enum > &items)
    {
        items.clear();
        if (CorbaConversion::int_to_int< bool >(data.DiscloseName)) {
            items.insert(Epp::ContactDisclose::name);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseOrganization)) {
            items.insert(Epp::ContactDisclose::organization);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseAddress)) {
            items.insert(Epp::ContactDisclose::address);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseTelephone)) {
            items.insert(Epp::ContactDisclose::telephone);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseFax)) {
            items.insert(Epp::ContactDisclose::fax);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseEmail)) {
            items.insert(Epp::ContactDisclose::email);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseVAT)) {
            items.insert(Epp::ContactDisclose::vat);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseIdent)) {
            items.insert(Epp::ContactDisclose::ident);
        }
        if (CorbaConversion::int_to_int< bool >(data.DiscloseNotifyEmail)) {
            items.insert(Epp::ContactDisclose::notify_email);
        }
        if (items.empty()) {
            throw std::runtime_error("Empty set is disallowed.");
        }
    }

    bool is_set_at_least_one_flag(const ccReg::ContactChange &data)
    {
        return CorbaConversion::int_to_int< bool >(data.DiscloseName) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseOrganization) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseAddress) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseTelephone) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseFax) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseEmail) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseVAT) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseIdent) ||
               CorbaConversion::int_to_int< bool >(data.DiscloseNotifyEmail);
    }

    void set_to_hide_and_to_disclose_items(
        const ccReg::ContactChange &data,
        std::set< Epp::ContactDisclose::Enum > &to_hide,
        std::set< Epp::ContactDisclose::Enum > &to_disclose)
    {
        switch (data.DiscloseFlag)
        {
            //element <contact:disclose flag="0">
            case ccReg::DISCL_HIDE:
                set_disclosed_items(data, to_hide);
                to_disclose.clear();
                return;
            //element <contact:disclose flag="1">
            case ccReg::DISCL_DISPLAY:
                to_hide.clear();
                set_disclosed_items(data, to_disclose);
                return;
            //missing element <contact:disclose>
            case ccReg::DISCL_EMPTY:
                if (is_set_at_least_one_flag(data)) {
                    throw std::runtime_error("Not a single one disclose flag has to be set.");
                }
                to_hide.clear();
                to_disclose.clear();
                return;
        }
        throw std::runtime_error("Invalid DiscloseFlag value");
    }

    bool is_contact_change_string_meaning_to_delete(const char *value)
    {
        return value[0] = '\b';
    }

    bool is_contact_change_string_meaning_not_to_touch(const char *value)
    {
        return value[0] = '\0';
    }

    boost::optional< Nullable< std::string > > convert_contact_change_string(const char *src)
    {
        const bool src_has_special_meaning_to_delete = is_contact_change_string_meaning_to_delete(src);
        if (src_has_special_meaning_to_delete) {
            return Nullable< std::string >();
        }
        const bool src_has_special_meaning_not_to_touch = is_contact_change_string_meaning_not_to_touch(src);
        if (src_has_special_meaning_not_to_touch) {
            return boost::optional< Nullable< std::string > >();
        }
        return Nullable< std::string >(boost::trim_copy(Corba::unwrap_string(src)));
    }

    Epp::ContactDisclose convert_ContactChange_to_ContactDisclose(
            const ccReg::ContactChange &src,
            Epp::ContactDisclose::Flag::Enum meaning)
    {
        Epp::ContactDisclose result(meaning);
        if (CorbaConversion::int_to_int< bool >(src.DiscloseName)) {
            result.add(Epp::ContactDisclose::name);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseOrganization)) {
            result.add(Epp::ContactDisclose::organization);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseAddress)) {
            result.add(Epp::ContactDisclose::address);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseTelephone)) {
            result.add(Epp::ContactDisclose::telephone);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseFax)) {
            result.add(Epp::ContactDisclose::fax);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseEmail)) {
            result.add(Epp::ContactDisclose::email);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseVAT)) {
            result.add(Epp::ContactDisclose::vat);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseIdent)) {
            result.add(Epp::ContactDisclose::ident);
        }
        if (CorbaConversion::int_to_int< bool >(src.DiscloseNotifyEmail)) {
            result.add(Epp::ContactDisclose::notify_email);
        }
    }

    boost::optional< Epp::ContactDisclose > unwrap_ContactChange_to_optional_ContactDisclose(
            const ccReg::ContactChange &src)
    {
        switch (src.DiscloseFlag)
        {
            //element <contact:disclose flag="0">
            case ccReg::DISCL_HIDE:
                return convert_ContactChange_to_ContactDisclose(src, Epp::ContactDisclose::Flag::to_hide);
            //element <contact:disclose flag="1">
            case ccReg::DISCL_DISPLAY:
                return convert_ContactChange_to_ContactDisclose(src, Epp::ContactDisclose::Flag::to_disclose);
            //missing element <contact:disclose>
            case ccReg::DISCL_EMPTY:
                return boost::optional< Epp::ContactDisclose >();
        }
        throw std::runtime_error("Invalid DiscloseFlag value");
    }

    }//namespace Corba::{anonymous}

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactCreateInputData &dst)
    {
        const StreetAddressPart streets = Corba::unwrap_streets(src.Streets);
        dst.name              = boost::trim_copy(Corba::unwrap_string(src.Name));
        dst.organization      = boost::trim_copy(Corba::unwrap_string(src.Organization));
        dst.street1           = boost::trim_copy(streets.street_line_1);
        dst.street2           = boost::trim_copy(streets.street_line_2);
        dst.street3           = boost::trim_copy(streets.street_line_3);
        dst.city              = boost::trim_copy(Corba::unwrap_string(src.City));
        dst.state_or_province = boost::trim_copy(Corba::unwrap_string(src.StateOrProvince));
        dst.postal_code       = boost::trim_copy(Corba::unwrap_string(src.PostalCode));
        dst.country_code      = boost::trim_copy(Corba::unwrap_string(src.CC));
        dst.telephone         = boost::trim_copy(Corba::unwrap_string(src.Telephone));
        dst.fax               = boost::trim_copy(Corba::unwrap_string(src.Fax));
        dst.email             = boost::trim_copy(Corba::unwrap_string(src.Email));
        dst.notify_email      = boost::trim_copy(Corba::unwrap_string(src.NotifyEmail));
        dst.VAT               = boost::trim_copy(Corba::unwrap_string(src.VAT));
        dst.ident             = boost::trim_copy(Corba::unwrap_string(src.ident));
        dst.identtype         = Corba::unwrap_ident_type(src.identtype);
        dst.authinfo          = boost::trim_copy(Corba::unwrap_string(src.AuthInfoPw));
        set_to_hide_and_to_disclose_items(src, dst.to_hide, dst.to_disclose);
    }

    static Optional<std::string> convert_corba_string_change(const char* input) {
        const std::string safer_input = Corba::unwrap_string(input);

        /* XXX Defined by convention. Could probably be substituted by more explicit means in IDL interface. */
        const char char_for_value_deleting = '\b';

        return
            safer_input.empty()
            ?   Optional<std::string>()
            :   safer_input.at(0) == char_for_value_deleting
                    ?   ""
                    :   boost::trim_copy(Corba::unwrap_string(input));
    }

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactUpdateInputData &dst)
    {
        dst.name              = convert_corba_string_change(src.Name);
        dst.organization      = convert_corba_string_change(src.Organization);
        dst.street1           = convert_corba_string_change(0 < src.Streets.length() ? src.Streets[0] : "");
        dst.street2           = convert_corba_string_change(1 < src.Streets.length() ? src.Streets[1] : "");
        dst.street3           = convert_corba_string_change(2 < src.Streets.length() ? src.Streets[2] : "");
        dst.city              = convert_corba_string_change(src.City);
        dst.state_or_province = convert_corba_string_change(src.StateOrProvince);
        dst.postal_code       = convert_corba_string_change(src.PostalCode);
        dst.country_code      = convert_corba_string_change(src.CC);
        dst.telephone         = convert_corba_string_change(src.Telephone);
        dst.fax               = convert_corba_string_change(src.Fax);
        dst.email             = convert_corba_string_change(src.Email);
        dst.notify_email      = convert_corba_string_change(src.NotifyEmail);
        dst.VAT               = convert_corba_string_change(src.VAT);
        dst.ident             = convert_corba_string_change(src.ident);
        dst.identtype         = Corba::unwrap_ident_type(src.identtype);
        dst.authinfo          = convert_corba_string_change(src.AuthInfoPw);
        set_to_hide_and_to_disclose_items(src, dst.to_hide, dst.to_disclose);
    }

    void unwrap_ContactChange(const ccReg::ContactChange &src, Epp::ContactChange &dst)
    {
        dst.name              = convert_contact_change_string(src.Name);
        dst.organization      = convert_contact_change_string(src.Organization);
        for (unsigned idx = 0; idx < src.Streets.length(); ++idx) {
            dst.streets.push_back(convert_contact_change_string(src.Streets[idx]));
        }
        dst.city              = convert_contact_change_string(src.City);
        dst.state_or_province = convert_contact_change_string(src.StateOrProvince);
        dst.postal_code       = convert_contact_change_string(src.PostalCode);
        dst.country_code      = convert_contact_change_string(src.CC);
        dst.telephone         = convert_contact_change_string(src.Telephone);
        dst.fax               = convert_contact_change_string(src.Fax);
        dst.email             = convert_contact_change_string(src.Email);
        dst.notify_email      = convert_contact_change_string(src.NotifyEmail);
        dst.VAT               = convert_contact_change_string(src.VAT);
        dst.ident             = convert_contact_change_string(src.ident);
        dst.identtype         = Corba::unwrap_ident_type(src.identtype);
        dst.authinfo          = convert_contact_change_string(src.AuthInfoPw);
        unwrap_ContactChange(src, dst.disclose);
    }

    static std::string formatTime(const boost::posix_time::ptime& tm) {
        char buffer[100];
        convert_rfc3339_timestamp(buffer, sizeof(buffer), boost::posix_time::to_iso_extended_string(tm).c_str());
        return buffer;
    }

    
    namespace {

    template < Epp::ContactDisclose::Enum ITEM >
    CORBA::Boolean presents(const std::set< Epp::ContactDisclose::Enum > &src)
    {
        return CorbaConversion::int_to_int< CORBA::Boolean >(src.find(ITEM) != src.end());
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
        dst.identtype = Corba::wrap_ident_type(src.identtype);
        dst.AuthInfoPw = Corba::wrap_string_to_corba_string(src.auth_info_pw.get_value_or_default());

        const bool some_entry_to_hide = !src.to_hide.empty();
        const bool some_entry_to_disclose = !src.to_disclose.empty();
        if (some_entry_to_hide && some_entry_to_disclose) {
            throw std::runtime_error("Only hide or disclose can be set, not both.");
        }
        if (!some_entry_to_hide && !some_entry_to_disclose) {
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
            const std::set< Epp::ContactDisclose::Enum > &to_set = some_entry_to_hide ? src.to_hide
                                                                                      : src.to_disclose;
            dst.DiscloseFlag         = some_entry_to_hide ? ccReg::DISCL_HIDE : ccReg::DISCL_DISPLAY;
            dst.DiscloseName         = presents< Epp::ContactDisclose::name         >(to_set);
            dst.DiscloseOrganization = presents< Epp::ContactDisclose::organization >(to_set);
            dst.DiscloseAddress      = presents< Epp::ContactDisclose::address      >(to_set);
            dst.DiscloseTelephone    = presents< Epp::ContactDisclose::telephone    >(to_set);
            dst.DiscloseFax          = presents< Epp::ContactDisclose::fax          >(to_set);
            dst.DiscloseEmail        = presents< Epp::ContactDisclose::email        >(to_set);
            dst.DiscloseVAT          = presents< Epp::ContactDisclose::vat          >(to_set);
            dst.DiscloseIdent        = presents< Epp::ContactDisclose::ident        >(to_set);
            dst.DiscloseNotifyEmail  = presents< Epp::ContactDisclose::notify_email >(to_set);
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

}//namespace Corba
