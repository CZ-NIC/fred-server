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

Value<bool>& RequestImpl::addIsMonitoring()
{
  Value<bool> *tmp = new Value<bool>(Column("is_monitoring", joinRequestTable()));
  tmp->setName("IsMonitoring");
  add(tmp);
  return *tmp;
}

RequestServiceType& RequestImpl::addService()
{
  RequestServiceType *tmp = new RequestServiceType(Column("service", joinRequestTable()));
  tmp->setName("ServiceType");
  add(tmp);
  return *tmp;
}

RequestActionType& RequestImpl::addActionType()
{
  RequestActionType *tmp = new RequestActionType (Column("action_type", joinRequestTable()));
  tmp->setName("ActionType");
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

Value<bool>& RequestPropertyValueImpl::addOutputFlag()
{
  Value<bool> *tmp = new Value<bool>(Column("output", joinRequestPropertyValueTable()));
  tmp->setName("Output");
  add(tmp);
  return *tmp;
}

Table &RequestPropertyValueImpl::joinRequestPropertyTable()
{
	return joinTable("request_property");
}

Value<std::string>& RequestPropertyValueImpl::addName()
{
  addJoin(new Join(
	Column("name_id", joinRequestPropertyValueTable()),
	SQL_OP_EQ,
	Column("id", joinRequestPropertyTable())
  ));
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
  tmp->setName("Content");
  add(tmp);
  return *tmp;
}

Value<bool>& RequestDataImpl::addResponseFlag()
{
  Value<bool>* tmp = new Value<bool>(Column("is_response", joinRequestDataTable()));
  tmp->setName("ResponseFlag");
  add(tmp);
  return *tmp;
}

SessionImpl::SessionImpl(bool set_active) {
  setName("SessionImpl");
  active = set_active;
}

Session *Session::create() 
{
  return new SessionImpl(true);
}

Table &SessionImpl::joinSessionTable()
{
  return joinTable("session");
}

Value<Database::ID>& SessionImpl::addId()
{
  Value<Database::ID>* tmp = new Value<Database::ID>(Column("id", joinSessionTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<std::string>& SessionImpl::addName()
{
  Value<std::string>* tmp  = new Value<std::string>(Column("name", joinSessionTable())); 
  tmp->setName("Name");
  add(tmp);
  return *tmp;
}

Interval<Database::DateTimeInterval>& SessionImpl::addLoginDate()
{
  Interval<Database::DateTimeInterval>* tmp  = new Interval<Database::DateTimeInterval>(Column("login_date", joinSessionTable()));
  tmp->setName("LoginDate");
  add(tmp);
  return *tmp;
}

Interval<Database::DateTimeInterval>& SessionImpl::addLogoutDate()
{
  Interval<Database::DateTimeInterval>* tmp  = new Interval<Database::DateTimeInterval>(Column("logout_date", joinSessionTable()));
  tmp->setName("LogoutDate");
  add(tmp);
  return *tmp;
}

Value<std::string>& SessionImpl::addLang()
{
  Value<std::string>* tmp  = new Value<std::string>(Column("lang", joinSessionTable())); 
  tmp->setName("Lang");
  add(tmp);
  return *tmp;
}

std::ostream& operator<<(std::ostream &_os, const RequestServiceType& _v) {
  return _os << _v.getValue().getValue();
}

std::istream& operator>>(std::istream &_is, RequestServiceType& _v) {
  long int tmp;
  _is >> tmp;
  _v.setValue(Database::Null<long>(tmp));
  return _is;
}

std::ostream& operator<<(std::ostream &_os, const RequestActionType& _v) {
  return _os << _v.getValue().getValue();
}

std::istream& operator>>(std::istream &_is, RequestActionType& _v) {
  long int tmp;
  _is >> tmp;
  _v.setValue(Database::Null<long>(tmp));
  return _is;

}

bool operator<(const RequestServiceType &_left, const RequestServiceType &_right) 
{
	return _left.getValue().getValue() < _right.getValue().getValue();
}

bool operator>(const RequestServiceType &_left, const RequestServiceType &_right)
{
	return _left.getValue().getValue() > _right.getValue().getValue();
}

bool operator<(const RequestActionType &_left, const RequestActionType &_right) 
{
	return _left.getValue().getValue() < _right.getValue().getValue();
}

bool operator>(const RequestActionType &_left, const RequestActionType &_right)
{
	return _left.getValue().getValue() > _right.getValue().getValue();
}


}
}



