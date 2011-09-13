#include "invoice_filter.h"

namespace Database {
namespace Filters {

Invoice* Invoice::create() {
  return new InvoiceImpl();
}

InvoiceImpl::InvoiceImpl() : Compound() {
  setName("Invoice");
  active = true;
}

InvoiceImpl::~InvoiceImpl() {
}

Table& InvoiceImpl::joinInvoiceTable() {
  return joinTable("invoice");
}

Value<Database::ID>& InvoiceImpl::addId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("id", joinInvoiceTable()));
  tmp->setName("Id");
  add(tmp);
  return *tmp;
}

Value<Database::ID>& InvoiceImpl::addZoneId() {
  Value<Database::ID> *tmp = new Value<Database::ID>(Column("zone_id", joinInvoiceTable()));
  tmp->setName("ZoneId");
  add(tmp);
  return *tmp;
}

Zone &
InvoiceImpl::addZone()
{
    Zone *tmp = new ZoneImpl();
    tmp->joinOn(new Join(
                Column("zone_id", joinInvoiceTable()),
                SQL_OP_EQ,
                Column("id", tmp->joinZoneTable())
                ));
    add(tmp);
    tmp->setName("Zone");
    return *tmp;
}

Value<int>& InvoiceImpl::addType() {
  addJoin(new Join(
                   Column("invoice_prefix_id", joinInvoiceTable()),
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

Value<Database::ID>& InvoiceImpl::addFileXML() {
  Column column = Column("filexml", joinInvoiceTable());
  Value<Database::ID> *tmp = new Value<Database::ID>(column);
  add(tmp);
  tmp->setName("FileXML");
  return *tmp;
}

Value<Database::ID>& InvoiceImpl::addFilePDF() {
  Column column = Column("file", joinInvoiceTable());
  Value<Database::ID> *tmp = new Value<Database::ID>(column);
  add(tmp);
  tmp->setName("FilePDF");
  return *tmp;
}

Interval<Database::DateTimeInterval>& InvoiceImpl::addCreateTime() {
  Interval<Database::DateTimeInterval> *tmp = new Interval<Database::DateTimeInterval>(Column("crdate", joinInvoiceTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Interval<Database::DateInterval>& InvoiceImpl::addTaxDate() {
  Interval<Database::DateInterval> *tmp = new Interval<Database::DateInterval>(Column("taxdate", joinInvoiceTable()));
  add(tmp);
  tmp->setName("TaxDate");
  return *tmp;
}

Registrar& InvoiceImpl::addRegistrar() {
  Registrar *tmp = new RegistrarImpl();
  add(tmp);
  tmp->joinOn(new Join(
                       Column("registrar_id", joinInvoiceTable()), 
                       SQL_OP_EQ, 
                       Column("id", tmp->joinRegistrarTable())));
  tmp->setName("Registrar");
  return *tmp;  
}

Object& InvoiceImpl::addObject() {
  Object *tmp = Object::create();
  add(tmp);
  addJoin(new Join(
                   Column("id", joinInvoiceTable()),
                   SQL_OP_EQ,
                   Column("ac_invoice_id", joinTable("invoice_operation"))
                   ));
  tmp->joinOn(new Join(
                       Column("object_id", joinTable("invoice_operation")),
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
