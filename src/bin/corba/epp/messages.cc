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
#include <stdio.h>
#include <string.h>

#include "src/bin/corba/epp/messages.hh"
#include "src/deprecated/util/log.hh"
#include "util/log/logger.hh"

Mesg::Mesg()
{
  m_lang=0;
}

void Mesg::AddMesg(
        const int id, const std::string msg, const std::string msg_cs)
{
    m_errMsg.push_back(msg);
    m_errMsg_cs.push_back(msg_cs);
    m_errID.push_back(id);
}

void Mesg::AddMesg(
    const int id, const char *msg, const char *msg_cs)
{
    m_errMsg.push_back(std::string(msg));
    m_errMsg_cs.push_back(std::string(msg_cs));
    m_errID.push_back(id);
}

Mesg::~Mesg()
{
}

std::string
Mesg::GetMesg(const int id)
{
    for (int i = 0; i < (int)m_errID.size(); i++) {
        if (id == m_errID.at(i)) {
            return m_errMsg.at(i);
        }
    }
    return "";
}
std::string
Mesg::GetMesg_CS(const int id)
{
    for (int i = 0; i < (int)m_errID.size(); i++) {
        if (id == m_errID.at(i)) {
            return m_errMsg_cs.at(i);
        }
    }
    return "";
}
