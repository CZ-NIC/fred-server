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
     */
    Optional<boost::posix_time::ptime> domain_expiration_datetime_actual(
        Fred::OperationContext &_ctx,
        unsigned long long _domain_id
    );

}


#endif
