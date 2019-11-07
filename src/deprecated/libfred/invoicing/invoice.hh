/*
 * Copyright (C) 2007-2019  CZ.NIC, z. s. p. o.
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
#ifndef INVOICE_HH_5ED6D3B6BFD449BD96BF4989DDF9A788
#define INVOICE_HH_5ED6D3B6BFD449BD96BF4989DDF9A788

#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/time_period.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>


#include "src/deprecated/libfred/common_object.hh"
#include "src/deprecated/libfred/object.hh"
#include "libfred/types.hh"
#include "src/deprecated/libfred/exceptions.hh"
#include "src/deprecated/libfred/documents.hh"
#include "libfred/mailer.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"
#include "util/decimal/decimal.hh"
#include "src/util/types/money.hh"

namespace LibFred {
namespace Invoicing {

// input is price in cents(penny)
// signed type allows negative amounts
//typedef unsigned long cent_amount;



// TODO copy of those in action.h
// mapped from enum_operation
enum  {
    INVOICING_DomainCreate = 1,
    INVOICING_DomainRenew = 2,
    INVOICING_GeneralOperation = 3
};

/// Member identification (i.e. for sorting) 
enum MemberType {
  MT_CRTIME,
  MT_CRDATE,
  MT_NUMBER,
  MT_REGISTRAR,
  MT_TOTAL,
  MT_CREDIT,
  MT_TYPE,
  MT_ZONE,
  MT_PRICE
};

/// invoice type
// duplicity from ccReg.h
enum Type {
  IT_DEPOSIT, ///< depositing invoice
  IT_ACCOUNT, ///< accounting invoice
  IT_NONE
};
std::string Type2Str(Type _type);

/// subject of invoicing (supplier or client)
class Subject {
protected:
  virtual ~Subject() {
  }
public:
  virtual TID getId() const = 0;
  virtual const std::string& getHandle() const = 0;
  virtual const std::string& getName() const = 0;
  virtual const std::string& getFullname() const = 0;
  virtual const std::string& getStreet() const = 0;
  virtual const std::string& getCity() const = 0;
  virtual const std::string& getZip() const = 0;
  virtual const std::string& getCountry() const = 0;
  virtual const std::string& getICO() const = 0;
  virtual const std::string& getVatNumber() const = 0;
  virtual bool getVatApply() const = 0;
  virtual const std::string& getRegistration() const = 0;
  virtual const std::string& getReclamation() const = 0;
  virtual const std::string& getEmail() const = 0;
  virtual const std::string& getURL() const = 0;
  virtual const std::string& getPhone() const = 0;
  virtual const std::string& getFax() const = 0;
};
/// description of any payment transfer with vat
class Payment {
protected:
  virtual ~Payment() {
  }
public:
  // base tax money amount  
  virtual Money getPrice() const = 0;
  // vat rate in percents
  virtual Decimal getVatRate() const = 0;
  // counted vat (approximatly price * vatRate / 100)
  virtual Money getVat() const = 0;
  // sum of price and vat
  virtual Money getPriceWithVat() const = 0;
};
class PaymentSource : virtual public Payment {
protected:
  virtual ~PaymentSource() {
  }
public:
  virtual TID getId() const = 0;
  /// number of source advance invoice 
  virtual unsigned long long getNumber() const = 0;
  /// remaining credit on source invoice
  virtual Money getCredit() const = 0;
  /// total price withput vat of source invoice
  virtual Money getTotalPrice() const = 0;
  /// total vat of source invoice
  virtual Money getTotalVat() const = 0;
  /// total price with vat of source invoice
  virtual Money getTotalPriceWithVat() const = 0;
  /// creation time of source invoice
  virtual boost::posix_time::ptime getCrTime() const = 0;
};

//this relies on PaymentActionType (operation_id - 1) in InvoiceImpl::addAction
enum PaymentActionType {
  PAT_CREATE_DOMAIN
  , PAT_RENEW_DOMAIN
  , PAT_REQUESTS_OVER_LIMIT
  , PAT_FINE
  , PAT_FEE
};
std::string PaymentActionType2Str(PaymentActionType type);

class PaymentAction : virtual public Payment {
protected:
  virtual ~PaymentAction() {
  }
public:
  /// id of object on which action was performed
  virtual TID getObjectId() const = 0;
  /// name of object on which action was performed
  virtual const std::string& getObjectName() const = 0;
  /// time of action 
  virtual boost::posix_time::ptime getActionTime() const = 0;
  /// fromdate when operation started otherwise (not-a-date)
  virtual boost::gregorian::date getFromDate() const = 0;
  /// exdate when operation was renew otherwise (not-a-date)
  virtual boost::gregorian::date getExDate() const = 0;
  /// type of action (creation or registration extension)
  virtual PaymentActionType getAction() const = 0;
  /// number of yearth for extenstion
  virtual unsigned getUnitsCount() const = 0;
  /// price (without vat) per unit
  virtual Money getPricePerUnit() const = 0;

  //// new interface - implement
  virtual std::string getActionStr() const = 0;
};
class Invoice : virtual public LibFred::CommonObject {
public:
  virtual ~Invoice() {
  }
  virtual const Subject* getSupplier() const = 0;
  virtual const Subject* getClient() const = 0;
  virtual TID getZone() const = 0;
  virtual const std::string& getZoneName() const = 0;
  virtual boost::posix_time::ptime getCrTime() const = 0;
  virtual boost::gregorian::date getTaxDate() const = 0;
  virtual boost::gregorian::date_period getAccountPeriod() const = 0;
  virtual Type getType() const = 0;
  virtual unsigned long long getNumber() const = 0;
  virtual TID getRegistrar() const = 0;
  virtual Money getCredit() const = 0;
  virtual Money getPrice() const = 0;
  virtual Decimal getVatRate() const = 0;
  virtual Money getTotal() const = 0;
  virtual Money getTotalVAT() const = 0;
  virtual const std::string& getVarSymbol() const = 0;
  virtual TID getFilePDF() const = 0;
  virtual TID getFileXML() const = 0;
  virtual std::string getFileNamePDF() const = 0;
  virtual std::string getFileNameXML() const = 0;
  virtual unsigned getSourceCount() const = 0;
  virtual const PaymentSource *getSource(unsigned idx) const = 0;
  virtual unsigned getActionCount() const = 0;
  virtual const PaymentAction *getAction(unsigned idx) const = 0;
  virtual unsigned getPaymentCount() const = 0;
  virtual const Payment *getPaymentByIdx(unsigned idx) const = 0;

    // //////////////////// new interface (partially from model_invoices)
    
  
    virtual TID getZoneId() const = 0;
    virtual boost::posix_time::ptime getCrDate() const = 0;
    // const Database::DateTime getCrDate() const = 0;
    // const Database::Date getTaxDate() const = 0;
    virtual TID getPrefix() const = 0;
    virtual TID getRegistrarId() const = 0;
    virtual Money getVat() const = 0;
    virtual Money getTotalVat() const = 0;
    virtual TID getFileId() const = 0;
    virtual std::string getFileHandle() const = 0;
    virtual TID getFileXmlId() const = 0;
    virtual std::string getFileXmlHandle() const = 0;
    virtual std::string getZoneFqdn() const = 0;

    virtual const Payment *getPayment(const unsigned int &index) const = 0;
};

class List : virtual public LibFred::CommonList {
public:
  /// publice destructor
  virtual ~List() {
  }
  ;
  /// type for setting archive filter
  enum ArchiveFilter
  {
    AF_IGNORE, ///< ignore this filter (NULL-value)
    AF_SET, ///< invoice is archived
    AF_UNSET ///< invoice is still unarchived
  };
  
  virtual const char* getTempTableName() const = 0;
  /// reload invoices with selected filter
  virtual void reload() = 0;
  /// new reload method
  virtual void reload(Database::Filters::Union& _uf) = 0;
  /// subsequential reload will not load any action 
  virtual void setPartialLoad(bool partialLoad) = 0;
  /// return invoice by index
  virtual Invoice *get(unsigned _idx) const = 0;
  /// clear filter settings
  virtual void clearFilter() = 0;
  /// xml output of all invoices in list
  virtual void exportXML(std::ostream& out) = 0;
  /// sort by column
  virtual void sort(MemberType _member, bool _asc) = 0;

  /// filter for invoice id
  virtual void setIdFilter(TID id) = 0;
  /// filter for registrar recieving invoice
  virtual void setRegistrarFilter(TID registrar) = 0;
  /// filter for registrar recieving invoice by handle
  virtual void setRegistrarHandleFilter(const std::string& handle) = 0;
  /// filter for id of associated zone 
  virtual void setZoneFilter(TID zone) = 0;
  /// filter for invoice type (advance=1 or normal=2)
  virtual void setTypeFilter(unsigned type) = 0;
  /// filter for variable symbol of payment
  virtual void setVarSymbolFilter(const std::string& varSymbol) = 0;
  /// filter for invoice number
  virtual void setNumberFilter(const std::string& number) = 0;
  /// filter for crDate
  virtual void
      setCrDateFilter(const boost::posix_time::time_period& crDatePeriod) = 0;
  /// filter for taxDate
  virtual void
      setTaxDateFilter(const boost::posix_time::time_period& taxDatePeriod) = 0;
  /// filter for existance of archived file
  virtual void setArchivedFilter(ArchiveFilter archive) = 0;
  /// filter for object attached to invoice by id
  virtual void setObjectIdFilter(TID objectId) = 0;
  /// filter for object attached to invoice by name
  virtual void setObjectNameFilter(const std::string& objectName) = 0;
  /// filter for account invoices with selected advance invoice source
  virtual void setAdvanceNumberFilter(const std::string& number) = 0;
  
  /// from CommonList; propably will be removed in future
  virtual void makeQuery(bool, bool, std::stringstream&) const = 0;  

  virtual unsigned int getSize() const = 0;

  };

typedef std::vector<unsigned long long> InvoiceIdVect;//exported invoices for test

//unpaid invoices
struct unpaid_account_invoice
{
    unsigned long long id;
    Money balance;
    Decimal vat;
};

/// facade of invoicing subsystem
class Manager {
public:
  /// public destructor - client is responsible for destroying manager
  virtual ~Manager() {
  }
  /// find unarchived invoices and archive then in PDF format
  virtual InvoiceIdVect archiveInvoices(bool send, InvoiceIdVect archive_only_this_if_set = InvoiceIdVect()) = 0;
  /// create empty list of invoices
  virtual List* createList() = 0;
  /// return credit for registrar by zone
  virtual std::string getCreditByZone(const std::string& registrarHandle,
                                TID zone) = 0;
  /// factory method
   static Manager *create(
                         Document::Manager *_doc_manager,
                         Mailer::Manager *_mail_manager);

  static Manager *create();

  virtual bool insertInvoicePrefix(unsigned long long zoneId,
          int type, int year, unsigned long long prefix) = 0;
  virtual bool insertInvoicePrefix(const std::string &zoneName,
          int type, int year, unsigned long long prefix) = 0;
  virtual void createInvoicePrefixes(bool for_current_year) = 0;

  virtual void addInvoiceNumberPrefix( unsigned long prefix
          , const std::string& zone_fqdn
          , const std::string invoice_type_name) = 0;

  // added methods
  virtual  unsigned long long  createDepositInvoice(boost::gregorian::date tax_date, unsigned long long zoneId, unsigned long long registrarId, Money price, boost::posix_time::ptime invoice_date, Money& out_credit) = 0;

  virtual bool charge_operation_auto_price(
            const std::string& operation
            , unsigned long long zone_id
            , unsigned long long registrar_id
            , unsigned long long object_id
            , boost::posix_time::ptime crdate //local timestamp
            , boost::gregorian::date date_from //local date
            , boost::gregorian::date date_to //local date
            , Decimal quantity) = 0;

  virtual bool chargeRequestFee(
          const Database::ID &registrar_id,
          date poll_msg_period_to) = 0;

  virtual void createAccountInvoices(
          const std::string& zone_fqdn
          , boost::gregorian::date taxdate
          , boost::gregorian::date fromdate
          , boost::gregorian::date todate
          , boost::posix_time::ptime invoicedate) = 0;
  virtual void createAccountInvoice(
          const std::string& registrarHandle
          , const std::string& zone_fqdn
          , boost::gregorian::date taxdate
          , boost::gregorian::date fromdate
          , boost::gregorian::date todate
          , boost::posix_time::ptime invoicedate) = 0;

  virtual std::vector<unpaid_account_invoice> find_unpaid_account_invoices(
          unsigned long long registrar_id
          , unsigned long long zone_id) = 0;

  virtual Money lower_account_invoice_balance_by_paid_amount(Money paid
          , Decimal invoice_vat_rate
          , unsigned long long invoice_id) = 0;

  virtual Money zero_account_invoice_balance(unsigned long long invoice_id) = 0;

}; // Manager

std::string getRequestUnitPrice(unsigned zone_id);
void getRequestFeeParams(unsigned *zone_id, unsigned *base_free_count = NULL, unsigned *per_domain_free_count = NULL);

}
; // Invoicing
}
; // Fred

#endif
