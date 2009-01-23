/*
 *  Copyright (C) 2007, 2008, 2009  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _INVOICE_H_
#define _INVOICE_H_

#include "common_object.h"
#include "object.h"
#include "db/database.h"
#include "model/model_filters.h"
#include "file.h"
#include "mailer.h"
#include "documents.h"

namespace Register {
namespace Invoicing {

typedef long long Money;

enum MemberType {
  MT_CRDATE,
  MT_NUMBER,
  MT_REGISTRAR,
  MT_TOTAL,
  MT_CREDIT,
  MT_TYPE,
  MT_ZONE
};

enum Type {
  IT_DEPOSIT, ///< depositing invoice
  IT_ACCOUNT, ///< accounting invoice
  IT_NONE

};
std::string Type2Str(Type type);
int Type2SqlType(Type type);

enum PaymentActionType {
    PAT_CREATE_DOMAIN,
    PAT_RENEW_DOMAIN
};
std::string PaymentActionType2Str(PaymentActionType type);

class Subject {
protected:
    virtual ~Subject()
    { }
public:
    virtual Database::ID getId() const = 0;
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

class Payment {
protected:
    virtual ~Payment()
    { }
public:
    virtual Database::Money getPrice() const = 0;
    virtual unsigned int getVatRate() const = 0;
    virtual Database::Money getVat() const = 0;
    virtual Database::Money getPriceWithVat() const = 0;
};

class PaymentSource:
    virtual public Payment {
protected:
    virtual ~PaymentSource()
    { }
public:
    virtual Database::ID getId() const = 0;
    virtual unsigned long long getNumber() const = 0;
    virtual Database::Money getCredit() const = 0;
    virtual Database::Money getTotalPrice() const = 0;
    virtual Database::Money getTotalVat() const = 0;
    virtual Database::Money getTotalPriceWithVat() const = 0;
    virtual Database::DateTime getCrTime() const = 0;
};

class PaymentAction:
    virtual public Payment {
protected:
    virtual ~PaymentAction()
    { }
public:
    virtual Database::ID getObjectId() const = 0;
    virtual const std::string &getObjectName() const = 0;
    virtual Database::DateTime getActionTime() const = 0;
    virtual Database::Date getExDate() const = 0;
    virtual PaymentActionType getAction() const = 0;
    virtual unsigned int getUnitsCount() const = 0;
    virtual Database::Money getPricePerUnit() const = 0;
};

class AnnualPartitioning:
    virtual public Payment {
protected:
    virtual ~AnnualPartitioning()
    { }
public:
    virtual void resetIterator(unsigned vatRate) = 0;
    virtual bool end() const = 0;
    virtual void next() = 0;
    virtual unsigned int getYear() const = 0;
};

class Invoice:
    virtual public Register::CommonObject {
public:
    virtual Database::ID getZone() const = 0;
    virtual const std::string &getZoneName() const = 0;
    virtual Database::DateTime getCrTime() const = 0;
    virtual Database::Date getTaxDate() const = 0;
    virtual Database::Date getToDate() const = 0;
    virtual Database::DateInterval getAccountPeriod() const = 0;
    virtual unsigned long long getNumber() const = 0;
    virtual Database::ID getRegistrar() const = 0;
    virtual std::string getRegistrarName() const = 0;
    virtual Database::Money getCredit() const = 0;
    virtual Database::Money getPrice() const = 0;
    virtual int getVatRate() const = 0;
    virtual Database::Money getTotal() const = 0;
    virtual Database::Money getTotalVAT() const = 0;
    virtual const std::string& getVarSymbol() const = 0;
    virtual Database::ID getFilePDF() const = 0;
    virtual Database::ID getFileXML() const = 0;
    virtual std::string getFileNamePDF() const = 0;
    virtual std::string getFileNameXML() const = 0;
    virtual Type getType() const = 0;
    virtual Database::ID getInvoicePrefixTypeId() const = 0;
    virtual std::string getLastError() const = 0;
    virtual std::vector<std::string> getErrors() const = 0;

    // XXX 
    virtual bool domainBilling(Database::ID zone, Database::ID registrar,
            Database::ID objectId,
            Database::Date exDate, int units_count) = 0;
    virtual std::vector<Database::Money> countPayments(
            std::vector<Database::Money> credit, Database::Money price) = 0;
    // virtual void addAction(PaymentActionType action, Database::ID objectId,
            // Database::Date exDate = Database::Date(), unsigned int unitsCount = 0) = 0;

    virtual void setId(Database::ID id) = 0;
    virtual void setZone(Database::ID id) = 0;
    virtual void setZoneName(std::string zoneName) = 0;
    virtual void setCrTime(Database::DateTime crDate) = 0;
    virtual void setCrTime(std::string crDate) = 0;
    virtual void setTaxDate(Database::Date taxDate) = 0;
    virtual void setTaxDate(std::string taxDate) = 0;
    virtual void setToDate(Database::Date toDate) = 0;
    virtual void setToDate(std::string toDate) = 0;
    virtual void setNumber(unsigned long long number) = 0;
    virtual void setRegistrar(Database::ID registrar) = 0;
    virtual void setRegistrarName(std::string registrar) = 0;
    virtual void setCredit(Database::Money credit) = 0;
    virtual void setPrice(Database::Money price) = 0;
    virtual void setVATRate(int vatRate) = 0;
    virtual void setTotal(Database::Money total) = 0;
    virtual void setTotalVAT(Database::Money totalVat) = 0;
    virtual void setType(Type type) = 0;
    virtual void setFileIdPDF(Database::ID id) = 0;
    virtual void setFileIdXML(Database::ID id) = 0;
    virtual void setInvoicePrefixTypeId(Database::ID id) = 0;
    virtual void setAccountPeriod(Database::DateInterval period) = 0;

    virtual unsigned int getSourceCount() const = 0;
    virtual const PaymentSource *getSource(unsigned int index) const = 0;
    virtual unsigned int getActionCount() const = 0;
    virtual const PaymentAction *getAction(unsigned int index) const = 0;
    virtual AnnualPartitioning *getAnnualPartitioning() = 0;
    virtual unsigned int getPaymentCount() const = 0;
    virtual const Payment *getPayment(unsigned int index) const = 0;

    virtual const Subject *getClient() const = 0;
    virtual const Subject *getSupplier() const = 0;

    virtual bool save() = 0;
};

class List:
    virtual public Register::CommonList {
public:
    virtual Invoice *get(unsigned int index) const = 0;
    virtual void reload(Database::Filters::Union &filter,
            Database::Manager *dbman = NULL) = 0;
    virtual void sort(MemberType member, bool asc) = 0;

    virtual const char *getTempTableName() const = 0;
    virtual void makeQuery(bool, bool, std::stringstream &) const = 0;
    virtual void reload() = 0;
    virtual void setPartialLoad(bool partialLoad) = 0;
    virtual void exportXML(std::ostream &out) = 0;
};

class Exporter {
public:
    virtual ~Exporter() 
    { }
    virtual void doExport(Invoice *) = 0;
};

class Manager {
public:
    virtual List *createList() const = 0;
    static Manager *create(
            Database::Manager *dbMan,
            Document::Manager *docMan = NULL,
            Mailer::Manager *mailMan = NULL);
    virtual void archiveInvoices(bool send) const = 0;
    virtual Database::Money countVat(Database::Money price,
            unsigned int vatRate, bool base) = 0;
    virtual Invoice *createAccountInvoice() = 0;
    virtual Invoice *createDepositInvoice() = 0;
    virtual Invoice *createInvoice(Type type) = 0;
    virtual Database::Money getCreditByZone(
            const std::string &registrarHandle, Database::ID zoneId) const = 0;
};

} // namespace Invoicing
} // namespace Register

#endif // _INVOICE_H_
