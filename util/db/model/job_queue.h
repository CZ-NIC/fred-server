/*  
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
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

/**
 *  @file job_queue.h
 *  Queue for simple batch query jobs
 */

#ifndef JOB_QUEUE_H_
#define JOB_QUEUE_H_

#include <queue>
#include <string>
#include <boost/format.hpp>

#include "../database.h"
#include "log/logger.h"


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

