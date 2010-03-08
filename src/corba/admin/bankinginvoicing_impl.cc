#include <corba/ccReg.hh>

#include "bankinginvoicing_impl.h"

#include "common.h"
#include "log/logger.h"
#include "log/context.h"
#include "register/bank_manager.h"

#include "corba/file_manager_client.h"
#include "corba/connection_releaser.h"

/*
bool
ccReg_BankingInvoicing_i::addBankAccount(
        const char *zoneName,
        const char *accountNumber,
        const char *accountName,
        const char *bankCode)
{
    ConnectionReleaser releaser;

    using namespace Register;
    FileManagerClient fm_client(ns_);
    File::ManagerPtr file_manager(File::Manager::create(&fm_client));
    Banking::ManagerPtr bank_manager(Banking::Manager::create(file_manager.get()));

    return bank_manager->insertBankAccount(zoneName, accountNumber, accountName, bankCode);
}

void 
ccReg_BankingInvoicing_i::archiveInvoices(
        bool send)
{
    ConnectionReleaser releaser;
    std::auto_ptr<Register::Invoicing::Manager> 
        invMan(Register::Invoicing::Manager::create());
    return invMan->archiveInvoices(send);
}
*/

bool
ccReg_BankingInvoicing_i::pairPaymentRegistrarId(
        CORBA::ULongLong paymentId,
        CORBA::ULongLong registrarId)
{
    /*
    ConnectionReleaser releaser;

    using namespace Register;
    FileManagerClient fm_client(ns_);
    File::ManagerPtr file_manager(File::Manager::create(&fm_client));

    Banking::ManagerPtr bank_manager(Banking::Manager::create(file_manager.get()));
    return bank_manager->manualCreateInvoice(paymentId, registrarId);
    */
    return true;
}

bool ccReg_BankingInvoicing_i::pairPaymentRegistrarHandle(
        CORBA::ULongLong paymentId,
        const char *registrarHandle)
{
    ConnectionReleaser releaser;

    using namespace Register;
    FileManagerClient fm_client(ns_);
    File::ManagerPtr file_manager(File::Manager::create(&fm_client));

    Banking::ManagerPtr bank_manager(Banking::Manager::create(file_manager.get()));
    return bank_manager->pairPaymentWithRegistrar(paymentId, registrarHandle);
}

bool ccReg_BankingInvoicing_i::setPaymentType(
        CORBA::ULongLong payment_id,
        CORBA::Short payment_type)
{
    ConnectionReleaser releaser;

    try {
        using namespace Register;
        FileManagerClient fm_client(ns_);
        File::ManagerPtr file_manager(File::Manager::create(&fm_client));

        Banking::ManagerPtr bank_manager(Banking::Manager::create(file_manager.get()));
        bank_manager->setPaymentType(payment_id, payment_type);
        return true;
    }
    catch (...) {
        return false;
    }
}

/*
bool
ccReg_BankingInvoicing_i::addPrefix(
        const char *zoneName,
        CORBA::Long type,
        CORBA::Long year,
        ccReg::TID prefix)
{
    ConnectionReleaser releaser;
    std::auto_ptr<Register::Invoicing::Manager> 
        invMan(Register::Invoicing::Manager::create());
    return invMan->insertInvoicePrefix(zoneName, type, year, prefix);
}

bool
ccReg_BankingInvoicing_i::pairInvoices()
{
    ConnectionReleaser releaser;
    std::auto_ptr<Register::Invoicing::Manager> 
        invMan(Register::Invoicing::Manager::create());
    return invMan->pairInvoices(false);
}

bool
ccReg_BankingInvoicing_i::createCreditInvoice(
        ccReg::TID zone,
        ccReg::TID registrar,
        const char *price,
        const char *taxdate,
        const char *crdate)
{
    ConnectionReleaser releaser;
    std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create());
    std::auto_ptr<Register::Invoicing::Invoice>
        invoice(invMan->createDepositInvoice());
    invoice->setZoneId(zone);
    invoice->setRegistrarId(registrar);
    invoice->setPrice(Database::Money(price));
    if (strlen(taxdate) != 0) {
        invoice->setTaxDate(Database::Date(taxdate));
    }
    if (strlen(crdate) != 0) {
        invoice->setCrDate(Database::DateTime(crdate));
    }
    return invoice->save();
}

bool
ccReg_BankingInvoicing_i::factoringOne(
        Register::Invoicing::Manager *manager,
        ccReg::TID zone,
        ccReg::TID registrar,
        const Database::Date &todate,
        const Database::Date &taxdate)
{
    std::auto_ptr<Register::Invoicing::Invoice>
        invoice(manager->createAccountInvoice());
    invoice->setZoneId(zone);
    invoice->setRegistrarId(registrar);
    invoice->setToDate(todate);
    invoice->setTaxDate(taxdate);
    return invoice->save();
}

bool
ccReg_BankingInvoicing_i::factoring(
        ccReg::TID zone,
        ccReg::TID registrar,
        const char *todate,
        const char *taxdate)
{
    ConnectionReleaser releaser;
    std::auto_ptr<Register::Invoicing::Manager>
        invMan(Register::Invoicing::Manager::create());
    Database::Date now(Database::NOW);
    Database::Date first_this(now.get().year(), now.get().month(), 1);
    Database::Date last_prev(first_this - Database::Days(1));

    Database::Date toDate;
    if (strlen(todate) == 0) {
        toDate = last_prev;
    } else {
        toDate = Database::Date(todate);
    }
    
    Database::Date taxDate;
    if (strlen(taxdate) == 0) {
        taxDate = first_this;
    } else {
        taxDate = Database::Date(taxdate);
    }

    if (registrar == 0) {
        Database::Connection conn = Database::Manager::acquire();
        Database::Query select;
        select.buffer()
            << "SELECT id FROM registrar ORDER BY id;";
        try {
            Database::Result res = conn.exec(select);
            int failed = 0;
            for (int i = 0; i < (int)res.size(); i++) {
                bool retval = factoringOne(invMan.get(), zone, res[i][0], toDate, taxDate);
                if (retval == false) {
                    failed++;
                }
            }
            if (failed != 0) {
                LOGGER(PACKAGE).warning( boost::format(
                            "ccReg_BankingInvoicing_i::factoring: %1% failures "
                            "(total: %2%)")
                        % failed % res.size());
            }
        } catch (...) {
            LOGGER(PACKAGE).error(
                    "ccReg_BankingInvoicing_i::factoring: an exception has occured.");
            return false;
        }
    } else {
        return factoringOne(invMan.get(), zone, registrar, toDate, taxDate);
    }
    return true;
}
bool ccReg_BankingInvoicing_i::addPrice(
        ccReg::TID zone,
        ccReg::BankingInvoicing::OperationType operation,
        const char *validfrom,
        const char *validto,
        const char *price,
        CORBA::Long period)
{
    ConnectionReleaser releaser;
    DB db;
    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create());
    Database::DateTime validFrom(Database::NOW_UTC);
    if (strlen(validfrom) != 0) {
        validFrom = Database::DateTime(validfrom);
    }
    Database::DateTime validTo;
    if (strlen(validto) != 0) {
        validTo = Database::DateTime(validto);
    }
    Database::Money p_price(price);
    Register::Zone::Operation op =
        ((operation == ccReg::BankingInvoicing::OT_CREATE) ? 
         Register::Zone::CREATE :
         Register::Zone::RENEW);
    zoneMan->addPrice(zone, op, validFrom, validTo,
            p_price, period);
    return true;
}
*/

