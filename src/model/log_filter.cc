#include "log_filter.h"

namespace Database {
namespace Filters {

LogEntry* LogEntry::create() { 
	return new LogEntryImpl(true); 
} 

LogEntryImpl::LogEntryImpl(bool set_active) : Compound() { 
	setName("LogEntry"); 
	active = set_active; 
} 

Table& LogEntryImpl::joinLogEntryTable()
{
  return joinTable("log_entry");
}

Value<Database::ID>& LogEntryImpl::addId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinLogEntryTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}
Interval<Database::DateTimeInterval>& LogEntryImpl::addDateTime()
{
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("time", joinLogEntryTable()));
  tmp->setName("Timestamp");
  add(tmp);
  return *tmp;
}

Value<std::string>& LogEntryImpl::addSourceIp()
{
  Value<std::string> *tmp = new Value<std::string>(Column("source_ip", joinLogEntryTable()));
  tmp->setName("SourceIp");
  add(tmp);
  return *tmp;
}

Value<Database::LogEventType>& LogEntryImpl::addFlag()
{
  Value<Database::LogEventType> *tmp = new Value<Database::LogEventType>(Column("flag", joinLogEntryTable()));
  tmp->setName("Flag");
  add(tmp);
  return *tmp;
}

Value<Database::LogComponent>& LogEntryImpl::addComponent()
{
  Value<Database::LogComponent> *tmp = new Value<Database::LogComponent>(Column("component", joinLogEntryTable()));
  tmp->setName("Component");
  add(tmp);
  return *tmp;
}

Value<std::string>& LogEntryImpl::addContent()
{
  Value<std::string> *tmp = new Value<std::string>(Column("content", joinLogEntryTable()));
  tmp->setName("Content");
  add(tmp);
  return *tmp;
}

Value<int>& LogEntryImpl::addClientId()
{
  Value<int> *tmp = new Value<int>(Column("client_id", joinLogEntryTable()));
  tmp->setName("ClientId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& LogEntryImpl::addLogPropertyId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("entry_id", joinTable("log_property")));
  tmp->setName("LogPropertyId");
  add(tmp);
  return *tmp;
}

LogProperty& LogEntryImpl::addLogProperty()
{
  LogProperty *tmp = new LogPropertyImpl(true);
  tmp->setName("LogProperty");

  tmp->joinOn(new Join(Column("id", joinLogEntryTable()), SQL_OP_EQ,  Column("entry_id", tmp->joinLogPropertyTable())));
  add(tmp);
  return *tmp;
}

LogPropertyImpl::LogPropertyImpl(bool set_active) {
  setName("LogProperty");
  active = set_active;
}

LogProperty *LogProperty::create()
{
  return new LogPropertyImpl(true);
}

Table &LogPropertyImpl::joinLogPropertyTable()
{
  return joinTable("log_property");     
}

Value<std::string> &LogPropertyImpl::addValue()
{
  Value<std::string> *tmp = new Value<std::string>(Column("value", joinLogPropertyTable()));
  tmp->setName("Value");
  add(tmp);
  return *tmp;
}

Value<std::string> &LogPropertyImpl::addName()
{
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinLogPropertyTable()));
  tmp->setName("Name");
  add(tmp);
  return *tmp;
}

}
}

