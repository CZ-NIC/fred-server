#include "model_invoicecreditpaymentmap.h"

std::string ModelInvoiceCreditPaymentMap::table_name = "invoice_credit_payment_map";

DEFINE_PRIMARY_KEY(ModelInvoiceCreditPaymentMap, unsigned long long, invoiceId, m_invoiceId, table_name, "invoiceid", .setNotNull())
DEFINE_FOREIGN_KEY(ModelInvoiceCreditPaymentMap, ModelInvoice, unsigned long long, advanceInvoiceId, m_advanceInvoiceId, table_name, "ainvoiceid", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoiceCreditPaymentMap, Database::Money, credit, m_credit, table_name, "credit", .setNotNull().setDefault())
DEFINE_BASIC_FIELD(ModelInvoiceCreditPaymentMap, Database::Money, balance, m_balance, table_name, "balance", .setNotNull().setDefault())

DEFINE_ONE_TO_ONE(ModelInvoiceCreditPaymentMap, ModelInvoice, advanceInvoice, m_advanceInvoice, unsigned long long, advanceInvoiceId, m_advanceInvoiceId)

ModelInvoiceCreditPaymentMap::field_list ModelInvoiceCreditPaymentMap::fields = list_of<ModelInvoiceCreditPaymentMap::field_list::value_type>
    (&ModelInvoiceCreditPaymentMap::invoiceId)
    (&ModelInvoiceCreditPaymentMap::advanceInvoiceId)
    (&ModelInvoiceCreditPaymentMap::credit)
    (&ModelInvoiceCreditPaymentMap::balance);

