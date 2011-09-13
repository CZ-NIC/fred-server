#ifndef _MODEL_PRICELIST_H_
#define _MODEL_PRICELIST_H_

#include "db_settings.h"
#include "model.h"

class ModelPriceList:
    public Model::Base {
public:
    ModelPriceList()
    { }
    virtual ~ModelPriceList()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getZoneId() const
    {
        return m_zoneId.get();
    }
    /*
    ModelZone *getZone()
    {
        return ftab_zone.getRelated(this);
    }
    */
    const unsigned long long &getOperationId() const
    {
        return m_operationId.get();
    }
    /*
    ModelEnumOperation *getOperation()
    {
        return ftab_operation.getRelated(this);
    }
    */
    const Database::DateTime &getValidFrom() const
    {
        return m_validFrom.get();
    }
    const Database::DateTime &getValidTo() const
    {
        return m_validTo.get();
    }
    std::string getPrice() const
    {
        return m_price.get();
    }
    const int &getQuantity() const
    {
        return m_quantity.get();
    }
    const bool &getEnablePostpaidOperation() const
    {
        return m_enable_postpaid_operation.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setZoneId(const unsigned long long &zoneId)
    {
        m_zoneId = zoneId;
    }
    /*
    void setZone(ModelZone *foreign_value)
    {
        ftab_zone.setRelated(this, foreign_value);
    }
    */
    void setOperationId(const unsigned long long &operationId)
    {
        m_operationId = operationId;
    }
    /*
    void setOperation(ModelEnumOperation *foreign_value)
    {
        ftab_operation.setRelated(this, foreign_value);
    }
    */
    void setValidFrom(const Database::DateTime &validFrom)
    {
        m_validFrom = validFrom;
    }
    void setValidTo(const Database::DateTime &validTo)
    {
        m_validTo = validTo;
    }
    void setPrice(const std::string &price)
    {
        m_price = price;
    }
    void setQuantity(const int &quantity)
    {
        m_quantity = quantity;
    }
    void setEnablePostpaidOperation(const bool &postpaid)
    {
        m_enable_postpaid_operation = postpaid;
    }

    friend class Model::Base;

    void insert()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::insert(this);
        tx.commit();
    }

    void update()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::update(this);
        tx.commit();
    }

    void reload()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::reload(this);
        tx.commit();
    }

    void load(const Database::Row &_data)
    {
        Model::Base::load(this, _data);
    }

    std::string toString() const
    {
        return Model::Base::toString(this);
    }

    typedef Model::Field::List<ModelPriceList>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_zoneId;
    Field::Field<unsigned long long> m_operationId;
    Field::Field<Database::DateTime> m_validFrom;
    Field::Field<Database::DateTime> m_validTo;
    Field::Field<std::string> m_price;
    Field::Field<int> m_quantity;
    Field::Field<bool> m_enable_postpaid_operation;

    //Field::Lazy::Field<ModelZone *> m_ftab_zone;
    //Field::Lazy::Field<ModelEnumOperation *> m_ftab_operation;

public:
    static Model::Field::PrimaryKey<ModelPriceList, unsigned long long> id;
    static Model::Field::Basic<ModelPriceList, unsigned long long> zoneId;
    static Model::Field::Basic<ModelPriceList, unsigned long long> operationId;
    static Model::Field::Basic<ModelPriceList, Database::DateTime> validFrom;
    static Model::Field::Basic<ModelPriceList, Database::DateTime> validTo;
    static Model::Field::Basic<ModelPriceList, std::string> price;
    static Model::Field::Basic<ModelPriceList, int> quantity;
    static Model::Field::Basic<ModelPriceList, bool> enable_postpaid_operation;

    //static Model::Field::Related::OneToOne<ModelPriceList, unsigned long long, ModelZone> ftab_zone;
    //static Model::Field::Related::OneToOne<ModelPriceList, unsigned long long, ModelEnumOperation> ftab_operation;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelPriceList

#endif // _MODEL_PRICELIST_H_

