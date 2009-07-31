#include "model_invoice.h"

std::string ModelInvoice::table_name = "invoice";

DEFINE_PRIMARY_KEY(ModelInvoice, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_FOREIGN_KEY(ModelInvoice, ModelZone, unsigned long long, zoneId, m_zoneId, table_name, "zone", id, )
DEFINE_BASIC_FIELD(ModelInvoice, Database::DateTime, crDate, m_crDate, table_name, "crdate", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelInvoice, Database::Date, taxDate, m_taxDate, table_name, "taxdate", .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoice, unsigned long long, prefix, m_prefix, table_name, "prefix", .setNotNull())
DEFINE_FOREIGN_KEY(ModelInvoice, ModelRegistrar, unsigned long long, registrarId, m_registrarId, table_name, "registrarid", id, .setNotNull())
DEFINE_BASIC_FIELD(ModelInvoice, Database::Money, credit, m_credit, table_name, "credit", .setDefault())
DEFINE_BASIC_FIELD(ModelInvoice, Database::Money, price, m_price, table_name, "price", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelInvoice, int, vat, m_vat, table_name, "vat", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelInvoice, Database::Money, total, m_total, table_name, "total", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelInvoice, Database::Money, totalVat, m_totalVat, table_name, "totalvat", .setDefault().setNotNull())
DEFINE_FOREIGN_KEY(ModelInvoice, ModelInvoicePrefix, unsigned long long, prefixTypeId, m_prefixTypeId, table_name, "prefix_type", id, .setNotNull())
DEFINE_FOREIGN_KEY(ModelInvoice, ModelFiles, unsigned long long, fileId, m_fileId, table_name, "file", id, )
DEFINE_FOREIGN_KEY(ModelInvoice, ModelFiles, unsigned long long, fileXmlId, m_fileXmlId, table_name, "filexml", id, )

DEFINE_ONE_TO_ONE(ModelInvoice, ModelZone, zone, m_zone, unsigned long long, zoneId, m_zoneId)
DEFINE_ONE_TO_ONE(ModelInvoice, ModelRegistrar, registrar, m_registrar, unsigned long long, registrarId, m_registrarId)
DEFINE_ONE_TO_ONE(ModelInvoice, ModelInvoicePrefix, prefixType, m_prefixType, unsigned long long, prefixTypeId, m_prefixTypeId)
DEFINE_ONE_TO_ONE(ModelInvoice, ModelFiles, file, m_file, unsigned long long, fileId, m_fileId)
DEFINE_ONE_TO_ONE(ModelInvoice, ModelFiles, fileXml, m_fileXml, unsigned long long, fileXmlId, m_fileXmlId)

ModelInvoice::field_list ModelInvoice::fields = list_of<ModelInvoice::field_list::value_type>
    (&ModelInvoice::id)
    (&ModelInvoice::zoneId)
    (&ModelInvoice::crDate)
    (&ModelInvoice::taxDate)
    (&ModelInvoice::prefix)
    (&ModelInvoice::registrarId)
    (&ModelInvoice::credit)
    (&ModelInvoice::price)
    (&ModelInvoice::vat)
    (&ModelInvoice::total)
    (&ModelInvoice::totalVat)
    (&ModelInvoice::prefixTypeId)
    (&ModelInvoice::fileId)
    (&ModelInvoice::fileXmlId)
;

