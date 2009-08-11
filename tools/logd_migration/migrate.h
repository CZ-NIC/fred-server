#ifndef _MIGRATE_H_
#define _MIGRATE_H_

#include <iostream>
#include <fstream>
#include <map>


#include "db/transaction.h"
#include "log_impl.h"

#include "util/types/id.h"

// needed becase of pool_subst
#include "m_epp_parser.h"

#define ALLOC_STEP 4

using namespace Database;

typedef unsigned long long TID;

typedef Impl_Log Backend;



#endif

