#include "src/bin/corba/Admin.hh"

#include "src/bin/corba/admin/bankinginvoicing_impl.hh"

#include "src/bin/corba/admin/common.hh"
#include "src/util/log/logger.hh"
#include "src/util/log/context.hh"
#include "src/libfred/banking/bank_manager.hh"

#include "src/bin/corba/file_manager_client.hh"
#include "src/bin/corba/connection_releaser.hh"

/*
bool
ccReg_BankingInvoicing_i::addBankAccount(
        const char *zoneName,
        const char *accountNumber,
        const char *accountName,
        const char *bankCode)
{
    ConnectionReleaser releaser;

    using namespace LibFred;
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
    std::auto_ptr<LibFred::Invoicing::Manager>
        invMan(LibFred::Invoicing::Manager::create());
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

    using namespace LibFred;
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

    using namespace LibFred;
    FileManagerClient fm_client(ns_);
    File::ManagerPtr file_manager(File::Manager::create(&fm_client));

    Banking::ManagerPtr bank_manager(Banking::Manager::create(file_manager.get()));
    try
    {
        bank_manager->pairPaymentWithRegistrar(paymentId, registrarHandle);
    }
    catch (...)
    {
        return false;
    }
    return true;
}

bool ccReg_BankingInvoicing_i::setPaymentType(
        CORBA::ULongLong payment_id,
        CORBA::Short payment_type)
{
    ConnectionReleaser releaser;

    try {
        using namespace LibFred;
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

