#ifndef DOMAIN_EXPIRATION_DATETIME_H_20934852039
#define DOMAIN_EXPIRATION_DATETIME_H_20934852039

#include "src/fredlib/opcontext.h"
#include "util/optional_value.h"
#include <boost/date_time/posix_time/posix_time.hpp>

namespace Whois
{

    /**
    * From domain expiration and object state procedure parameters computes
    * datetime when status 'expired' should be set
    *
    * @param   _exdate    domain expiration date
    * @return  domain expiration datetime
    */
    boost::posix_time::ptime domain_expiration_datetime_estimate(
        Fred::OperationContext &_ctx,
        const boost::gregorian::date &_exdate
    );


    /**
     * @param   _domain_id   id of domain
     * @return  domain 'expired' status valid from date time if this status is active
     *          if _domain_id or active status is not found it returns ''not set'' optional value
     */
    Optional<boost::posix_time::ptime> domain_expiration_datetime_actual(
        Fred::OperationContext &_ctx,
        unsigned long long _domain_id
    );


    /**
    * From domain validation expiration and object state procedure parameters computes
    * datetime when status 'not_validated' should be set
    *
    * @param   _exdate    domain validation expiration date
    * @return  domain validation expiration datetime
    */
    boost::posix_time::ptime domain_validation_expiration_datetime_estimate(
        Fred::OperationContext &_ctx,
        const boost::gregorian::date &_exdate
    );


    /**
     * @param   _domain_id   id of domain
     * @return  domain 'not_validated' status valid from date time if this status is active
     *          if _domain_id or active status is not found it returns ''not set'' optional value
     */
    Optional<boost::posix_time::ptime> domain_validation_expiration_datetime_actual(
        Fred::OperationContext &_ctx,
        unsigned long long _domain_id
    );

}


#endif