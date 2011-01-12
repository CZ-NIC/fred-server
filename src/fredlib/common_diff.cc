#include "common_diff.h"

namespace Fred {


void _diff_object(ChangesMap &_changes,
                  const Fred::Object *_prev,
                  const Fred::Object *_act)
{
    compare_and_fill(_changes, "object.authinfo", _prev->getAuthPw(), _act->getAuthPw());
}


}

