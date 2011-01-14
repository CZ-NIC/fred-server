#ifndef DOMAIN_DIFF_H__
#define DOMAIN_DIFF_H__

#include "domain.h"
#include "common_diff.h"

namespace Fred {
namespace Domain {


Fred::ChangesMap diff(Manager *_m,
                      const unsigned long long &_prev_hid,
                      const unsigned long long &_act_hid);


Fred::ChangesMap diff_last_history(Manager *_m, const unsigned long long &_id);


Fred::ChangesMap diff(const Domain *_prev,
                      const Domain *_act);


}
}


#endif /*DOMAIN_DIFF_H__*/

