#ifndef NSSET_DIFF_H_B068B6284C66485C9142EE2EA089774B
#define NSSET_DIFF_H_B068B6284C66485C9142EE2EA089774B

#include "src/fredlib/nsset.h"
#include "common_diff.h"

namespace Fred {
namespace Nsset {


Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid);


Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id);


Fred::ChangesMap diff(const Nsset *_prev,
                      const Nsset *_act);


}
}


#endif /*NSSET_DIFF_H__*/
