#include "file_filter.h"

namespace DBase {
namespace Filters {

FileImpl::FileImpl(bool _set_active) :
  Compound() {
  setName("File");
  active = _set_active;
}

FileImpl::~FileImpl() {
}

Table& FileImpl::joinFileTable() {
  return joinTable("files");
}

Value<DBase::ID>& FileImpl::addId() {
  Value<DBase::ID> *tmp = new Value<DBase::ID>(Column("id", joinFileTable()));
  add(tmp);
  tmp->setName("Id");
  return *tmp;
}

Value<std::string>& FileImpl::addName() {
  Value<std::string> *tmp = new Value<std::string>(Column("name", joinFileTable()));
  add(tmp);
  tmp->setName("Name");
  return *tmp;
}

Value<std::string>& FileImpl::addPath() {
  Value<std::string> *tmp = new Value<std::string>(Column("path", joinFileTable()));
  add(tmp);
  tmp->setName("Path");
  return *tmp;
}

Value<std::string>& FileImpl::addMimeType() {
  Value<std::string> *tmp = new Value<std::string>(Column("mimetype", joinFileTable()));
  add(tmp);
  tmp->setName("MimeType");
  return *tmp;
}

Interval<DBase::DateTimeInterval>& FileImpl::addCreateTime() {
  Interval<DBase::DateTimeInterval> *tmp = new Interval<DBase::DateTimeInterval>(
                                                  Column("crdate", joinFileTable()));
  add(tmp);
  tmp->setName("CreateTime");
  return *tmp;
}

Value<int>& FileImpl::addSize() {
  Value<int> *tmp = new Value<int>(Column("filesize", joinFileTable()));
  add(tmp);
  tmp->setName("Size");
  return *tmp;
}

Value<int>& FileImpl::addType() {
  Value<int> *tmp = new Value<int>(Column("filetype", joinFileTable()));
  add(tmp);
  tmp->setName("Type");
  return *tmp;
}


}
}
