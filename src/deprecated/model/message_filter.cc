/*
 * Copyright (C) 2010-2019  CZ.NIC, z. s. p. o.
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
#include "src/deprecated/model/message_filter.hh"

namespace Database {
namespace Filters {

MessageImpl::MessageImpl(bool set_active)
{
    setName("Message");
    active = set_active;
}

MessageImpl::~MessageImpl()
{
}

Table &
MessageImpl::joinMessageArchiveTable()
{
    return joinTable("message_archive");
}

Value<Database::ID> &
MessageImpl::addId()
{
    Value<Database::ID> *tmp = new Value<Database::ID>(
            Column("id", joinMessageArchiveTable()));
    add(tmp);
    tmp->setName("Id");
    return *tmp;
}

Interval<Database::DateTimeInterval> &
MessageImpl::addCrDate()
{
    Interval<Database::DateTimeInterval> *tmp =
        new Interval<Database::DateTimeInterval>(
            Column("crdate", joinMessageArchiveTable()));
    add(tmp);
    tmp->setName("CrDate");
    return *tmp;
}

Interval<Database::DateTimeInterval> &
MessageImpl::addModDate()
{
    Interval<Database::DateTimeInterval> *tmp =
        new Interval<Database::DateTimeInterval>(
            Column("moddate", joinMessageArchiveTable()));
    add(tmp);
    tmp->setName("ModDate");
    return *tmp;
}

Value<int> &
MessageImpl::addAttempt()
{
    Value<int> *tmp = new Value<int>(
            Column("attempt", joinMessageArchiveTable()));
    add(tmp);
    tmp->setName("Attempt");
    return *tmp;
}

Value<int> &
MessageImpl::addStatus()
{
    Value<int> *tmp = new Value<int>(
            Column("status_id", joinMessageArchiveTable()));
    add(tmp);
    tmp->setName("Status");
    return *tmp;
}


Value<int> &
MessageImpl::addCommType()
{
    Value<int> *tmp = new Value<int>(
            Column("comm_type_id", joinMessageArchiveTable()));
    add(tmp);
    tmp->setName("CommType");
    return *tmp;
}

Value<int> &
MessageImpl::addMessageType()
{
    Value<int> *tmp = new Value<int>(
            Column("message_type_id", joinMessageArchiveTable()));
    add(tmp);
    tmp->setName("MessageType");
    return *tmp;
}

Value<std::string>&
MessageImpl::addSmsPhoneNumber()
{
    joinMessageArchiveTable();
    this->active = true;

    addJoin(new Join(
      Column("id", joinTable("message_archive")),
      SQL_OP_EQ,
      Column("id", joinTable("sms_archive"))
    ));

    Value<std::string> *tmp = new Value<std::string>(Column("phone_number", joinTable("sms_archive")));
    tmp->setName("SmsPhoneNumber");
    add(tmp);
    return *tmp;

}


Value<std::string>&
MessageImpl::addLetterAddrName()
{
    joinMessageArchiveTable();

    addJoin(new Join(
      Column("id", joinMessageArchiveTable()),
      SQL_OP_EQ,
      Column("id", joinTable("letter_archive"))
    ));

    Value<std::string> *tmp = new Value<std::string>(Column("postal_address_name", joinTable("letter_archive")));
    tmp->setName("LetterAddresName");
    add(tmp);
    return *tmp;

}

Contact& MessageImpl::addMessageContact() {
  Contact *tmp = new ContactImpl();
  add(tmp);
  tmp->setName("MessageContact");
  tmp->addJoin(
          new Join(
              Column("contact_object_registry_id", joinTable("message_contact_history_map")),
              SQL_OP_EQ,
              Column("id", tmp->joinObjectRegistryTable())
              )
          );
  tmp->joinOn(
          new Join(
              Column("id", joinMessageArchiveTable()),
              SQL_OP_EQ,
              Column("message_archive_id", joinTable("message_contact_history_map"))
              )
          );

  return *tmp;
}




} // namespace Filters
} // namespace Database
