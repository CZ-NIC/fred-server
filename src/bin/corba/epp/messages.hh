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
#ifndef MESSAGES_HH_377A21B18114490F9B559BBF0539E492
#define MESSAGES_HH_377A21B18114490F9B559BBF0539E492

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
