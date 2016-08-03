#include "src/corba/epp/corba_conversions.h"

#include "src/corba/EPP.hh"
#include "src/epp/error.h"
#include "src/epp/contact/ident_type.h"
#include "src/epp/param.h"
#include "src/epp/disclose_policy.h"
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
            result.push_back( Corba::unwrap_string_from_const_char_ptr(handles[i]) );
        }

        return result;
    }

    Epp::RequestParams unwrap_epp_request_params(const ccReg::EppParams& _epp_request_params) {

        return Epp::RequestParams(
            CorbaConversion::int_to_int<unsigned long long>(_epp_request_params.loginID),
            Corba::unwrap_string(_epp_request_params.clTRID),
            _epp_request_params.requestID == 0
                ?   Optional<unsigned long long>()
                :   Optional<unsigned long long>( CorbaConversion::int_to_int<unsigned long long>(_epp_request_params.requestID) )
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

        CorbaConversion::int_to_int( to_description_db_id(_input.response), result.code );
        result.svTRID = Corba::wrap_string_to_corba_string(_server_transaction_handle);
        result.msg = Corba::wrap_string_to_corba_string(_input.localized_msg);

        return result;
    }

    ccReg::EPP::EppError wrap_error(const Epp::LocalizedFailResponse& _input, const std::string& _server_transaction_handle) {
        ccReg::EPP::EppError result;

        CorbaConversion::int_to_int( to_description_db_id(_input.response), result.errCode );
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
            (   _corba_streets.length() > 0
                ?   _corba_streets[0].in() != NULL ? Corba::unwrap_string(_corba_streets[0].in()) : ""
                :   ""
            ),
            (   _corba_streets.length() > 1
                ?   _corba_streets[1].in() != NULL ? Corba::unwrap_string(_corba_streets[1].in()) : ""
                :   ""
            ),
            (   _corba_streets.length() > 2
                ?   _corba_streets[2].in() != NULL ? Corba::unwrap_string(_corba_streets[2].in()) : ""
                :   ""
            )
        );
    }

    Epp::ContactCreateInputData unwrap_contact_create_input_data(const char* const handle, const ccReg::ContactChange& c) {

        const StreetAddressPart streets = Corba::unwrap_streets(c.Streets);

        return Epp::ContactCreateInputData(
            boost::trim_copy( Corba::unwrap_string(handle) ),
            boost::trim_copy( Corba::unwrap_string(c.Name) ),
            boost::trim_copy( Corba::unwrap_string(c.Organization) ),
            boost::trim_copy( streets.street_line_1 ),
            boost::trim_copy( streets.street_line_2 ),
            boost::trim_copy( streets.street_line_3 ),
            boost::trim_copy( Corba::unwrap_string(c.City) ),
            boost::trim_copy( Corba::unwrap_string(c.StateOrProvince) ),
            boost::trim_copy( Corba::unwrap_string(c.PostalCode) ),
            boost::trim_copy( Corba::unwrap_string(c.CC) ),
            boost::trim_copy( Corba::unwrap_string(c.Telephone) ),
            boost::trim_copy( Corba::unwrap_string(c.Fax) ),
            boost::trim_copy( Corba::unwrap_string(c.Email) ),
            boost::trim_copy( Corba::unwrap_string(c.NotifyEmail) ),
            boost::trim_copy( Corba::unwrap_string(c.VAT) ),
            boost::trim_copy( Corba::unwrap_string(c.ident) ),
            Corba::unwrap_ident_type(c.identtype),
            boost::trim_copy( Corba::unwrap_string(c.AuthInfoPw) ),
            should_be_disclosed(c.DiscloseName, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseOrganization, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseAddress, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseTelephone, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseFax, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseEmail, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseVAT, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseIdent, c.DiscloseFlag),
            should_be_disclosed(c.DiscloseNotifyEmail, c.DiscloseFlag)
        );
    }

    namespace {

    bool is_disclosed(Epp::DiscloseFlag::Enum value)
    {
        switch (value)
        {
            case Epp::DiscloseFlag::HIDE:     return false;
            case Epp::DiscloseFlag::DISCLOSE: return true;
        }
        throw std::runtime_error("Invalid disclose flag value");
    }

    bool is_hidden(Epp::DiscloseFlag::Enum value)
    {
        switch (value)
        {
            case Epp::DiscloseFlag::HIDE:     return true;
            case Epp::DiscloseFlag::DISCLOSE: return false;
        }
        throw std::runtime_error("Invalid disclose flag value");
    }

    bool is_default_policy_disclose()
    {
        return is_disclosed(Epp::get_default_disclose_policy());
    }

    bool is_default_policy_hide()
    {
        return is_hidden(Epp::get_default_disclose_policy());
    }

    }//namespace Corba::{anonymous}

    //disclose flag rules:
    // a)   in DISCL_HIDE mode and default policy DISCLOSE, then 'true' is saved into
    //      database for idl items with 'false' value and 'false' for idl items with 'true' value
    //
    // b)   in DISCL_DISPLAY mode and default policy DISCLOSE, then is saved into database
    //      to all items 'true' value
    //
    // c)   in DISCL_HIDE mode and default policy HIDE, then is saved into database
    //      to all items 'false' value
    //
    // d)   in DISCL_DISPLAY mode and default policy HIDE, then 'true' is saved into
    //      database for idl items with 'true' value and 'false' for idl items with 'false' value
    //
    // e)   in DISCL_EMPTY mode and default policy DISCLOSE, then 'true' is saved into database for all
    //
    // f)   in DISCL_EMPTY mode and default policy HIDE, then 'false' is saved into database for all
    //
    //  rule | mode          | policy   | db
    // ======+===============+==========+=======
    //   a)  | DISCL_HIDE    | DISCLOSE | !idl
    //   b)  | DISCL_DISPLAY | DISCLOSE | true
    //   c)  | DISCL_HIDE    | HIDE     | false
    //   d)  | DISCL_DISPLAY | HIDE     | idl
    //   e)  | DISCL_EMPTY   | DISCLOSE | true
    //   f)  | DISCL_EMPTY   | HIDE     | false
    //
    //  id | mode          | policy   | idl   | flag
    // ====+===============+==========+=======+=======
    //   1 | DISCL_HIDE    | HIDE     | false | HIDE
    //   2 | DISCL_HIDE    | HIDE     | true  | HIDE
    //   3 | DISCL_HIDE    | DISCLOSE | false | DISCLOSE
    //   4 | DISCL_HIDE    | DISCLOSE | true  | HIDE
    //   5 | DISCL_DISPLAY | HIDE     | false | HIDE
    //   6 | DISCL_DISPLAY | HIDE     | true  | DISCLOSE
    //   7 | DISCL_DISPLAY | DISCLOSE | false | DISCLOSE
    //   8 | DISCL_DISPLAY | DISCLOSE | true  | DISCLOSE
    //   9 | DISCL_EMPTY   | HIDE     | false | HIDE
    //  10 | DISCL_EMPTY   | HIDE     | true  | HIDE
    //  11 | DISCL_EMPTY   | DISCLOSE | false | DISCLOSE
    //  12 | DISCL_EMPTY   | DISCLOSE | true  | DISCLOSE

    bool should_be_disclosed(CORBA::Boolean is_set, ccReg::Disclose mode)
    {
        const bool value = CorbaConversion::int_to_int< bool >(is_set);
        switch (mode)
        {
            case ccReg::DISCL_HIDE:
                return is_disclosed((is_default_policy_disclose() && !value) ? Epp::DiscloseFlag::DISCLOSE
                                                                             : Epp::DiscloseFlag::HIDE);
            case ccReg::DISCL_DISPLAY:
                return is_disclosed((is_default_policy_hide() && !value) ? Epp::DiscloseFlag::HIDE
                                                                         : Epp::DiscloseFlag::DISCLOSE);
            case ccReg::DISCL_EMPTY:
                return is_disclosed(Epp::get_default_disclose_policy());
        }
        throw std::runtime_error("Invalid ccReg::Disclose value");
    }

    // update it is same as create only if there is DISCL_EMPTY flag then database isn't updated (no matter to server policy)
    Optional< bool > get_new_disclose_flag_value(CORBA::Boolean is_set, ccReg::Disclose mode)
    {
        return (mode == ccReg::DISCL_EMPTY) ? Optional< bool >()
                                            : should_be_disclosed(is_set, mode);
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
                    :   boost::trim_copy( Corba::unwrap_string(input) );
    }

    Epp::ContactUpdateInputData unwrap_contact_update_input_data(const char* const handle, const ccReg::ContactChange& c) {
        return Epp::ContactUpdateInputData(
            Corba::unwrap_string(handle),
            convert_corba_string_change(c.Name),
            convert_corba_string_change(c.Organization),
            convert_corba_string_change(c.Streets.length() > 0 ? c.Streets[0] : ""),
            convert_corba_string_change(c.Streets.length() > 1 ? c.Streets[1] : ""),
            convert_corba_string_change(c.Streets.length() > 2 ? c.Streets[2] : ""),
            convert_corba_string_change(c.City),
            convert_corba_string_change(c.StateOrProvince),
            convert_corba_string_change(c.PostalCode),
            convert_corba_string_change(c.CC),
            convert_corba_string_change(c.Telephone),
            convert_corba_string_change(c.Fax),
            convert_corba_string_change(c.Email),
            convert_corba_string_change(c.NotifyEmail),
            convert_corba_string_change(c.VAT),
            convert_corba_string_change(c.ident),
            Corba::unwrap_ident_type(c.identtype),
            convert_corba_string_change(c.AuthInfoPw),
            get_new_disclose_flag_value(c.DiscloseName, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseOrganization, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseAddress, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseTelephone, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseFax, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseEmail, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseVAT, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseIdent, c.DiscloseFlag),
            get_new_disclose_flag_value(c.DiscloseNotifyEmail, c.DiscloseFlag)
        );
    }

    static std::string formatTime(const boost::posix_time::ptime& tm) {
        char buffer[100];
        convert_rfc3339_timestamp(buffer, sizeof(buffer), boost::posix_time::to_iso_extended_string(tm).c_str());
        return buffer;
    }

    // A)  if there is policy DISCLOSE, then flag means "hide" and items from database, which
    // have value 'false', will have value 'true', rest 'false'. If there isn't once single item with 'true',
    // flag DISCL_EMPTY returns (value of items aren't unsubstantial)
    //
    // B)  if there is policy HIDE, then flag means "disclose" and items from database, which
    // have value 'true', will have value 'true', rest 'false'. If there isn't once single item with 'true',
    // flag DISCL_EMPTY returns (value of items aren't unsubstantial)
    CORBA::Boolean wrap_disclose_flag_to_Boolean(bool is_set)
    {
        switch (Epp::get_default_disclose_policy())
        {
            case Epp::DiscloseFlag::HIDE:
                return CorbaConversion::int_to_int< CORBA::Boolean >(is_set);
            case Epp::DiscloseFlag::DISCLOSE:
                return CorbaConversion::int_to_int< CORBA::Boolean >(!is_set);
        }
        throw std::runtime_error("Invalid default disclose policy value");
    }

    namespace {

    ccReg::Disclose wrap_disclose_flag_to_disclose(Epp::DiscloseFlag::Enum disclose_flag)
    {
        switch (disclose_flag)
        {
            case Epp::DiscloseFlag::HIDE:     return ccReg::DISCL_DISPLAY;
            case Epp::DiscloseFlag::DISCLOSE: return ccReg::DISCL_HIDE;
        }
        throw std::runtime_error("Invalid disclose flag value");
    }

    }//namespace Corba::{anonymous}

    ccReg::Contact wrap_localized_info_contact(const Epp::LocalizedContactInfoOutputData& _input ) {
        ccReg::Contact result;

        result.handle = wrap_string_to_corba_string( _input.handle );
        result.ROID = wrap_string_to_corba_string( _input.roid );
        result.ClID = wrap_string_to_corba_string( _input.sponsoring_registrar_handle );
        result.CrID = wrap_string_to_corba_string( _input.creating_registrar_handle );
        // XXX IDL nonsense
        result.UpID = wrap_string_to_corba_string( _input.last_update_registrar_handle.isnull() ? std::string() : _input.last_update_registrar_handle.get_value() );

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

        result.CrDate = wrap_string_to_corba_string( formatTime( _input.crdate ) );
        // XXX IDL nonsense
        result.UpDate = wrap_string_to_corba_string(
            _input.last_update.isnull()
                ? std::string()
                : formatTime( _input.last_update.get_value() )
        );
        // XXX IDL nonsense
        result.TrDate = wrap_string_to_corba_string(
            _input.last_transfer.isnull()
                ? std::string()
                : formatTime( _input.last_transfer.get_value() )
        );
        result.Name = wrap_string_to_corba_string(_input.name.get_value_or_default());
        result.Organization = wrap_string_to_corba_string(_input.organization.get_value_or_default());

        result.Streets.length(
            ! _input.street3.isnull() && ! _input.street3.get_value().empty()
                ? 3
                : ! _input.street2.isnull() && ! _input.street2.get_value().empty()
                    ? 2
                    : ! _input.street1.isnull() && ! _input.street1.get_value().empty()
                        ? 1
                        : 0
        );
        if(result.Streets.length() > 0) {
            result.Streets[0] = Corba::wrap_string_to_corba_string(_input.street1.get_value_or_default());
        }
        if(result.Streets.length() > 1) {
            result.Streets[1] = Corba::wrap_string_to_corba_string(_input.street2.get_value_or_default());
        }
        if(result.Streets.length() > 2) {
            result.Streets[2] = Corba::wrap_string_to_corba_string(_input.street3.get_value_or_default());
        }

        result.City = Corba::wrap_string_to_corba_string(_input.city.get_value_or_default());
        result.StateOrProvince = Corba::wrap_string_to_corba_string(_input.state_or_province.get_value_or_default());
        result.PostalCode = Corba::wrap_string_to_corba_string(_input.postal_code.get_value_or_default());
        result.CountryCode = Corba::wrap_string_to_corba_string(_input.country_code.get_value_or_default());
        result.Telephone = Corba::wrap_string_to_corba_string(_input.telephone.get_value_or_default());
        result.Fax = Corba::wrap_string_to_corba_string(_input.fax.get_value_or_default());
        result.Email = Corba::wrap_string_to_corba_string(_input.email.get_value_or_default());
        result.NotifyEmail = Corba::wrap_string_to_corba_string(_input.notify_email.get_value_or_default());
        result.VAT = Corba::wrap_string_to_corba_string(_input.VAT.get_value_or_default());
        result.ident = Corba::wrap_string_to_corba_string(_input.ident.get_value_or_default());
        result.identtype = Corba::wrap_ident_type(_input.identtype);
        result.AuthInfoPw = Corba::wrap_string_to_corba_string(_input.auth_info_pw.get_value_or_default());

        result.DiscloseFlag           = wrap_disclose_flag_to_disclose(Epp::get_default_disclose_policy());
        result.DiscloseName           = wrap_disclose_flag_to_Boolean(_input.disclose_name);
        result.DiscloseOrganization   = wrap_disclose_flag_to_Boolean(_input.disclose_organization);
        result.DiscloseAddress        = wrap_disclose_flag_to_Boolean(_input.disclose_address);
        result.DiscloseTelephone      = wrap_disclose_flag_to_Boolean(_input.disclose_telephone);
        result.DiscloseFax            = wrap_disclose_flag_to_Boolean(_input.disclose_fax);
        result.DiscloseEmail          = wrap_disclose_flag_to_Boolean(_input.disclose_email);
        result.DiscloseVAT            = wrap_disclose_flag_to_Boolean(_input.disclose_VAT);
        result.DiscloseIdent          = wrap_disclose_flag_to_Boolean(_input.disclose_ident);
        result.DiscloseNotifyEmail    = wrap_disclose_flag_to_Boolean(_input.disclose_notify_email);

        // if there is policy DISCLOSE or HIDE and if there isn't once single item with 'true',
        // flag DISCL_EMPTY is set
        const bool no_one_of_disclose_flags_is_set = !result.DiscloseName &&
                                                     !result.DiscloseOrganization &&
                                                     !result.DiscloseAddress &&
                                                     !result.DiscloseTelephone &&
                                                     !result.DiscloseFax &&
                                                     !result.DiscloseEmail &&
                                                     !result.DiscloseVAT &&
                                                     !result.DiscloseIdent &&
                                                     !result.DiscloseNotifyEmail;
        if (no_one_of_disclose_flags_is_set) {
            result.DiscloseFlag = ccReg::DISCL_EMPTY;
        }

        return result;
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
        result.length( contact_handles.size() );

        CORBA::ULong i = 0;
        for(
            std::vector<std::string>::const_iterator it = contact_handles.begin();
            it != contact_handles.end();
            ++it, ++i
        ) {
            const boost::optional< Epp::LocalizedContactHandleRegistrationObstruction > check_result = map_at(contact_handle_check_results, *it);

            result[i].avail = wrap_contact_handle_check_result( check_result );
            result[i].reason = Corba::wrap_string_to_corba_string(check_result.is_initialized() ? check_result.get().description : "");
        }

        return result;
    }
}
