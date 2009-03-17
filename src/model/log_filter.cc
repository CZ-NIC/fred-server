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

Interval<Database::DateTimeInterval>& LogEntryImpl::addTimeBegin()
{
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("time_begin", joinLogEntryTable()));
  tmp->setName("TimeBegin");
  add(tmp);
  return *tmp;
}

Interval<Database::DateTimeInterval>& LogEntryImpl::addTimeEnd()
{
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("time_end", joinLogEntryTable()));
  tmp->setName("TimeEnd");
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

Value<Database::LogServiceType>& LogEntryImpl::addServiceType()
{
  Value<Database::LogServiceType> *tmp = new Value<Database::LogServiceType>(Column("service", joinLogEntryTable()));
  tmp->setName("ServiceType");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& LogEntryImpl::addLogRawContentId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("entry_id", joinTable("log_raw_content")));
  tmp->setName("LogRawContentId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& LogEntryImpl::addLogPropertyValueId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("entry_id", joinTable("log_property_value")));
  tmp->setName("LogPropertyValueId");
  add(tmp);
  return *tmp;
}

LogPropertyValue& LogEntryImpl::addLogPropertyValue()
{
  LogPropertyValue *tmp = new LogPropertyValueImpl(true);
  tmp->setName("LogPropertyValue");

  tmp->joinOn(new Join(Column("id", joinLogEntryTable()), SQL_OP_EQ,  Column("entry_id", tmp->joinLogPropertyValueTable())));
  add(tmp);
  return *tmp;
}

LogPropertyName& LogPropertyValueImpl::addLogPropertyName()
{
  LogPropertyName *tmp = new LogPropertyNameImpl(true);
  tmp->setName("LogPropertyName");

  tmp->joinOn(new Join(Column("name_id", joinLogPropertyValueTable()), SQL_OP_EQ,  Column("id", tmp->joinLogPropertyNameTable())));
  add(tmp);
  return *tmp;
}

LogRawContent& LogEntryImpl::addLogRawContent()
{
	LogRawContent *tmp = new LogRawContentImpl(true);
	tmp->setName("LogRawContent");

	tmp->joinOn(new Join(Column("id", joinLogEntryTable()), SQL_OP_EQ, Column("entry_id", tmp->joinLogRawContentTable())));
	add(tmp);
	return *tmp;
}

LogPropertyValueImpl::LogPropertyValueImpl(bool set_active) {
  setName("LogPropertyValue");
  active = set_active;
}

LogPropertyValue *LogPropertyValue::create()
{
  return new LogPropertyValueImpl(true);
}

Table &LogPropertyValueImpl::joinLogPropertyValueTable()
{
  return joinTable("log_property_value");
}

Value<std::string> &LogPropertyValueImpl::addValue()
{
  Value<std::string> *tmp = new Value<std::string>(Column("value", joinLogPropertyValueTable()));
  tmp->setName("Value");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& LogPropertyValueImpl::addLogPropertyNameId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("name_id", joinLogPropertyValueTable()));
  tmp->setName("NameId");
  add(tmp);
  return *tmp;
}

Value<bool>& LogPropertyValueImpl::addOutputFlag()
{
  Value<bool> *tmp = new Value<bool>(Column("output", joinLogPropertyValueTable()));
  tmp->setName("Output");
  add(tmp);
  return *tmp;
}

LogPropertyNameImpl::LogPropertyNameImpl(bool set_active)
{
	setName("LogPropertyName");
	active = set_active;
}

LogPropertyName *LogPropertyName::create()
{
  return new LogPropertyNameImpl(true);
}

Table &LogPropertyNameImpl::joinLogPropertyNameTable()
{
	return joinTable("log_property_name");
}

Value<std::string>& LogPropertyNameImpl::addName()
{
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinLogPropertyNameTable()));
  tmp->setName("Name");
  add(tmp);
  return *tmp;
}

LogRawContentImpl::LogRawContentImpl(bool set_active) {
  setName("LogRawContentImpl");
  active = set_active;
}

Table &LogRawContentImpl::joinLogRawContentTable()
{
	return joinTable("log_raw_content");
}

Value<std::string>& LogRawContentImpl::addContent()
{
  Value<std::string>* tmp = new Value<std::string>(Column("content", joinLogRawContentTable()));
  setName("Response");
  add(tmp);
  return *tmp;
}

Value<bool>& LogRawContentImpl::addResponseFlag()
{
  Value<bool>* tmp = new Value<bool>(Column("is_response", joinLogRawContentTable()));
  setName("RequestFlag");
  add(tmp);
  return *tmp;
}

}


}



