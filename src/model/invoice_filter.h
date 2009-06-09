#ifndef INVOICE_FILTER_H_
#define INVOICE_FILTER_H_

#include "db/query/base_filters.h"
#include "object_filter.h"
#include "registrar_filter.h"
#include "file_filter.h"
#include "zone_filter.h"

namespace Database {
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
  virtual Value<Database::ID>& addId() = 0;
  virtual Value<Database::ID>& addZoneId() = 0;
  virtual Value<int>& addType() = 0;
  virtual Value<std::string>& addNumber() = 0;
  virtual Value<Database::ID>& addFileXML() = 0;
  virtual Value<Database::ID>& addFilePDF() = 0;
  virtual Interval<Database::DateTimeInterval>& addCreateTime() = 0;
  virtual Interval<Database::DateInterval>& addTaxDate() = 0;
  virtual Registrar& addRegistrar() = 0;
  virtual Object& addObject() = 0;
  virtual File& addFile() = 0;
  virtual Zone &addZone() = 0;
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Compound);
  }

  static Invoice* create();
};

class InvoiceImpl : virtual public Invoice {
public:
  InvoiceImpl();
  virtual ~InvoiceImpl();
  
  virtual Table& joinInvoiceTable();
  virtual Value<Database::ID>& addId();
  virtual Value<Database::ID>& addZoneId();
  virtual Value<int>& addType();
  virtual Value<std::string>& addNumber();
  virtual Value<Database::ID>& addFileXML();
  virtual Value<Database::ID>& addFilePDF();
  virtual Interval<Database::DateTimeInterval>& addCreateTime();
  virtual Interval<Database::DateInterval>& addTaxDate();
  virtual Registrar& addRegistrar();
  virtual Object& addObject();
  virtual File& addFile();
  virtual Zone &addZone();
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Invoice);
  }
};

}
}

#endif /*INVOICE_FILTER_H_*/
