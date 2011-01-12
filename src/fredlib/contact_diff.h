#ifndef CONTACT_DIFF_H__
#define CONTACT_DIFF_H__

#include "contact.h"
#include "common_diff.h"

namespace Fred {
namespace Contact {


Fred::ChangesMap diff(Manager *_cm,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid);

Fred::ChangesMap diff(const Contact *_prev,
                      const Contact *_act);


}
}


#endif /*CONTACT_DIFF_H__*/

