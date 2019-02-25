/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef MAIL_HH_8C9378EE39F54B9695A7961ED83CA3DA
#define MAIL_HH_8C9378EE39F54B9695A7961ED83CA3DA

#include "src/deprecated/libfred/common_object.hh"
#include "src/deprecated/libfred/object.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"

namespace LibFred {
namespace Mail {

enum MemberType {
  MT_CRDATE,
  MT_TYPE,
  MT_STATUS
};


class Mail : virtual public LibFred::CommonObject {
public:
  virtual const Database::DateTime& getCreateTime() const = 0;
  virtual const Database::DateTime& getModTime() const = 0;
  virtual const long &getType() const = 0;
  virtual const std::string& getTypeDesc() const = 0;
  virtual const long &getStatus() const = 0;
  virtual const std::string& getContent() const = 0;
  
  virtual void addHandle(const std::string& _handle) = 0;
  virtual unsigned getHandleSize() const = 0;
  virtual const std::string& getHandle(unsigned _idx) const = 0;
  
  virtual void addAttachment(const LibFred::OID& _oid) = 0;
  virtual unsigned getAttachmentSize() const = 0;
  virtual const LibFred::OID& getAttachment(unsigned _idx) const = 0;
};


class List : virtual public LibFred::CommonList {
public:
  virtual Mail* get(unsigned _idx) const = 0;
  virtual void reload(Database::Filters::Union& _filter) = 0;
  virtual void sort(MemberType _member, bool _asc) = 0;
  
  /// from CommonList; propably will be removed in future
  virtual const char* getTempTableName() const = 0;
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;
  virtual void reload() = 0;
};


class Manager {
public:
  virtual ~Manager() {
  }
  
  virtual List* createList() const = 0;
  static Manager* create();
};

}
}

#endif /*MAIL_H_*/
