#include "src/bin/corba/Admin.hh"

#include "src/bin/corba/admin/bankinginvoicing_impl.hh"

#include "src/bin/corba/admin/common.hh"
#include "src/util/log/logger.hh"
#include "src/util/log/context.hh"
#include "src/libfred/banking/bank_manager.hh"

#include "src/bin/corba/file_manager_client.hh"
#include "src/bin/corba/connection_releaser.hh"

bool
ccReg_BankingInvoicing_i::pairPaymentRegistrarId(
        CORBA::ULongLong paymentId,
        CORBA::ULongLong registrarId)
{
    return false;
}

bool ccReg_BankingInvoicing_i::pairPaymentRegistrarHandle(
        CORBA::ULongLong paymentId,
        const char *registrarHandle)
{
    return false;
}

bool ccReg_BankingInvoicing_i::setPaymentType(
        CORBA::ULongLong payment_id,
        CORBA::Short payment_type)
{
    return false;
}

