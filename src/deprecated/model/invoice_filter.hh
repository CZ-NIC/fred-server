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
#ifndef INVOICE_FILTER_HH_4983047A67094228AA9068AC1D37383B
#define INVOICE_FILTER_HH_4983047A67094228AA9068AC1D37383B

#include "src/util/db/query/base_filters.hh"
#include "src/deprecated/model/object_filter.hh"
#include "src/deprecated/model/registrar_filter.hh"
#include "src/deprecated/model/file_filter.hh"
#include "src/deprecated/model/zone_filter.hh"

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
