#ifndef KEYSET_DIFF_H__
#define KEYSET_DIFF_H__

#include "keyset.h"
#include "common_diff.h"

namespace Fred {
namespace KeySet {


Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid);


Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id);


Fred::ChangesMap diff(const KeySet *_prev,
                      const KeySet *_act);


}
}


#endif /*KEYSET_DIFF_H__*/

