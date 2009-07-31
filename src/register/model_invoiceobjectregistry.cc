#include "model_invoiceobjectregistry.h"

std::string ModelInvoiceObjectRegistry::table_name = "invoice_object_registry";

DEFINE_PRIMARY_KEY(ModelInvoiceObjectRegistry, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelInvoiceObjectRegistry, ModelInvoice, unsigned long long, invoiceId, m_invoiceId, table_name, "invoiceid", id, )
DEFINE_BASIC_FIELD(ModelInvoiceObjectRegistry, Database::DateTime, crDate, m_crDate, table_name, "crdate", .setNotNull().setDefault())
DEFINE_FOREIGN_KEY(ModelInvoiceObjectRegistry, ModelObjectRegistry, unsigned long long, objectId, m_objectId, table_name, "objectid", id, )
DEFINE_FOREIGN_KEY(ModelInvoiceObjectRegistry, ModelZone, unsigned long long, zoneId, m_zoneId, table_name, "zone", id, )
DEFINE_FOREIGN_KEY(ModelInvoiceObjectRegistry, ModelRegistrar, unsigned long long, registrarId, m_registrarId, table_name, "registrarid", id, .setNotNull())
DEFINE_FOREIGN_KEY(ModelInvoiceObjectRegistry, ModelEnumOperation, unsigned long long, operationId, m_operationId, table_name, "operation", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoiceObjectRegistry, Database::Date, exDate, m_exDate, table_name, "exdate", )
DEFINE_BASIC_FIELD(ModelInvoiceObjectRegistry, int, period, m_period, table_name, "period", .setDefault())

DEFINE_ONE_TO_ONE(ModelInvoiceObjectRegistry, ModelInvoice, invoice, m_invoice, unsigned long long, invoiceId, m_invoiceId)
DEFINE_ONE_TO_ONE(ModelInvoiceObjectRegistry, ModelObjectRegistry, object, m_object, unsigned long long, objectId, m_objectId)
DEFINE_ONE_TO_ONE(ModelInvoiceObjectRegistry, ModelZone, zone, m_zone, unsigned long long, zoneId, m_zoneId)
DEFINE_ONE_TO_ONE(ModelInvoiceObjectRegistry, ModelRegistrar, registrar, m_registrar, unsigned long long, registrarId, m_registrarId)
DEFINE_ONE_TO_ONE(ModelInvoiceObjectRegistry, ModelEnumOperation, operation, m_operation, unsigned long long, operationId, m_operationId)

ModelInvoiceObjectRegistry::field_list ModelInvoiceObjectRegistry::fields = list_of<ModelInvoiceObjectRegistry::field_list::value_type>
    (&ModelInvoiceObjectRegistry::id)
    (&ModelInvoiceObjectRegistry::invoiceId)
    (&ModelInvoiceObjectRegistry::crDate)
    (&ModelInvoiceObjectRegistry::objectId)
    (&ModelInvoiceObjectRegistry::zoneId)
    (&ModelInvoiceObjectRegistry::registrarId)
    (&ModelInvoiceObjectRegistry::operationId)
    (&ModelInvoiceObjectRegistry::exDate)
    (&ModelInvoiceObjectRegistry::period);
