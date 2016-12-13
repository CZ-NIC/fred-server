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
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/mpl/assert.hpp>
#include <boost/integer_traits.hpp>
#include <boost/optional.hpp>

namespace Corba {

    /**
     * integral types conversion with overflow detection to be replaced by CORBA wrappers
     */
    template < class SOURCE_INTEGRAL_TYPE, class TARGET_INTEGRAL_TYPE >
    void numeric_cast_by_ref(SOURCE_INTEGRAL_TYPE src, TARGET_INTEGRAL_TYPE &dst)
    {
        typedef boost::integer_traits< SOURCE_INTEGRAL_TYPE > source_integral_type_traits;
        typedef boost::integer_traits< TARGET_INTEGRAL_TYPE > target_integral_type_traits;

        BOOST_MPL_ASSERT_MSG(source_integral_type_traits::is_integral, source_type_have_to_be_integral, (SOURCE_INTEGRAL_TYPE));
        BOOST_MPL_ASSERT_MSG(target_integral_type_traits::is_integral, target_type_have_to_be_integral, (TARGET_INTEGRAL_TYPE));
        dst = boost::numeric_cast< TARGET_INTEGRAL_TYPE >(src);
    }


    std::vector<std::string> unwrap_handle_sequence_to_string_vector(const ccReg::Check& handles) {
        std::vector<std::string> result;

        result.reserve(handles.length());

        for(CORBA::ULong i = 0; i < handles.length(); ++i) {
            result.push_back(unwrap_string_from_const_char_ptr(handles[i]) );
        }

        return result;
    }

    std::vector< std::string > unwrap_TechContact_to_vector_string(const ccReg::TechContact &_tech_contacts)
    {
        std::vector< std::string > result;
        result.reserve(_tech_contacts.length());

        for(CORBA::ULong idx = 0; idx < _tech_contacts.length(); ++idx) {
            result.push_back(unwrap_string_from_const_char_ptr(_tech_contacts[idx]));
        }
        return result;
    }

    std::vector< Epp::KeySet::DsRecord > unwrap_ccReg_DSRecord_to_vector_Epp_KeySet_DsRecord(
        const ccReg::DSRecord &_ds_records)
    {
        std::vector< Epp::KeySet::DsRecord > result;
        result.reserve(_ds_records.length());

        for(CORBA::ULong idx = 0; idx < _ds_records.length(); ++idx) {
            Epp::KeySet::DsRecord ds_record;
            unwrap_ccReg_DSRecord_str(_ds_records[idx], ds_record);
            result.push_back(ds_record);
        }
        return result;
    }

    void unwrap_ccReg_DSRecord_str(const ccReg::DSRecord_str &_src, Epp::KeySet::DsRecord &_dst)
    {
        long key_tag;
        CorbaConversion::unwrap_int(_src.keyTag, key_tag);
        long alg;
        CorbaConversion::unwrap_int(_src.alg, alg);
        long digest_type;
        CorbaConversion::unwrap_int(_src.digestType, digest_type);
        const std::string digest = unwrap_string(_src.digest);
        long max_sig_life;
        CorbaConversion::unwrap_int(_src.maxSigLife, max_sig_life);
        _dst = Epp::KeySet::DsRecord(key_tag, alg, digest_type, digest, max_sig_life);
    }

    std::vector< Epp::KeySet::DnsKey > unwrap_ccReg_DNSKey_to_vector_Epp_KeySet_DnsKey(
        const ccReg::DNSKey &_dns_keys)
    {
        std::vector< Epp::KeySet::DnsKey > result;
        result.reserve(_dns_keys.length());

        for(CORBA::ULong idx = 0; idx < _dns_keys.length(); ++idx) {
            Epp::KeySet::DnsKey dns_key;
            unwrap_ccReg_DNSKey_str(_dns_keys[idx], dns_key);
            result.push_back(dns_key);
        }
        return result;
    }

    void unwrap_ccReg_DNSKey_str(const ccReg::DNSKey_str &_src, Epp::KeySet::DnsKey &_dst)
    {
        unsigned short flags;
        CorbaConversion::unwrap_int(_src.flags, flags);
        unsigned short protocol;
        CorbaConversion::unwrap_int(_src.protocol, protocol);
        unsigned short alg;
        CorbaConversion::unwrap_int(_src.alg, alg);
        const std::string key = unwrap_string(_src.key);
        _dst = Epp::KeySet::DnsKey(flags, protocol, alg, key);
    }

    Epp::RequestParams unwrap_EppParams(const ccReg::EppParams &_epp_request_params)
    {
        Epp::RequestParams result;
        CorbaConversion::unwrap_int(_epp_request_params.loginID, result.session_id);
        result.client_transaction_id = unwrap_string(_epp_request_params.clTRID);
        unsigned long long log_request_id;
        CorbaConversion::unwrap_int(_epp_request_params.requestID, log_request_id);
        if (log_request_id != 0) {
            result.log_request_id = log_request_id;
        }
        return result;
    }

    boost::optional<short> unwrap_tech_check_level(CORBA::Short level)
    {
        return level < 0
            ? boost::optional<short>()
            : boost::optional<short>(boost::numeric_cast<short>(level));
    }

    struct ExceptionInvalidIdentType {};

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

    ccReg::Response wrap_response(const Epp::LocalizedSuccessResponse& _input, const std::string& _server_transaction_handle)
    {
        ccReg::Response result;
        wrap_Epp_LocalizedSuccessResponse(_input, _server_transaction_handle, result);
        return result;
    }

    void wrap_Epp_LocalizedSuccessResponse(const Epp::LocalizedSuccessResponse &_src,
                                           const std::string &_server_transaction_handle,
                                           ccReg::Response &_dst)
    {
        CorbaConversion::wrap_int(Epp::to_description_db_id(_src.response), _dst.code);
        _dst.svTRID = _server_transaction_handle.c_str();
        _dst.msg = _src.localized_msg.c_str();
    }

    ccReg::EPP::EppError wrap_error(const Epp::LocalizedFailResponse& _input, const std::string& _server_transaction_handle) {
        ccReg::EPP::EppError result;

        CorbaConversion::wrap_int(Epp::to_description_db_id(_input.response), result.errCode);
        result.svTRID = wrap_string_to_corba_string(_server_transaction_handle);
        result.errMsg = wrap_string_to_corba_string(_input.localized_msg);

        const std::set<Epp::Error>::size_type size = _input.errors.size();
        result.errorList.length(size);

        int i = 0;
        for(std::set<Epp::LocalizedError>::const_iterator it = _input.errors.begin();
            it != _input.errors.end();
            ++it, ++i
        ) {
            result.errorList[i].code = wrap_param_error(it->param);
            CorbaConversion::wrap_int(it->position, result.errorList[i].position);
            result.errorList[i].reason = wrap_string_to_corba_string(it->localized_reason_description);
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

    template < class TARGET_INTEGRAL_TYPE, class SOURCE_INTEGRAL_TYPE >
    TARGET_INTEGRAL_TYPE wrap_int(SOURCE_INTEGRAL_TYPE src)
    {
        TARGET_INTEGRAL_TYPE dst;
        CorbaConversion::wrap_int(src, dst);
        return dst;
    }

    Epp::ContactDisclose convert_ContactChange_to_ContactDisclose(
            const ccReg::ContactChange &src,
            Epp::ContactDisclose::Flag::Enum meaning)
    {
        Epp::ContactDisclose result(meaning);
        if (wrap_int< bool >(src.DiscloseName)) {
            result.add< Epp::ContactDisclose::Item::name >();
        }
        if (wrap_int< bool >(src.DiscloseOrganization)) {
            result.add< Epp::ContactDisclose::Item::organization >();
        }
        if (wrap_int< bool >(src.DiscloseAddress)) {
            result.add< Epp::ContactDisclose::Item::address >();
        }
        if (wrap_int< bool >(src.DiscloseTelephone)) {
            result.add< Epp::ContactDisclose::Item::telephone >();
        }
        if (wrap_int< bool >(src.DiscloseFax)) {
            result.add< Epp::ContactDisclose::Item::fax >();
        }
        if (wrap_int< bool >(src.DiscloseEmail)) {
            result.add< Epp::ContactDisclose::Item::email >();
        }
        if (wrap_int< bool >(src.DiscloseVAT)) {
            result.add< Epp::ContactDisclose::Item::vat >();
        }
        if (wrap_int< bool >(src.DiscloseIdent)) {
            result.add< Epp::ContactDisclose::Item::ident >();
        }
        if (wrap_int< bool >(src.DiscloseNotifyEmail)) {
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

    Optional<std::string> unwrap_string_for_change_to_Optional_string(const char* _src) {
        const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

        return
            unwrapped_src.empty()
            ? Optional<std::string>() // do not set
            : boost::trim_copy(unwrapped_src);
    }

    Optional<std::string> unwrap_string_for_change_or_remove_to_Optional_string(const char* _src) {
        const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

        /* Defined by convention. Could be substituted by more explicit means in IDL interface
         * (like using _add and _rem elements, not just _chg for all operations). */
        static const char char_for_value_deleting = '\b';

        return
            unwrapped_src.empty()
            ? Optional<std::string>() // do not set
            : unwrapped_src.at(0) == char_for_value_deleting
                ? std::string() // set empty
                : boost::trim_copy(unwrapped_src);
    }

    Optional<Nullable<std::string> > unwrap_string_for_change_or_remove_to_Optional_Nullable_string(const char* _src) {
        const std::string unwrapped_src = unwrap_string_from_const_char_ptr(_src);

        /* Defined by convention. Could be substituted by more explicit means in IDL interface
         * (like using _add and _rem elements, not just _chg for all operations). */
        static const char char_for_value_deleting = '\b';

        return
            unwrapped_src.empty()
            ? Optional<Nullable<std::string> >() // do not set
            : unwrapped_src.at(0) == char_for_value_deleting
                ? Optional<Nullable<std::string> >(Nullable<std::string>()) // set NULL
                : boost::trim_copy(unwrapped_src);
    }

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

    namespace {

    template < Epp::ContactDisclose::Item::Enum ITEM >
    CORBA::Boolean presents(const Epp::ContactDisclose &src)
    {
        return wrap_int< CORBA::Boolean >(src.presents< ITEM >());
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

    CORBA::String_var wrap_boost_posix_time_ptime_to_string(const boost::posix_time::ptime &_src)
    {
        static const unsigned size_enough_for_string_representation_of_time = 100;//2016-04-18T13:00:00+02:00
        char time[size_enough_for_string_representation_of_time];
        const std::string iso_extended_time = boost::posix_time::to_iso_extended_string(_src);
        convert_rfc3339_timestamp(time, size_enough_for_string_representation_of_time, iso_extended_time.c_str());
        return const_cast< const char* >(time);
    }

    CORBA::String_var wrap_Nullable_boost_posix_time_ptime_to_string(const Nullable< boost::posix_time::ptime > &_src)
    {
        return _src.isnull() ? "" : wrap_boost_posix_time_ptime_to_string(_src.get_value());
    }

    CORBA::String_var wrap_Nullable_string_to_string(const Nullable< std::string > &src)
    {
        return src.isnull() ? "" : src.get_value().c_str();
    }

    }//namespace Corba::{anonymous}

    void wrap_LocalizedContactInfoOutputData(const Epp::LocalizedContactInfoOutputData &src, ccReg::Contact &dst)
    {
        dst.handle = wrap_string_to_corba_string(src.handle);
        dst.ROID = wrap_string_to_corba_string(src.roid);
        dst.ClID = wrap_string_to_corba_string(src.sponsoring_registrar_handle);
        dst.CrID = wrap_string_to_corba_string(src.creating_registrar_handle);
        // XXX IDL nonsense
        dst.UpID = wrap_Nullable_string_to_string(src.last_update_registrar_handle);

        dst.stat.length(src.localized_external_states.size());
        unsigned long idx = 0;
        for (std::map< std::string, std::string >::const_iterator value_text_ptr = src.localized_external_states.begin();
            value_text_ptr != src.localized_external_states.end(); ++value_text_ptr, ++idx)
        {
            dst.stat[idx].value = wrap_string_to_corba_string(value_text_ptr->first);
            dst.stat[idx].text  = wrap_string_to_corba_string(value_text_ptr->second);
        }

        dst.CrDate = wrap_boost_posix_time_ptime_to_string(src.crdate);
        // XXX IDL nonsense
        dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(src.last_update);
        // XXX IDL nonsense
        dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(src.last_transfer);
        dst.Name = wrap_Nullable_string_to_string(src.name);
        dst.Organization = wrap_Nullable_string_to_string(src.organization);

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
            dst.Streets[0] = wrap_Nullable_string_to_string(src.street1);
        }
        if (1 < number_of_streets) {
            dst.Streets[1] = wrap_Nullable_string_to_string(src.street2);
        }
        if (2 < number_of_streets) {
            dst.Streets[2] = wrap_Nullable_string_to_string(src.street3);
        }

        dst.City = wrap_Nullable_string_to_string(src.city);
        dst.StateOrProvince = wrap_Nullable_string_to_string(src.state_or_province);
        dst.PostalCode = wrap_Nullable_string_to_string(src.postal_code);
        dst.CountryCode = wrap_Nullable_string_to_string(src.country_code);
        dst.Telephone = wrap_Nullable_string_to_string(src.telephone);
        dst.Fax = wrap_Nullable_string_to_string(src.fax);
        dst.Email = wrap_Nullable_string_to_string(src.email);
        dst.NotifyEmail = wrap_Nullable_string_to_string(src.notify_email);
        dst.VAT = wrap_Nullable_string_to_string(src.VAT);
        dst.ident = wrap_Nullable_string_to_string(src.ident);
        dst.identtype = wrap_identtyp(src.identtype);
        dst.AuthInfoPw = wrap_Nullable_string_to_string(src.auth_info_pw);

        if (!src.disclose.is_initialized()) {
            dst.DiscloseFlag           = ccReg::DISCL_EMPTY;
            dst.DiscloseName           = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseOrganization   = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseAddress        = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseTelephone      = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseFax            = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseEmail          = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseVAT            = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseIdent          = wrap_int< CORBA::Boolean >(false);
            dst.DiscloseNotifyEmail    = wrap_int< CORBA::Boolean >(false);
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


    ccReg::NSSet wrap_localized_info_nsset(const Epp::LocalizedNssetInfoOutputData& _input ) {
        ccReg::NSSet result;

        result.handle = wrap_string_to_corba_string( _input.handle );
        result.ROID = wrap_string_to_corba_string( _input.roid );
        result.ClID = wrap_string_to_corba_string( _input.sponsoring_registrar_handle );
        result.CrID = wrap_string_to_corba_string( _input.creating_registrar_handle );
        // XXX IDL nonsense
        result.UpID = wrap_string_to_corba_string( _input.last_update_registrar_handle.isnull() ? std::string() : _input.last_update_registrar_handle.get_value() );

        {
            result.stat.length( _input.localized_external_states.size() );
            unsigned long i = 0;
            for(
                std::map<std::string, std::string>::const_iterator it = _input.localized_external_states.begin();
                it != _input.localized_external_states.end();
                ++it, ++i
            ) {
                result.stat[i].value = wrap_string_to_corba_string( it->first );
                result.stat[i].text = wrap_string_to_corba_string( it->second );
            }
        }

        result.CrDate = wrap_boost_posix_time_ptime_to_string(_input.crdate);
        // XXX IDL nonsense
        result.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_input.last_update);
        // XXX IDL nonsense
        result.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_input.last_transfer);

        result.AuthInfoPw = Corba::wrap_string_to_corba_string(_input.auth_info_pw.get_value_or_default());

        {
            result.dns.length( _input.dns_host.size() );
            unsigned long i = 0;
            for(
                std::vector<Epp::DNShostOutput>::const_iterator it = _input.dns_host.begin();
                it != _input.dns_host.end();
                ++it, ++i
            ) {
                result.dns[i].fqdn = wrap_string_to_corba_string( it->fqdn );

                result.dns[i].inet.length(it->inet_addr.size());
                unsigned long j = 0;
                for(
                    std::vector<boost::asio::ip::address>::const_iterator ipit = it->inet_addr.begin();
                    ipit != it->inet_addr.end();
                    ++ipit, ++j
                ) {
                    result.dns[i].inet[j] = wrap_string_to_corba_string(ipit->to_string());
                }
            }
        }

        {
            result.tech.length( _input.tech_contacts.size() );
            unsigned long i = 0;
            for(
                std::vector<std::string>::const_iterator it = _input.tech_contacts.begin();
                it != _input.tech_contacts.end();
                ++it, ++i
            ) {
                result.tech[i] = wrap_string_to_corba_string( *it);
            }
        }

        //TODO replace with superseder of Corba::int_to_int template
        numeric_cast_by_ref(_input.tech_check_level, result.level);

        return result;
    }

    static ccReg::CheckAvail wrap_contact_handle_check_result(const boost::optional< Epp::LocalizedContactHandleRegistrationObstruction >& _obstruction) {
        if (!_obstruction.is_initialized()) {
            return ccReg::NotExist;
        }

        switch (_obstruction.get().state)
        {
            case Epp::ContactHandleRegistrationObstruction::invalid_handle      : return ccReg::BadFormat;
            case Epp::ContactHandleRegistrationObstruction::protected_handle    : return ccReg::DelPeriod; // XXX oh my
            case Epp::ContactHandleRegistrationObstruction::registered_handle   : return ccReg::Exist;
        }

        throw std::runtime_error("unknown_contact_state");
    }

    static ccReg::CheckAvail wrap_nsset_handle_check_result(const boost::optional<Epp::LocalizedNssetHandleRegistrationObstruction>& _obstruction) {

        if(!_obstruction.is_initialized()) {
            return ccReg::NotExist;
        }

        switch( _obstruction.get().state ) {
            case Epp::NssetHandleRegistrationObstruction::invalid_handle      : return ccReg::BadFormat;
            case Epp::NssetHandleRegistrationObstruction::protected_handle    : return ccReg::DelPeriod; // XXX oh my
            case Epp::NssetHandleRegistrationObstruction::registered_handle   : return ccReg::Exist;
        }

        throw std::runtime_error("unknown_nsset_state");
    }

    std::vector<std::string> unwrap_ccreg_techcontacts_to_vector_string(const ccReg::TechContact & in)
    {
        std::vector<std::string> ret;
        ret.reserve(in.length());
        for(unsigned long long i = 0 ; i < in.length();++i)
        {
            if(in[i] == 0)
            {
                throw std::runtime_error("null char ptr");
            }
            ret.push_back(std::string(in[i]));
        }
        return ret;
    }

    std::vector<boost::optional<boost::asio::ip::address> > unwrap_inet_addr_to_vector_asio_addr(const ccReg::InetAddress& in)
    {
        std::vector<boost::optional<boost::asio::ip::address> > ret;
        ret.reserve(in.length());
        for(unsigned long long i = 0 ; i < in.length();++i)
        {
            if(in[i] == 0)
            {
                throw std::runtime_error("null char ptr");
            }
            boost::system::error_code boost_error_code;//invalid ip address is transformed to non-initialized optional
            boost::asio::ip::address ipaddr = boost::asio::ip::address::from_string(in[i],boost_error_code);
            boost::optional<boost::asio::ip::address> optional_ipaddr;
            if (!boost_error_code)
            {
                optional_ipaddr = ipaddr;
            }
            ret.push_back(optional_ipaddr);
        }
        return ret;
    }

    std::vector<Epp::DNShostInput> unwrap_ccreg_dnshosts_to_vector_dnshosts(const ccReg::DNSHost& in)
    {
        std::vector<Epp::DNShostInput> ret;
        ret.reserve(in.length());
        for(unsigned long long i = 0 ; i < in.length();++i)
        {
            if(in[i].fqdn == 0)
            {
                throw std::runtime_error("null char ptr");
            }
            ret.push_back(Epp::DNShostInput(std::string(in[i].fqdn),
                unwrap_inet_addr_to_vector_asio_addr(in[i].inet)));
        }
        return ret;
    }

    /**
     * @returns check results in the same order as input handles
     */
    ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& contact_handles,
        const std::map<std::string, boost::optional< Epp::LocalizedContactHandleRegistrationObstruction > > &contact_handle_check_results
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
            result[i].reason = wrap_string_to_corba_string(check_result.is_initialized() ? check_result.get().description : "");
        }

        return result;
    }

    static ccReg::CheckAvail wrap_keyset_handle_check_result(
        const Nullable< Epp::KeySet::Localized::HandlesCheck::Result > &_check_result)
    {
        if (_check_result.isnull()) {
            return ccReg::NotExist;
        }

        switch (_check_result.get_value().state)
        {
            case Epp::KeySet::HandleCheckResult::invalid_handle   : return ccReg::BadFormat;
            case Epp::KeySet::HandleCheckResult::protected_handle : return ccReg::DelPeriod;
            case Epp::KeySet::HandleCheckResult::registered_handle: return ccReg::Exist;
        }

        throw std::runtime_error("unknown keyset handle check result");
    }

    void wrap_Epp_KeySet_Localized_HandlesCheck_Results(
        const std::vector< std::string > &handles,
        const Epp::KeySet::Localized::HandlesCheck::Results &check_results,
        ccReg::CheckResp &dst)
    {
        dst.length(handles.size());

        typedef std::vector< std::string > Handles;
        ::size_t idx = 0;
        for (Handles::const_iterator handle_ptr = handles.begin(); handle_ptr != handles.end(); ++handle_ptr, ++idx)
        {
            typedef Epp::KeySet::Localized::HandlesCheck::Results CheckResults;
            const CheckResults::const_iterator result_ptr = check_results.find(*handle_ptr);
            if (result_ptr == check_results.end()) {
                throw std::out_of_range("handle " + (*handle_ptr) + " not found");
            }
            dst[idx].avail = wrap_keyset_handle_check_result(result_ptr->second);
            dst[idx].reason = result_ptr->second.isnull() ? ""
                                                          : result_ptr->second.get_value().description.c_str();
        }
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

    void wrap_Epp_KeySet_Localized_InfoData(const Epp::KeySet::Localized::InfoData &_src, ccReg::KeySet &_dst)
    {
        _dst.handle = _src.handle.c_str();
        _dst.ROID = _src.roid.c_str();
        _dst.ClID = _src.sponsoring_registrar_handle.c_str();
        _dst.CrID = _src.creating_registrar_handle.c_str();
        _dst.UpID = wrap_Nullable_string_to_string(_src.last_update_registrar_handle);
        wrap_Epp_LocalizedStates(_src.states, _dst.stat);
        _dst.CrDate = wrap_boost_posix_time_ptime_to_string(_src.crdate);
        _dst.UpDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_update);
        _dst.TrDate = wrap_Nullable_boost_posix_time_ptime_to_string(_src.last_transfer);
        _dst.AuthInfoPw = wrap_Nullable_string_to_string(_src.auth_info_pw);
        _dst.dsrec.length(0); // has to be empty
        wrap_Epp_KeysetInfoData_DnsKeys(_src.dns_keys, _dst.dnsk);
        wrap_Epp_KeysetInfoData_TechContacts(_src.tech_contacts, _dst.tech);
    }

    /**
     * @returns check results in the same order as input handles
     */
    ccReg::CheckResp wrap_localized_check_info(
        const std::vector<std::string>& nsset_handles,
        const std::map<std::string, boost::optional<Epp::LocalizedNssetHandleRegistrationObstruction> >& nsset_handle_check_results
    ) {
        ccReg::CheckResp result;
        result.length( nsset_handles.size() );

        CORBA::ULong i = 0;
        for(
            std::vector<std::string>::const_iterator it = nsset_handles.begin();
            it != nsset_handles.end();
            ++it, ++i
        ) {
            const boost::optional<Epp::LocalizedNssetHandleRegistrationObstruction> check_result = map_at(nsset_handle_check_results, *it);

            result[i].avail = wrap_nsset_handle_check_result( check_result );
            result[i].reason = Corba::wrap_string_to_corba_string( !check_result.is_initialized() ? "" : check_result.get().description );
        }

        return result;
    }

    Epp::DomainRegistrationTime unwrap_domain_registration_period(const ccReg::Period_str& period)
    {
        switch(period.unit)
        {
            case ccReg::unit_month:
                return Epp::DomainRegistrationTime(period.count, Epp::DomainRegistrationTime::Unit::month);
            case ccReg::unit_year:
                return Epp::DomainRegistrationTime(period.count, Epp::DomainRegistrationTime::Unit::year);
        };
        throw std::runtime_error("unwrap_domain_registration_period internal error");
    }

    std::vector<std::string> unwrap_ccreg_admincontacts_to_vector_string(const ccReg::AdminContact & in)
    {
        std::vector<std::string> ret;
        ret.reserve(in.length());
        for(unsigned long long i = 0; i < in.length(); ++i)
        {
            if(in[i] == 0)
            {
                throw std::runtime_error("null char ptr");
            }
            ret.push_back(std::string(in[i]));
        }
        return ret;
    }

    std::vector<Epp::ENUMValidationExtension> unwrap_enum_validation_extension(const ccReg::ExtensionList& ext)
    {
        const ccReg::ENUMValidationExtension *enum_ext = 0;
        std::vector<Epp::ENUMValidationExtension> ret;
        ret.reserve(ext.length());
        for(unsigned i = 0; i < ext.length(); ++i)
        {
            if (ext[i] >>= enum_ext)
            {
                ret.push_back(Epp::ENUMValidationExtension(
                        boost::gregorian::from_simple_string(
                                Corba::unwrap_string_from_const_char_ptr(enum_ext->valExDate)),
                        enum_ext->publish == ccReg::DISCL_DISPLAY)
                );
            }
            else
            {
                throw std::runtime_error("unknown extension found when extracting domain enum extension");
            }
        }
        return ret;
    }
}

