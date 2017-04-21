/*
 *  Copyright (C) 2017  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef MESSAGES_H_58EFA707C484477FB1267D1695B2EEB9
#define MESSAGES_H_58EFA707C484477FB1267D1695B2EEB9

#include <iostream>
#include <vector>

#define LANG_EN 0
#define LANG_CS 1

class Mesg
{
public:
  Mesg();
  ~Mesg();

  void SetLang(
    int l)
  {
    if (l == LANG_CS)
      m_lang = LANG_CS;
    else
      m_lang = LANG_EN; // default EN
  }

  int GetLang()
  {
    return m_lang;
  }
  int GetNum()
  {
    return m_errID.size();
  }

  void AddMesg(const int id, const char *msg, const char *msg_cs);
  void AddMesg(const int id, const std::string msg, const std::string msg_cs);
  std::string GetMesg(int id);
  std::string GetMesg_CS(int id);

private:
  std::vector<std::string> m_errMsg;
  std::vector<std::string> m_errMsg_cs;
  std::vector<int> m_errID;
  int m_lang;
};

#endif
