/*
 * Copyright (C) 2011  CZ.NIC, z.s.p.o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef REGBLOCK_PARAMS_H_
#define REGBLOCK_PARAMS_H_

#include "util/types/optional.h"


struct RegBlockArgs {
    bool over_limit;
    bool list_only;
    optional_ulonglong block_id;
    optional_ulonglong unblock_id;
    unsigned shell_cmd_timeout;
    std::string notify_email;

    RegBlockArgs() :
        over_limit(false),
        list_only(false),
        block_id(0),
        unblock_id(0),
        shell_cmd_timeout(0),
        notify_email()
        {  }

    RegBlockArgs(
            bool _over_limit,
            bool _list_only,
            optional_ulonglong _block_id,
            optional_ulonglong _unblock_id,
            unsigned _shell_cmd_timeout,
            std::string _notify_email) :
                over_limit(_over_limit),
                list_only(_list_only),
                block_id(_block_id),
                unblock_id(_unblock_id),
                shell_cmd_timeout(_shell_cmd_timeout),
                notify_email(_notify_email)
        {  }
};

#endif // REGBLOCK_PARAMS_H_
