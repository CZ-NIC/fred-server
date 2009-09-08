#ifndef PIDFILE_H
#define PIDFILE_H

#include <fstream>
#include <string>
#include <exception>

//Maintenance of the process pidfile

namespace PidFileNS
{
  //PID type
#ifndef pid_t
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

}  //namespace pidfilens

#endif //PIDFILE_H
