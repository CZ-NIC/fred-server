/*
 * Copyright (C) 2010  CZ.NIC, z.s.p.o.
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


#ifndef TEST_REGISTRAR_GROUP_H_
#define TEST_REGISTRAR_GROUP_H_

#include <memory>
#include <iostream>
#include <string>
#include <algorithm>
#include <functional>
#include <numeric>
#include <map>
#include <exception>
#include <queue>
#include <sys/time.h>
#include <time.h>

#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time.hpp>
#include <boost/assign/list_of.hpp>

#include "register/db_settings.h"
#include "corba_wrapper.h"
#include "log/logger.h"
#include "log/context.h"

#include "random_data_generator.h"
#include "concurrent_queue.h"
#include "common.h"
#include "register/model_files.h"

#include "handle_general_args.h"
#include "handle_database_args.h"
#include "handle_threadgroup_args.h"
#include "handle_corbanameservice_args.h"


ModelFiles mf;
unsigned long long mfid = 0;

#ifdef BOOST_NO_STDC_NAMESPACE
namespace std
{
  using ::time;
}
#endif


#endif // TEST_REGISTRAR_GROUP_H_





#ifndef TESTREGISTRARGROUP_H_
#define TESTREGISTRARGROUP_H_


#endif /* TESTREGISTRARGROUP_H_ */