#include "model_invoicegeneration.h"

std::string ModelInvoiceGeneration::table_name = "invoice_generation";

DEFINE_PRIMARY_KEY(ModelInvoiceGeneration, unsigned long long , id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelInvoiceGeneration, Database::Date, fromDate, m_fromDate, table_name, "fromdate", .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoiceGeneration, Database::Date, toDate, m_toDate, table_name, "todate", .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoiceGeneration, unsigned long long , registrarId, m_registrarId, table_name, "registrarid", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelInvoiceGeneration, unsigned long long , zoneId, m_zoneId, table_name, "zone", .setForeignKey() )
DEFINE_BASIC_FIELD(ModelInvoiceGeneration, unsigned long long , invoiceId, m_invoiceId, table_name, "invoiceid", .setForeignKey() )

//DEFINE_ONE_TO_ONE(ModelInvoiceGeneration, ModelRegistrar, registrar, m_registrar, unsigned long long , registrarId, m_registrarId)
//DEFINE_ONE_TO_ONE(ModelInvoiceGeneration, ModelZone, zone, m_zone, unsigned long long , zoneId, m_zoneId)
//DEFINE_ONE_TO_ONE(ModelInvoiceGeneration, ModelInvoice, invoice, m_invoice, unsigned long long , invoiceId, m_invoiceId)

ModelInvoiceGeneration::field_list ModelInvoiceGeneration::fields = list_of<ModelInvoiceGeneration::field_list::value_type>
    (&ModelInvoiceGeneration::id)
    (&ModelInvoiceGeneration::fromDate)
    (&ModelInvoiceGeneration::toDate)
    (&ModelInvoiceGeneration::registrarId)
    (&ModelInvoiceGeneration::zoneId)
    (&ModelInvoiceGeneration::invoiceId);
