#include "message_filter.h"

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

} // namespace Filters
} // namespace Database
