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
#ifndef PIDFILE_HH_0CC94EEC68B8493EAA21406B4CA09FEF
#define PIDFILE_HH_0CC94EEC68B8493EAA21406B4CA09FEF

#include <fstream>
#include <string>
#include <exception>

//Maintenance of the process pidfile

namespace PidFileNS
{
  //PID type
#ifndef PIDFILE_HH_0CC94EEC68B8493EAA21406B4CA09FEF
  typedef unsigned long pid_t;
#endif

  //creating and removing pidfile
  class PidFileS
  {
    std::string   m_pidFile;
    static bool pidfile_created;

    PidFileS(); // no default ctor 
    PidFileS(std::string pidFileName)
      : m_pidFile(pidFileName)
    {}
    PidFileS(PidFileS const&); // no copy ctor
    PidFileS& operator=(PidFileS const&); // no assignment op.
    ~PidFileS() // dtor
    {
      remove (m_pidFile.c_str());
    }

  public:

    //creation of pidfile - call only once
    static void writePid(pid_t pid, std::string pidFileName)
    {
      if (pidfile_created)
      {
        throw std::runtime_error("pidfile already created error");
      }
      std::ofstream ofstrFile;
      ofstrFile.open(pidFileName.c_str()
        ,std::ios_base::out | std::ios_base::trunc);
      if(!ofstrFile)
      {
        throw std::runtime_error("pidfile open error");
      }

      ofstrFile << pid << std::endl;
      ofstrFile.close();
      static PidFileS instance(pidFileName);
      pidfile_created = true;
    }

  };//class PidFileS

  //pidfile creation flag
  bool PidFileS::pidfile_created = false;

} // namespace pidfilens

#endif
