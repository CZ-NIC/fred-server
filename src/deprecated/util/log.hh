/*
 * Copyright (C) 2006-2019  CZ.NIC, z. s. p. o.
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
#ifndef LOG_HH_35AF77A705CD4C51A25DBE915252CEF2
#define LOG_HH_35AF77A705CD4C51A25DBE915252CEF2

#include <syslog.h>
#include <stdarg.h>

#define EMERG_LOG     LOG_EMERG
#define ALERT_LOG     LOG_ALERT
#define ERROR_LOG     LOG_ERR
#define CRIT_LOG      LOG_CRIT
#define WARNING_LOG   LOG_WARNING
#define SQL_LOG       LOG_DEBUG
#define DEBUG_LOG     LOG_DEBUG
#define NOTICE_LOG    LOG_NOTICE
#define INFO_LOG      LOG_INFO

class SysLogger
{
private:
  SysLogger() :
    level(DEBUG_LOG), facility(2)
  {
  } ///< ctor hidden
  SysLogger(
    SysLogger const&)
  {
  } ///< copy ctor hidden
  SysLogger& operator=(
    SysLogger const&)
  {
    return *this;
  } ///< assign op. hidden
  ~SysLogger()
  {
  } ///< dtor hidden
  int level; ///< minimal level of logging
  int facility; ///< syslog facility 
public:
  static SysLogger &get()
  {
    static SysLogger l;
    return l;
  }
  void log(
    int prio, const char* t, ...)
  {
    if (level < prio)
      return;
    va_list arglist;
    va_start(arglist, t);
    vsyslog(prio | ((LOG_FAC(LOG_LOCAL0)+facility) << 3), t, arglist);
    va_end(arglist);
  }
  void setFacility(
    int _facility)
  {
    facility = _facility;
  }
  void setLevel(
    int _level)
  {
    level = _level;
  }
};

/**
 * commented out old style logger
 *
 *  #define LOG SysLogger::get().log
 */

#endif

