/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file job_queue.h
 *  Queue for simple batch query jobs
 */

#ifndef JOB_QUEUE_HH_5939D4346CAC4B96A447F58E854A241A
#define JOB_QUEUE_HH_5939D4346CAC4B96A447F58E854A241A

#include "src/deprecated/libfred/db_settings.hh"

#include "util/db/database.hh"
#include "util/log/logger.hh"

#include <boost/format.hpp>

#include <queue>
#include <string>

namespace Model {


class JobQueue : public std::queue<Database::Statement*> {
public:
  virtual ~JobQueue() {
    while (!empty()) {
      Database::Statement *q = front();
      pop();
      delete q;
    }
  }

  
  void execute() {
    while (!empty()) {
      Database::Statement *q = front();
      Database::Connection c = Database::Manager::acquire();
      c.exec(*q);
      pop();
      delete q;
    }
  }
};

}


#endif /*JOBS_QUEUE_H_*/

