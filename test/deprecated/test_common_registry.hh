
#include <string>
#include <exception>
#include <boost/date_time.hpp>
#include "src/util/types/money.hh"

#include "src/deprecated/libfred/registrar.hh"
#include "src/deprecated/libfred/invoicing/invoice.hh"

static std::string zone_registrar_credit_query (
        "SELECT credit FROM registrar_credit"
        " WHERE zone_id = $1::bigint AND registrar_id =$2::bigint");



Database::ID get_zone_cz_id();
LibFred::Registrar::Registrar::AutoPtr createTestRegistrarClass();
//LibFred::Registrar::Registrar::AutoPtr createTestRegistrarClassNoCz(const std::string &varsymb);
Database::ID createTestRegistrar();

bool check_std_exception_invoice_prefix(std::exception const & ex);
bool check_std_exception_out_of_range(std::exception const & ex);
bool check_std_exception_archiveInvoices(std::exception const & ex);
bool check_std_exception_createAccountInvoice(std::exception const & ex);
bool check_std_exception_billing_fail(std::exception const & ex);
bool check_dummy(std::exception const & ex);

void try_insert_invoice_prefix();
Money getOperationPrice(unsigned op, Database::ID zone_id, unsigned requested_quantity);

const Decimal get_credit(Database::ID reg_id, Database::ID zone_id);

void get_vat(int &vat_percent, std::string &vat_koef, date taxdate = day_clock::local_day());

void insert_poll_request_fee(Database::ID reg_id,
        Decimal price,
        date poll_from = date(),
        date poll_to   = date()
        );
