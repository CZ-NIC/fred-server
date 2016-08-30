#include "src/epp/domain/domain_check_impl.h"

#include "src/epp/domain/domain_check_localization.h"
#include "src/epp/domain/domain_registration_obstruction.h"
#include "src/fredlib/object/object_type.h"
#include "util/idn_utils.h"

#include <boost/foreach.hpp>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/regex.hpp>
#include <boost/format.hpp>
#include <idna.h>

namespace Epp {

namespace Domain {

namespace {

bool domain_zone_is_not_in_registry(Fred::OperationContext& ctx, const std::string& domain_fqdn) {
    const Database::Result db_res = ctx.get_conn().exec(
            "SELECT fqdn as zone "
            "FROM zone "
            "ORDER BY LENGTH(fqdn) DESC"
    );

    ctx.get_log().debug(boost::format("domain_zone_is_not_in_registry(%1%) == zones:%2%") % domain_fqdn % db_res.size());

    if (db_res.size() == 0) {
        return true;
    }

    bool domain_zone_found = false;
    for (std::size_t i = 0; i < db_res.size(); ++i) {
        const std::string zone = static_cast<std::string>(db_res[i]["zone"]);
        ctx.get_log().debug(boost::format("domain_zone_is_not_in_registry(%1%) zone %2%") % domain_fqdn % zone);
        if (boost::algorithm::ends_with(domain_fqdn, zone)) {
            domain_zone_found = true;
            break;
        }
    }

    ctx.get_log().debug(boost::format("domain_zone_is_not_in_registry(%1%) == %2%") % domain_fqdn % !domain_zone_found);

    return !domain_zone_found;
}

bool domain_is_registered(Fred::OperationContext& ctx, const std::string& domain_fqdn) {
    const std::string object_type_name = Conversion::Enums::to_db_handle(Fred::Object_Type::domain);
    const Database::Result db_res = ctx.get_conn().exec_params(
            "SELECT 1 "
            "FROM object_registry "
            "WHERE erdate IS NULL "
              "AND type=get_object_type_id($1::TEXT) "
              "AND name=LOWER($2::TEXT)",
            Database::query_param_list
                (object_type_name)
                (domain_fqdn)
    );

    ctx.get_log().debug(boost::format("domain_is_registered(%1%) == %2%") % domain_fqdn % db_res.size());

    return db_res.size() > 0;
}

bool domain_is_blacklisted(Fred::OperationContext& ctx, const std::string& domain_fqdn) {
    const Database::Result db_res = ctx.get_conn().exec_params(
            "SELECT 1 "
            "FROM domain_blacklist "
            "WHERE regexp ~ '$1::TEXT' "
              "AND NOW() > valid_from "
              "AND "
                "(NOW() < valid_to "
                "OR valid_to IS NULL)",
            Database::query_param_list
                (domain_fqdn)
    );

    ctx.get_log().debug(boost::format("domain_is_blacklisted(%1%) == %2%") % domain_fqdn % db_res.size());

    return db_res.size() > 0;
}

std::string utf8_to_punycode(const std::string& fqdn) {
    char *p;

    if(idna_to_ascii_8z(fqdn.c_str(), &p, 0) == IDNA_SUCCESS) {
        std::string result( p );
        if (p != NULL) {
            free(p);
        }
        return result;

    } else {

        if (p != NULL) {
            free(p);
        }
        throw "id conversion failed";
    }
}

std::string punycode_to_utf8(const std::string& fqdn) {
    char *p;

    if(idna_to_unicode_8z8z(fqdn.c_str(), &p, 0) == IDNA_SUCCESS) {
        std::string result( p );
        if (p != NULL) {
            free(p);
        }
        return result;

    } else {

        if (p != NULL) {
            free(p);
        }
        throw "idn conversion failed";
    }
}

bool domain_fqdn_label_is_valid_punycode(const std::string& fqdn) {
    // TODO - prepared for proper check, current implementation is not strict enough
    std::string dev_null;

    try {
        dev_null = punycode_to_utf8(fqdn);
        utf8_to_punycode(dev_null);
    } catch (...) {
        return false;
    }

    return true;
}

bool domain_fqdn_is_invalid(const std::string& domain_fqdn) { // TODO refactoring
    //// ! the last asterisk means only that last label has {0,n} chars with 0 enabling fqdn to end with dot
    ////                                    (somelabel.)*(label )
    ////                                    |          | |      |
    //static const boost::regex fqdn_regex("([^\\.]+\\.)*[^\\.]*");
    static const boost::regex fqdn_regex("([a-z0-9]([a-z0-9-]{0,61}[a-z0-9])?\\.)+[a-z0-9][a-z0-9-]{0,61}[a-z0-9]\\.?", boost::regex::icase);
    //static const boost::regex label_regex("[a-z0-9]|[a-z0-9][-a-z0-9]{0,61}[a-z0-9]", boost::regex::icase);
    static const boost::regex label_regex("[a-z0-9]([a-z0-9-]{0,61}[a-z0-9])?", boost::regex::icase);

    static const boost::regex punycode_label_regex("xn--[-a-z0-9]{0,58}[a-z0-9]", boost::regex::icase);

    static const std::size_t domain_fqdn_length_max = 255; // TODO incl. final dot? // old impl. used 253 without dot

    if (domain_fqdn.length() > domain_fqdn_length_max) {
        return true;
    }

    if (domain_fqdn.length() == 0) {
        return true;
    }

    if(!boost::regex_match(domain_fqdn, fqdn_regex)) {
        return true;
    }

    const std::string domain_fqdn_without_root_dot = domain_fqdn.at(domain_fqdn.length() - 1) == '.' ? domain_fqdn.substr(0, domain_fqdn.length() - 1) : domain_fqdn; // FIXME

    if (domain_fqdn_without_root_dot.length() == 0) {
        return true;
    }

    const std::string enum_suffix = "e164.arpa";
    const unsigned enum_suffix_number_of_labels = 2;

    const bool is_enum_domain = boost::algorithm::ends_with(domain_fqdn_without_root_dot, enum_suffix);

    std::vector<std::string> labels;
    boost::split(labels, domain_fqdn_without_root_dot, boost::is_any_of("."));

    if(is_enum_domain) {
        if(labels.size() <= enum_suffix_number_of_labels) {
            return true;
        }
        for (std::vector<std::string>::const_iterator label_ptr = labels.begin(); label_ptr != labels.end(); ++label_ptr) {
            if ((*label_ptr).length() != 1 || !std::isdigit((*label_ptr).at(0))) {
                return true;
            }
        }
    }
    else {
        if(labels.size() == 0) {
            return true;
        }
        for (std::vector<std::string>::const_iterator label_ptr = labels.begin(); label_ptr != labels.end(); ++label_ptr) {
            const unsigned domain_fqdn_label_length_max = 63;
            if((*label_ptr).length() == 0) {
                return true;
            }
            if((*label_ptr).length() > domain_fqdn_label_length_max) {
                return true;
            }
            const bool is_idn_domain = (*label_ptr).find("--") != std::string::npos;
            if (is_idn_domain) {
                //if(!is_idn_allowed) {
                //    return true;
                //}
                if (!boost::regex_match(*label_ptr, punycode_label_regex)) {
                    return true;
                }
                else if(!domain_fqdn_label_is_valid_punycode(*label_ptr)) { // FIXME
                    return true;
                }
            }
            else {
                if(!boost::regex_match(*label_ptr, label_regex)) {
                    return true;
                }
            }
        }
    }
    return false;
}

Nullable<DomainRegistrationObstruction::Enum> domain_get_registration_obstruction_by_fqdn(Fred::OperationContext& ctx, const std::string domain_fqdn) {
    ctx.get_log().debug("--------------------------------------- MARK ------------------------------------------");

    if(domain_zone_is_not_in_registry(ctx, domain_fqdn)) {
        return DomainRegistrationObstruction::zone_not_in_registry;
    }
    else if(domain_is_registered(ctx, domain_fqdn)) {
        return DomainRegistrationObstruction::registered;
    }
    else if(domain_is_blacklisted(ctx, domain_fqdn)) {
        return DomainRegistrationObstruction::blacklisted;
    }
    else if(domain_fqdn_is_invalid(domain_fqdn)) {
        return DomainRegistrationObstruction::invalid_fqdn;
    }
    return Nullable<DomainRegistrationObstruction::Enum>();
}

} // namespace Epp::Domain::{anonymous}

DomainFqdnToDomainRegistrationObstruction domain_check_impl(
    Fred::OperationContext& ctx,
    const std::set<std::string>& domain_fqdns
) {
    DomainFqdnToDomainRegistrationObstruction domain_fqdn_to_domain_registration_obstruction;

    BOOST_FOREACH(const std::string &domain_fqdn, domain_fqdns) {
        domain_fqdn_to_domain_registration_obstruction[domain_fqdn] = domain_get_registration_obstruction_by_fqdn(ctx, domain_fqdn);
        //domain_fqdn_to_domain_registration_obstruction[domain_fqdn] = domain_state_to_domain_check_result(
        //    Fred::Domain::get_domain_fqdn_syntax_validity(domain_fqdn),
        //    Fred::Domain::get_domain_registrability_by_domain_fqdn(ctx, domain_fqdn)
        //);
    }

    return domain_fqdn_to_domain_registration_obstruction;
}

}

}
