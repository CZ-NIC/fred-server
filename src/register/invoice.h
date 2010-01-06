#ifndef _INVOICE2_H_
#define _INVOICE2_H_

#include "common_impl_new.h"
#include "db_settings.h"
#include "model_invoice.h"
#include "invoice_common.h"
#include "invoice_exporter.h"
#include "old_utils/dbsql.h"

namespace Register {
namespace Invoicing {

enum Type {
  IT_DEPOSIT, ///< depositing invoice
  IT_ACCOUNT, ///< accounting invoice
  IT_NONE

};
std::string Type2Str(Type type);
int Type2SqlType(Type type);

class Manager;

//-----------------------------------------------------------------------------
//
// Invoice
//
//-----------------------------------------------------------------------------
class Invoice:
    public ModelInvoice,
    virtual public Register::CommonObjectImplNew {
private:
    Manager     *m_manager;
    Type        m_type;
    int         m_vatRate;
    bool        m_storeFileFlag;
    Subject     m_client;
    static Subject     m_supplier;
    std::string m_varSymbol;
    std::string m_zoneFqdn;
    std::string m_fileHandle;
    std::string m_fileXmlHandle;

    AnnualPartitioning              m_annualPartitioning;
    std::vector<PaymentSource *>    m_sources;
    std::vector<PaymentAction *>    m_actions;
    std::vector<Payment>            m_paid;
    std::vector<std::string>        m_errors;

    Database::Date  m_fromDate;
    Database::Date  m_toDate;
    Database::DateInterval  m_accountPeriod;

    void addError(const std::string &str);
    void addError(const boost::format &frmt);
    void clearActions();
    Database::Money getPrice2(
            Database::PSQLConnection *conn,
            PaymentActionType operation,
            const int &period);
    bool isRegistrarSystem(
            Database::PSQLConnection *conn,
            const Database::ID &registrar);
    std::vector<Database::Money> countPayments( 
            const std::vector<Database::Money> &credit, 
            const Database::Money &price);
    Database::ID newInvoiceObjectRegistry(Database::PSQLConnection *conn);
    bool newInvoiceObjectRegistryPriceMap(
            Database::PSQLConnection *conn,
            const Database::ID &invObjRegId,
            const Database::ID &invoiceId,
            const Database::Money &price);
    bool updateInvoiceCredit(
            Database::PSQLConnection *conn, 
            const Database::ID &invoiceId,
            const Database::Money &price);
    bool getCreditInvoicesId(
            Database::PSQLConnection *conn,
            std::vector<Database::ID> &vec_id,
            std::vector<Database::Money> &vec_money,
            const int &limit = 2);
    bool domainBilling(Database::PSQLConnection *conn);
    Database::Date getFromDateFromDB();
    int getRecordsCount();
    bool getRecordsPrice();
    int getSystemVAT();
    bool isYearInInvoicePrefix(const int &year);
    bool createNewYearInInvoicePrefix(const int &newYear);
    bool createNewYearInInvoicePrefix(const int &newYear, const int &oldYear);
    bool getInvoicePrefix();
    bool createNewInvoice();
    bool updateInvoiceObjectRegistry();
    bool updateRegistrarInvoice();
    std::vector<int> getAccountInvoicesNumbers();
    Database::Money getInvoiceSumPrice(const int &accountInvoiceId);
    Database::Money getInvoiceBalance(
            const int &accountInvoiceId,
            const Database::Money &invoiceSumPrice);
    bool updateInvoiceCreditPaymentMap(const std::vector<int> &idNumbers);
    bool testRegistrar();
    bool testRegistrarPrivileges();
    bool testZone();
    bool insertAccount();
    bool insertDeposit();
    bool updateAccount();
    bool updateDeposit();
    // bool update();
public:
    Invoice(
            Manager *manager):
        CommonObjectImplNew(),
        ModelInvoice(),
        m_manager(manager),
        m_type(IT_NONE),
        m_vatRate(-1),
        m_storeFileFlag(false)
    { }
    ~Invoice();
    bool domainBilling(
            DB *db, 
            const Database::ID &zone,
            const Database::ID &registrar,
            const Database::ID &objectId,
            const Database::Date &exDate,
            const int &units_count);
    bool domainBilling(
            Database::PSQLConnection *conn,
            const Database::ID &zone,
            const Database::ID &registrar,
            const Database::ID &objectId,
            const Database::Date &exDate,
            const int &units_count);
    bool save();
    void setType(Type type);
    Type getType() const;
    void setFromDate(const Database::Date &fromDate);
    void setFromDate(const std::string &fromDate);
    void setToDate(const Database::Date &toDate);
    void setToDate(const std::string &toDate);
    void setTaxDate(const Database::Date &taxDate);
    void setTaxDate(const std::string &taxDate);
    Database::Date getFromDate() const;
    Database::Date getToDate() const;
    const Database::DateInterval &getAccountPeriod() const;
    void setAccountPeriod(const Database::DateInterval &period);
    void setVarSymbol(const std::string &varSymbol);
    const std::string &getVarSymbol() const;
    const std::string &getZoneFqdn() const;
    void setZoneFqdn(std::string &zoneFqdn);
    const Subject *getClient() const;
    void setClient(Subject &client);
    const Subject *getSupplier() const;
    void addSource(PaymentSource *source);
    void addAction(PaymentAction *action);
    unsigned int getPaymentCount() const;
    const Payment *getPayment(const unsigned int &index) const;
    unsigned int getSourceCount() const;
    const PaymentSource *getSource(const unsigned int &index) const;
    unsigned int getActionCount() const;
    const PaymentAction *getAction(const unsigned int &index) const;
    AnnualPartitioning *getAnnualPartitioning();
    const std::string &getFileHandle() const;
    const std::string &getFileXmlHandle() const;
    void setFileHandle(std::string &fileHandle);
    void setFileXmlHandle(std::string &fileXmlHandle);

    void doExport(Exporter *exp);
    std::string getLastError() const;
    std::vector<std::string> getErrors() const;
    void setFile(const Database::ID &filePDF, const Database::ID &fileXML);
}; // class Invoice

COMPARE_CLASS_IMPL_NEW(Invoice, ZoneId);
COMPARE_CLASS_IMPL_NEW(Invoice, CrDate);
COMPARE_CLASS_IMPL_NEW(Invoice, TaxDate);
COMPARE_CLASS_IMPL_NEW(Invoice, ToDate);
COMPARE_CLASS_IMPL_NEW(Invoice, FromDate);
COMPARE_CLASS_IMPL_NEW(Invoice, Prefix);
COMPARE_CLASS_IMPL_NEW(Invoice, RegistrarId);
COMPARE_CLASS_IMPL_NEW(Invoice, Credit);
COMPARE_CLASS_IMPL_NEW(Invoice, Price);
COMPARE_CLASS_IMPL_NEW(Invoice, VarSymbol);
// COMPARE_CLASS_IMPL(Invoice, FilePDF);
// COMPARE_CLASS_IMPL(Invoice, FileXML);
COMPARE_CLASS_IMPL_NEW(Invoice, FileId);
COMPARE_CLASS_IMPL_NEW(Invoice, FileXmlId);
COMPARE_CLASS_IMPL_NEW(Invoice, Total);
COMPARE_CLASS_IMPL_NEW(Invoice, Type);



} // namespace Invoicing
} // namespace Register

#endif // _INVOICE2_H_
