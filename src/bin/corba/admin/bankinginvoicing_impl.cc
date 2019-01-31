#include "src/bin/corba/Admin.hh"

#include "src/bin/corba/admin/bankinginvoicing_impl.hh"

#include "src/bin/corba/admin/common.hh"
#include "util/log/logger.hh"
#include "util/log/context.hh"
#include "src/deprecated/libfred/banking/bank_manager.hh"

#include "src/bin/corba/file_manager_client.hh"
#include "src/bin/corba/connection_releaser.hh"

bool
ccReg_BankingInvoicing_i::pairPaymentRegistrarId(
        CORBA::ULongLong,
        CORBA::ULongLong)
{
    return false;
}

bool ccReg_BankingInvoicing_i::pairPaymentRegistrarHandle(
        CORBA::ULongLong,
        const char*)
{
    return false;
}

bool ccReg_BankingInvoicing_i::setPaymentType(
        CORBA::ULongLong,
        CORBA::Short)
{
    return false;
}

