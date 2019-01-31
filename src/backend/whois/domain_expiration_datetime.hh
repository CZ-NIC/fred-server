#ifndef DOMAIN_EXPIRATION_DATETIME_HH_564FEC20D41643E9B256B3FAF736A818
#define DOMAIN_EXPIRATION_DATETIME_HH_564FEC20D41643E9B256B3FAF736A818

#include "libfred/opcontext.hh"
#include "util/optional_value.hh"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Fred {
namespace Backend {
namespace Whois {

/**
 * From domain expiration and object state procedure parameters computes
 * datetime when status 'expired' should be set
 *
 * @param   _exdate    domain expiration date
 * @return  domain expiration datetime
 */
boost::posix_time::ptime domain_expiration_datetime_estimate(
        LibFred::OperationContext& _ctx,
        const boost::gregorian::date& _exdate);


/**
 * @param   _domain_id   id of domain
 * @return  domain 'expired' status valid from date time if this status is active
 *          if _domain_id or active status is not found it returns ''not set'' optional value
 */
Optional<boost::posix_time::ptime> domain_expiration_datetime_actual(
        LibFred::OperationContext& _ctx,
        unsigned long long _domain_id);


/**
 * From domain validation expiration and object state procedure parameters computes
 * datetime when status 'not_validated' should be set
 *
 * @param   _exdate    domain validation expiration date
 * @return  domain validation expiration datetime
 */
boost::posix_time::ptime domain_validation_expiration_datetime_estimate(
        LibFred::OperationContext& _ctx,
        const boost::gregorian::date& _exdate);


/**
 * @param   _domain_id   id of domain
 * @return  domain 'not_validated' status valid from date time if this status is active
 *          if _domain_id or active status is not found it returns ''not set'' optional value
 */
Optional<boost::posix_time::ptime> domain_validation_expiration_datetime_actual(
        LibFred::OperationContext& _ctx,
        unsigned long long _domain_id);


} // namespace Fred::Backend::Whois
} // namespace Fred::Backend
} // namespace Fred

#endif
