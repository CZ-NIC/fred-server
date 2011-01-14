#ifndef CONTACT_DIFF_H__
#define CONTACT_DIFF_H__

#include "contact.h"
#include "common_diff.h"

namespace Fred {
namespace Contact {


Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid);


Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id);


Fred::ChangesMap diff(const Contact *_prev,
                      const Contact *_act);


}
}


#endif /*CONTACT_DIFF_H__*/

