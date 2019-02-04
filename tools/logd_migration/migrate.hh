#ifndef MIGRATE_HH_B07B1FEC47E94E6FBABACCB0D12970E9
#define MIGRATE_HH_B07B1FEC47E94E6FBABACCB0D12970E9

#include <iostream>
#include <fstream>
#include <map>


#include "util/db/transaction.hh"

#include "src/deprecated/libfred/requests/request.hh"


#include "util/types/id.hh"

// needed becase of pool_subst
#include "tools/logd_migration/m_epp_parser.hh"

#define ALLOC_STEP 4

using namespace Database;
using namespace LibFred::Logger;

typedef unsigned long long TID;


#endif

