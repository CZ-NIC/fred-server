#include "log_filter.h"

namespace Database {
namespace Filters {

Request* Request::create() {
	return new RequestImpl(true);
}

RequestImpl::RequestImpl(bool set_active) : Compound() {
	setName("Request");
	active = set_active;
}

Table& RequestImpl::joinRequestTable()
{
  return joinTable("request");
}

Value<Database::ID>& RequestImpl::addId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinRequestTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Interval<Database::DateTimeInterval>& RequestImpl::addTimeBegin()
{
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("time_begin", joinRequestTable()));
  tmp->setName("TimeBegin");
  add(tmp);
  return *tmp;
}

Interval<Database::DateTimeInterval>& RequestImpl::addTimeEnd()
{
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("time_end", joinRequestTable()));
  tmp->setName("TimeEnd");
  add(tmp);
  return *tmp;
}

Value<std::string>& RequestImpl::addSourceIp()
{
  Value<std::string> *tmp = new Value<std::string>(Column("source_ip", joinRequestTable()));
  tmp->setName("SourceIp");
  add(tmp);
  return *tmp;
}

Value<Database::RequestServiceType>& RequestImpl::addServiceType()
{
  Value<Database::RequestServiceType> *tmp = new Value<Database::RequestServiceType>(Column("service", joinRequestTable()));
  tmp->setName("ServiceType");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& RequestImpl::addRequestDataId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("entry_id", joinTable("request_data")));
  tmp->setName("RequestDataId");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& RequestImpl::addRequestPropertyValueId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("entry_id", joinTable("request_property_value")));
  tmp->setName("RequestPropertyValueId");
  add(tmp);
  return *tmp;
}

RequestPropertyValue& RequestImpl::addRequestPropertyValue()
{
  RequestPropertyValue *tmp = new RequestPropertyValueImpl(true);
  tmp->setName("RequestPropertyValue");

  tmp->joinOn(new Join(Column("id", joinRequestTable()), SQL_OP_EQ,  Column("entry_id", tmp->joinRequestPropertyValueTable())));
  add(tmp);
  return *tmp;
}

RequestProperty& RequestPropertyValueImpl::addRequestProperty()
{
  RequestProperty *tmp = new RequestPropertyImpl(true);
  tmp->setName("RequestProperty");

  tmp->joinOn(new Join(Column("name_id", joinRequestPropertyValueTable()), SQL_OP_EQ,  Column("id", tmp->joinRequestPropertyTable())));
  add(tmp);
  return *tmp;
}

RequestData& RequestImpl::addRequestData()
{
	RequestData *tmp = new RequestDataImpl(true);
	tmp->setName("RequestData");

	tmp->joinOn(new Join(Column("id", joinRequestTable()), SQL_OP_EQ, Column("entry_id", tmp->joinRequestDataTable())));
	add(tmp);
	return *tmp;
}

RequestPropertyValueImpl::RequestPropertyValueImpl(bool set_active) {
  setName("RequestPropertyValue");
  active = set_active;
}

RequestPropertyValue *RequestPropertyValue::create()
{
  return new RequestPropertyValueImpl(true);
}

Table &RequestPropertyValueImpl::joinRequestPropertyValueTable()
{
  return joinTable("request_property_value");
}

Value<std::string> &RequestPropertyValueImpl::addValue()
{
  Value<std::string> *tmp = new Value<std::string>(Column("value", joinRequestPropertyValueTable()));
  tmp->setName("Value");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& RequestPropertyValueImpl::addRequestPropertyId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("name_id", joinRequestPropertyValueTable()));
  tmp->setName("NameId");
  add(tmp);
  return *tmp;
}

Value<bool>& RequestPropertyValueImpl::addOutputFlag()
{
  Value<bool> *tmp = new Value<bool>(Column("output", joinRequestPropertyValueTable()));
  tmp->setName("Output");
  add(tmp);
  return *tmp;
}

RequestPropertyImpl::RequestPropertyImpl(bool set_active)
{
	setName("RequestProperty");
	active = set_active;
}

RequestProperty *RequestProperty::create()
{
  return new RequestPropertyImpl(true);
}

Table &RequestPropertyImpl::joinRequestPropertyTable()
{
	return joinTable("request_property");
}

Value<std::string>& RequestPropertyImpl::addName()
{
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinRequestPropertyTable()));
  tmp->setName("Name");
  add(tmp);
  return *tmp;
}

RequestDataImpl::RequestDataImpl(bool set_active) {
  setName("RequestDataImpl");
  active = set_active;
}

Table &RequestDataImpl::joinRequestDataTable()
{
	return joinTable("request_data");
}

Value<std::string>& RequestDataImpl::addContent()
{
  Value<std::string>* tmp = new Value<std::string>(Column("content", joinRequestDataTable()));
  setName("Response");
  add(tmp);
  return *tmp;
}

Value<bool>& RequestDataImpl::addResponseFlag()
{
  Value<bool>* tmp = new Value<bool>(Column("is_response", joinRequestDataTable()));
  setName("RequestFlag");
  add(tmp);
  return *tmp;
}

}


}



