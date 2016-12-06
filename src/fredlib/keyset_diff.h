#ifndef KEYSET_DIFF_H_6C292B52F76A43DB83ED2C1137F26040
#define KEYSET_DIFF_H_6C292B52F76A43DB83ED2C1137F26040

#include "src/fredlib/keyset.h"
#include "common_diff.h"

namespace Fred {
namespace Keyset {


Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid);


Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id);


Fred::ChangesMap diff(const Keyset *_prev,
                      const Keyset *_act);


}
}


#endif /*KEYSET_DIFF_H__*/

