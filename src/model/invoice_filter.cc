#include "invoice_filter.h"

namespace DBase {
namespace Filters {

InvoiceImpl::InvoiceImpl() :
  Compound() {
  setName("Invoice");
  active = true;
}

InvoiceImpl::~InvoiceImpl() {
}

Table& InvoiceImpl::joinInvoiceTable() {
  return joinTable("invoice");
}

Value<DBase::ID>& InvoiceImpl::addId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("id", joinInvoiceTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<DBase::ID>& InvoiceImpl::addZoneId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("zone", joinInvoiceTable()));
  tmp->setName("ZoneId");
  add(tmp);
  return *tmp;
}

Value<int>& InvoiceImpl::addType() {
  addJoin(new Join(
                   Column("prefix_type", joinInvoiceTable()),
                   SQL_OP_EQ,
                   Column("id", joinTable("invoice_prefix"))
                   ));
  Value<int> *tmp = new Value<int>(Column("typ", joinTable("invoice_prefix")));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}

Value<std::string>& InvoiceImpl::addNumber() {
  Column column = Column("prefix", joinInvoiceTable());
  column.castTo("varchar");
  Value<std::string> *tmp = new Value<std::string>(column);
  add(tmp);
  tmp->setName("Number");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& InvoiceImpl::addCreateTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(Column("crdate", joinInvoiceTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Interval<DBase::DateInterval>& InvoiceImpl::addTaxDate() {
  Interval<DBase::DateInterval> *tmp = new Interval<DBase::DateInterval>(Column("taxdate", joinInvoiceTable()));
  add(tmp);
  tmp->setName("TaxDate");
  return *tmp;
}

Registrar& InvoiceImpl::addRegistrar() {
  Registrar *tmp = new RegistrarImpl();
  add(tmp);
  tmp->joinOn(new Join(
                       Column("registrarid", joinInvoiceTable()), 
                       SQL_OP_EQ, 
                       Column("id", tmp->joinRegistrarTable())));
  tmp->setName("Registrar");
  return *tmp;  
}

Object& InvoiceImpl::addObject() {
  Object *tmp = new ObjectHistoryImpl();
  add(tmp);
  addJoin(new Join(
                   Column("id", joinInvoiceTable()),
                   SQL_OP_EQ,
                   Column("invoiceid", joinTable("invoice_object_registry"))
                   ));
  tmp->joinOn(new Join(
                       Column("objectid", joinTable("invoice_object_registry")),
                       SQL_OP_EQ, 
                       Column("id", tmp->joinObjectTable())
                       ));
  tmp->setName("Object");
  return *tmp;
}

File& InvoiceImpl::addFile() {
  File *tmp = new FileImpl();
  tmp->joinOn(new Join(
                       Column("file", joinInvoiceTable()),
                       SQL_OP_EQ,
                       Column("id", tmp->joinFileTable())
                       ));
  add(tmp);
  tmp->setName("File");
  return *tmp;
}

}
}
