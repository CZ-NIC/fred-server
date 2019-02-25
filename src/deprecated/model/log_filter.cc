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
#include "src/deprecated/model/log_filter.hh"
#include "src/util/types/null.hh"

namespace Database { 

namespace Filters {

ResultCode *ResultCode::create() {
        return new ResultCodeImpl(true);
}

ResultCodeImpl::ResultCodeImpl(bool set_active) : Compound() {
        setName("ResultCode");
        active = set_active;
}

Table &ResultCodeImpl::joinResultCodeTable()
{
    return joinTable("result_code");
}

Value<Database::ID>& ResultCodeImpl::addServiceId() 
{
    Value<Database::ID> *tmp = new Value<Database::ID>(Column("service_id", joinResultCodeTable()));
    tmp->setName("ServiceId");
    add(tmp);
    return *tmp;
}

Value<int>& ResultCodeImpl::addResultCode() 
{
    Value<int> *tmp = new Value<int>(Column("result_code", joinResultCodeTable()));
    tmp->setName("ResultCode");
    add(tmp);
    return *tmp;
}

Value<std::string>& ResultCodeImpl::addName()
{
    Value<std::string> *tmp = new Value<std::string>(Column("name", joinResultCodeTable()));
    tmp->setName("Name");
    add(tmp);
    return *tmp;
}

RequestObjectRef* RequestObjectRef::create() {
        return new RequestObjectRefImpl(true);
}

RequestObjectRefImpl::RequestObjectRefImpl(bool set_active) : Compound() {
        setName("RequestObjectRef");
        active = set_active;
}

Table& RequestObjectRefImpl::joinRequestObjectRefTable()
{
    return joinTable("request_object_ref");
}

Table& RequestObjectRefImpl::joinRequestObjectTypeTable()
{
    return joinTable("request_object_type");
}

Value<std::string>& RequestObjectRefImpl::addObjectType()
{
  addJoin(new Join(
        Column("object_type_id", joinRequestObjectRefTable()),
        SQL_OP_EQ,
        Column("id", joinRequestObjectTypeTable())
    ));
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinRequestObjectTypeTable()));
  tmp->setName("ObjectType");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& RequestObjectRefImpl::addObjectId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("object_id", joinRequestObjectRefTable()));
  tmp->setName("ObjectId");
  add(tmp);
  return *tmp;
}





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

Value<std::string>& RequestImpl::addUserName()
{  
  Value<std::string> *tmp = new Value<std::string>(Column("user_name", joinRequestTable()));
  tmp->setName("UserName");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& RequestImpl::addUserId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("user_id", joinRequestTable()));
  tmp->setName("UserId");
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

ServiceType& RequestImpl::addServiceType()
{
  ServiceType *tmp = new ServiceType(Column("service_id", joinRequestTable()));
  tmp->setName("ServiceType");
  add(tmp);
  return *tmp;
}


RequestType& RequestImpl::addRequestType()
{
  RequestType *tmp = new RequestType (Column("request_type_id", joinRequestTable()));
  tmp->setName("RequestType");
  add(tmp);
  return *tmp;
}

RequestPropertyValue& RequestImpl::addRequestPropertyValue()
{
  RequestPropertyValue *tmp = new RequestPropertyValueImpl(true);
  tmp->setName("RequestPropertyValue");

  tmp->joinOn(new Join(Column("id", joinRequestTable()), SQL_OP_EQ,  Column("request_id", tmp->joinRequestPropertyValueTable())));
  add(tmp);
  return *tmp;
}

RequestData& RequestImpl::addRequestData()
{
	RequestData *tmp = new RequestDataImpl(true);
	tmp->setName("RequestData");

	tmp->joinOn(new Join(Column("id", joinRequestTable()), SQL_OP_EQ, Column("request_id", tmp->joinRequestDataTable())));
	add(tmp);
	return *tmp;
}

ResultCode& RequestImpl::addResultCode()
{
  ResultCode *tmp = new ResultCodeImpl(true);
  tmp->setName("ResultCode");
  
  tmp->joinOn(new Join(Column("result_code_id", joinRequestTable()), SQL_OP_EQ,
        Column("id", tmp->joinResultCodeTable())));              

  // tmp->joinOn(new Join(Column("id", tmp->joinResultCodeTable()), SQL_OP_EQ, 
  //        Column("result_code_id", joinRequestTable())));
  add(tmp);
  return *tmp;
}

RequestObjectRef& RequestImpl::addRequestObjectRef()
{
  RequestObjectRef *tmp = new RequestObjectRefImpl(true);
  tmp->setName("RequestObjectRef");
  
  tmp->joinOn(new Join(Column("id", joinRequestTable()), SQL_OP_EQ, Column("request_id", tmp->joinRequestObjectRefTable())));
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
	return joinTable("request_property_name");
}

Value<std::string>& RequestPropertyValueImpl::addName()
{
  addJoin(new Join(
	Column("property_name_id", joinRequestPropertyValueTable()),
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

Value<std::string>& SessionImpl::addUserName()
{
  Value<std::string>* tmp  = new Value<std::string>(Column("user_name", joinSessionTable())); 
  tmp->setName("UserName");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& SessionImpl::addUserId()
{
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("user_id", joinSessionTable()));
  tmp->setName("UserId");
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

std::ostream& operator<<(std::ostream &_os, const ServiceType& _v) {
  return _os << _v.getValue().getValue();
}

std::istream& operator>>(std::istream &_is, ServiceType& _v) {
  long int tmp;
  _is >> tmp;
  _v.setValue(Database::Null<long>(tmp));
  return _is;
}

std::ostream& operator<<(std::ostream &_os, const RequestType& _v) {
  return _os << _v.getValue().getValue();
}

std::istream& operator>>(std::istream &_is, RequestType& _v) {
  long int tmp;
  _is >> tmp;
  _v.setValue(Database::Null<long>(tmp));
  return _is;

}

bool operator<(const ServiceType &_left, const ServiceType &_right) 
{
	return _left.getValue().getValue() < _right.getValue().getValue();
}

bool operator>(const ServiceType &_left, const ServiceType &_right)
{
	return _left.getValue().getValue() > _right.getValue().getValue();
}

bool operator<(const RequestType &_left, const RequestType &_right) 
{
	return _left.getValue().getValue() < _right.getValue().getValue();
}

bool operator>(const RequestType &_left, const RequestType &_right)
{
	return _left.getValue().getValue() > _right.getValue().getValue();
}


}
}



