#include "model_price_list.h"

std::string ModelPriceList::table_name = "price_list";

DEFINE_PRIMARY_KEY(ModelPriceList, unsigned long long, id, m_id, table_name, "id", .setDefault())
DEFINE_BASIC_FIELD(ModelPriceList, unsigned long long, zoneId, m_zoneId, table_name, "zone", .setNotNull())
DEFINE_BASIC_FIELD(ModelPriceList, unsigned long long, operationId, m_operationId, table_name, "operation", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelPriceList, Database::DateTime, validFrom, m_validFrom, table_name, "valid_from", .setNotNull().setForeignKey())
DEFINE_BASIC_FIELD(ModelPriceList, Database::DateTime, validTo, m_validTo, table_name, "valid_to", )
DEFINE_BASIC_FIELD(ModelPriceList, std::string, price, m_price, table_name, "price", .setDefault().setNotNull())
DEFINE_BASIC_FIELD(ModelPriceList, int, period, m_period, table_name, "period", .setDefault())

ModelPriceList::field_list ModelPriceList::fields = list_of<ModelPriceList::field_list::value_type>
    (&ModelPriceList::id)
    (&ModelPriceList::zoneId)
    (&ModelPriceList::operationId)
    (&ModelPriceList::validFrom)
    (&ModelPriceList::validTo)
    (&ModelPriceList::price)
    (&ModelPriceList::period)
;

