#include "model_invoiceprefix.h"

std::string ModelInvoicePrefix::table_name = "invoice_prefix";

DEFINE_PRIMARY_KEY(ModelInvoicePrefix, unsigned long long, id, m_id, table_name, "id", .setDefault())
//DEFINE_FOREIGN_KEY(ModelInvoicePrefix, ModelZone, unsigned long long, zoneId, m_zoneId, table_name, "zone", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoicePrefix, int, type, m_type, table_name, "typ", .setDefault())
DEFINE_BASIC_FIELD(ModelInvoicePrefix, int, year, m_year, table_name, "year", .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoicePrefix, unsigned long long, prefix, m_prefix, table_name, "prefix", )

//DEFINE_ONE_TO_ONE(ModelInvoicePrefix, ModelZone, zone, m_zone, unsigned long long, zoneId, m_zoneId)

ModelInvoicePrefix::field_list ModelInvoicePrefix::fields = list_of<ModelInvoicePrefix::field_list::value_type>
    (&ModelInvoicePrefix::id)
    //(&ModelInvoicePrefix::zoneId)
    (&ModelInvoicePrefix::type)
    (&ModelInvoicePrefix::year)
    (&ModelInvoicePrefix::prefix);
