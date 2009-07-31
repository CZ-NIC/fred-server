#ifndef _MODEL_INVOICEOBJECTREGISTRY_H_
#define _MODEL_INVOICEOBJECTREGISTRY_H_

#include "db_settings.h"
#include "model_enumoperation.h"
#include "model_invoice.h"
#include "model_zone.h"
#include "model_registrar.h"
#include "model_object_registry.h"

class ModelInvoiceObjectRegistry:
    public Model::Base {
public:
    ModelInvoiceObjectRegistry()
    { }
    virtual ~ModelInvoiceObjectRegistry()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    const unsigned long long &getInvoiceId() const
    {
        return m_invoiceId.get();
    }
    void setInvoiceId(const unsigned long long &invoiceId)
    {
        m_invoiceId = invoiceId;
    }
    ModelInvoice *getInvoice()
    {
        return invoice.getRelated(this);
    }
    void setInvoice(ModelInvoice *inv)
    {
        invoice.setRelated(this, inv);
    }
    const Database::DateTime &getCrDate() const
    {
        return m_crDate.get();
    }
    void setCrDate(const Database::DateTime &crDate)
    {
        m_crDate = crDate;
    }
    const unsigned long long &getObjectId() const
    {
        return m_objectId.get();
    }
    void setObjectId(const unsigned long long &objectId)
    {
        m_objectId = objectId;
    }
    ModelObjectRegistry *getObject()
    {
        return object.getRelated(this);
    }
    void setObject(ModelObjectRegistry *obj)
    {
        object.setRelated(this, obj);
    }
    const unsigned long long &getZoneId() const
    {
        return m_zoneId.get();
    }
    void setZoneId(const unsigned long long &zoneId)
    {
        m_zoneId = zoneId;
    }
    ModelZone *getZone()
    {
        return zone.getRelated(this);
    }
    void setZone(ModelZone *zon)
    {
        zone.setRelated(this, zon);
    }
    const unsigned long long &getRegistrarId() const
    {
        return m_registrarId.get();
    }
    void setRegistrarId(const unsigned long long &registrarId)
    {
        m_registrarId = registrarId;
    }
    ModelRegistrar *getRegistrar()
    {
        return registrar.getRelated(this);
    }
    void setRegistrar(ModelRegistrar *reg)
    {
        registrar.setRelated(this, reg);
    }
    const unsigned long long getOperationId() const
    {
        return m_operationId.get();
    }
    void setOperationId(const unsigned long long &operationId)
    {
        m_operationId = operationId;
    }
    ModelEnumOperation *getOperation()
    {
        return operation.getRelated(this);
    }
    void setOperation(ModelEnumOperation *op)
    {
        operation.setRelated(this, op);
    }
    const Database::Date getExDate() const
    {
        return m_exDate.get();
    }
    void setExDate(const Database::Date &exDate)
    {
        m_exDate = exDate;
    }
    const int &getPeriod() const
    {
        return m_period.get();
    }
    void setPeriod(const int &period)
    {
        m_period = period;
    }

    friend class Model::Base;

    void insert()
    {
        Model::Base::insert(this);
    }
    void update()
    {
        Model::Base::update(this);
    }

    void reload()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction transaction(c);
        Model::Base::reload(this);
        transaction.commit();
    }


protected:
    Field::Field<unsigned long long>          m_id;
    Field::Field<unsigned long long>          m_invoiceId;
    Field::Field<Database::DateTime>    m_crDate;
    Field::Field<unsigned long long>          m_objectId;
    Field::Field<unsigned long long>          m_zoneId;
    Field::Field<unsigned long long>          m_registrarId;
    Field::Field<unsigned long long>          m_operationId;
    Field::Field<Database::Date>        m_exDate;
    Field::Field<int>                   m_period;

    Field::Lazy::Field<ModelInvoice *>      m_invoice;
    Field::Lazy::Field<ModelObjectRegistry *>       m_object;
    Field::Lazy::Field<ModelZone *>         m_zone;
    Field::Lazy::Field<ModelRegistrar *>    m_registrar;
    Field::Lazy::Field<ModelEnumOperation *>    m_operation;

    typedef Model::Field::List<ModelInvoiceObjectRegistry> field_list;

public:
    static Model::Field::PrimaryKey<ModelInvoiceObjectRegistry, unsigned long long>       id;
    static Model::Field::ForeignKey<ModelInvoiceObjectRegistry, unsigned long long, ModelInvoice> invoiceId;
    static Model::Field::Basic<ModelInvoiceObjectRegistry, Database::DateTime>      crDate;
    static Model::Field::ForeignKey<ModelInvoiceObjectRegistry, unsigned long long, ModelObjectRegistry>  objectId;
    static Model::Field::ForeignKey<ModelInvoiceObjectRegistry, unsigned long long, ModelZone>    zoneId;
    static Model::Field::ForeignKey<ModelInvoiceObjectRegistry, unsigned long long, ModelRegistrar> registrarId;
    static Model::Field::ForeignKey<ModelInvoiceObjectRegistry, unsigned long long, ModelEnumOperation> operationId;
    static Model::Field::Basic<ModelInvoiceObjectRegistry, Database::Date>          exDate;
    static Model::Field::Basic<ModelInvoiceObjectRegistry, int>                     period;

    static Model::Field::Related::OneToOne<ModelInvoiceObjectRegistry, unsigned long long, ModelInvoice>  invoice;
    static Model::Field::Related::OneToOne<ModelInvoiceObjectRegistry, unsigned long long, ModelObjectRegistry>   object;
    static Model::Field::Related::OneToOne<ModelInvoiceObjectRegistry, unsigned long long, ModelZone>     zone;
    static Model::Field::Related::OneToOne<ModelInvoiceObjectRegistry, unsigned long long, ModelRegistrar>        registrar;
    static Model::Field::Related::OneToOne<ModelInvoiceObjectRegistry, unsigned long long, ModelEnumOperation>    operation;


    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;
}; // class ModelInvoiceObjectRegistry

#endif // _MODEL_INVOICEOBJECTREGISTRY_H_
