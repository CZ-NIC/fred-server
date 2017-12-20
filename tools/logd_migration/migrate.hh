#ifndef _MIGRATE_H_
#define _MIGRATE_H_

#include <iostream>
#include <fstream>
#include <map>


#include "src/util/db/transaction.hh"

#include "src/libfred/requests/request.hh"


#include "src/util/types/id.hh"

// needed becase of pool_subst
#include "tools/logd_migration/m_epp_parser.hh"

#define ALLOC_STEP 4

using namespace Database;
using namespace LibFred::Logger;

typedef unsigned long long TID;


#endif

