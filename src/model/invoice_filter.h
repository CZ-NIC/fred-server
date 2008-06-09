#ifndef INVOICE_FILTER_H_
#define INVOICE_FILTER_H_

#include "db/base_filters.h"
#include "object_filter.h"
#include "registrar_filter.h"
#include "file_filter.h"

namespace DBase {
namespace Filters {

/// invoice type
enum InvoiceType {
  IT_DEPOSIT = 0, ///< depositing invoice
  IT_ACCOUNT = 1  ///< accounting invoice
};


class Invoice : virtual public Compound {
public:
  virtual ~Invoice() {
  }

  virtual Table& joinInvoiceTable() = 0;
  virtual Value<DBase::ID>& addId() = 0;
  virtual Value<DBase::ID>& addZoneId() = 0;
  virtual Value<int>& addType() = 0;
  virtual Value<std::string>& addNumber() = 0;
  virtual Interval<DBase::DateTimeInterval>& addCreateTime() = 0;
  virtual Interval<DBase::DateInterval>& addTaxDate() = 0;
  virtual Registrar& addRegistrar() = 0;
  virtual Object& addObject() = 0;
  virtual File& addFile() = 0;
};

class InvoiceImpl : virtual public Invoice {
public:
  InvoiceImpl();
  virtual ~InvoiceImpl();
  
  virtual Table& joinInvoiceTable();
  virtual Value<DBase::ID>& addId();
  virtual Value<DBase::ID>& addZoneId();
  virtual Value<int>& addType();
  virtual Value<std::string>& addNumber();
  virtual Interval<DBase::DateTimeInterval>& addCreateTime();
  virtual Interval<DBase::DateInterval>& addTaxDate();
  virtual Registrar& addRegistrar();
  virtual Object& addObject();
  virtual File& addFile();
};

}
}

#endif /*INVOICE_FILTER_H_*/
