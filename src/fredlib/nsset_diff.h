#ifndef NSSET_DIFF_H__
#define NSSET_DIFF_H__

#include "nsset.h"
#include "common_diff.h"

namespace Fred {
namespace NSSet {


Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid);


Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id);


Fred::ChangesMap diff(const NSSet *_prev,
                      const NSSet *_act);


}
}


#endif /*NSSET_DIFF_H__*/

