#ifndef MOJEID_NOTIFIER_H__
#define MOJEID_NOTIFIER_H__

#include <boost/shared_ptr.hpp>
#include "fredlib/mailer.h"
#include "fredlib/notifier/ntf_except.h"


namespace Fred {
namespace MojeID {


void notify_contact_create(const unsigned long long &_request_id,
                           boost::shared_ptr<Fred::Mailer::Manager> _mailer);


void notify_contact_update(const unsigned long long &_request_id,
                           boost::shared_ptr<Fred::Mailer::Manager> _mailer);


void notify_contact_transfer(const unsigned long long &_request_id,
                             boost::shared_ptr<Fred::Mailer::Manager> _mailer);


}
}


#endif /*MOJEID_NOTIFIER_H__*/

