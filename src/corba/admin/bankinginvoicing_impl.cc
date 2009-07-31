#include <corba/ccReg.hh>

#include "bankinginvoicing_impl.h"

#include "common.h"
#include "log/logger.h"
#include "log/context.h"
#include "register/bank_manager.h"

bool
ccReg_BankingInvoicing_i::addBankAccount(
        const char *zoneName,
        const char *accountNumber,
        const char *accountName,
        const char *bankCode)
{
    std::auto_ptr<Register::Banking::Manager>
        bankMan(Register::Banking::Manager::create());
    return bankMan->insertBankAccount(zoneName, accountNumber, accountName, bankCode);
}

void 
ccReg_BankingInvoicing_i::archiveInvoices(
        bool send)
{
    std::auto_ptr<Register::Invoicing::Manager> 
        invMan(Register::Invoicing::Manager::create());
    return invMan->archiveInvoices(send);
}

bool
ccReg_BankingInvoicing_i::createInvoiceForPayment(
        unsigned long long paymentId,
        unsigned long long registrarId)
{
    std::auto_ptr<Register::Invoicing::Manager> 
        invMan(Register::Invoicing::Manager::create());
    return invMan->manualCreateInvoice(paymentId, registrarId);
}
bool
ccReg_BankingInvoicing_i::addPrefix(
        const char *zoneName,
        long type,
        long year,
        unsigned long long prefix)
{
    std::auto_ptr<Register::Invoicing::Manager> 
        invMan(Register::Invoicing::Manager::create());
    return invMan->insertInvoicePrefix(zoneName, type, year, prefix);
}

bool
ccReg_BankingInvoicing_i::pairInvoices()
{
    std::auto_ptr<Register::Invoicing::Manager> 
        invMan(Register::Invoicing::Manager::create());
    return invMan->pairInvoices(false);
}

bool
ccReg_BankingInvoicing_i::createCreditInvoice(
        unsigned long long zone,
        unsigned long long registrar,
        const char *price,
        const char *taxdate,
        const char *crdate)
{
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
        unsigned long long zone,
        unsigned long long registrar,
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
        unsigned long long zone,
        unsigned long long registrar,
        const char *todate,
        const char *taxdate)
{
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
        unsigned long long zone,
        ccReg::BankingInvoicing::OperationType operation,
        const char *validfrom,
        const char *validto,
        const char *price,
        long period)
{
    DB db;
    db.OpenDatabase(m_connection_string.c_str());
    std::auto_ptr<Register::Zone::Manager> zoneMan(
            Register::Zone::Manager::create(&db));
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
