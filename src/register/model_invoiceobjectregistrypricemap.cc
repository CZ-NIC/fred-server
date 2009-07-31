#include "model_invoiceobjectregistrypricemap.h"

std::string ModelInvoiceObjectRegistryPriceMap::table_name = "invoice_object_registry_price_map";

// DEFINE_FOREIGN_KEY(ModelInvoiceObjectRegistryPriceMap, ModelInvoiceObjectRegistry, unsigned long long, invoiceObjectRegistryId, m_invoiceObjectRegistryId, table_name, "id", id, .setNotNull())
DEFINE_PRIMARY_KEY(ModelInvoiceObjectRegistryPriceMap, unsigned long long, invoiceObjectRegistryId, m_invoiceObjectRegistryId, table_name, "id", .setNotNull())
DEFINE_FOREIGN_KEY(ModelInvoiceObjectRegistryPriceMap, ModelInvoice, unsigned long long, invoiceId, m_invoiceId, table_name, "invoiceid", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoiceObjectRegistryPriceMap, Database::Money, price, m_price, table_name, "price", .setNotNull().setDefault())

// DEFINE_ONE_TO_ONE(ModelInvoiceObjectRegistryPriceMap, ModelInvoiceObjectRegistry, invoiceObjectRegistry, m_invoiceObjectRegistry, unsigned long long, invoiceObjectRegistryId, m_invoiceObjectRegistryId)
DEFINE_ONE_TO_ONE(ModelInvoiceObjectRegistryPriceMap, ModelInvoice, invoice, m_invoice, unsigned long long, invoiceId, m_invoiceId)

ModelInvoiceObjectRegistryPriceMap::field_list ModelInvoiceObjectRegistryPriceMap::fields = list_of<ModelInvoiceObjectRegistryPriceMap::field_list::value_type>
    (&ModelInvoiceObjectRegistryPriceMap::invoiceObjectRegistryId)
    (&ModelInvoiceObjectRegistryPriceMap::invoiceId)
    (&ModelInvoiceObjectRegistryPriceMap::price);
