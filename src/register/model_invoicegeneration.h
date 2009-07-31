#ifndef _MODEL_INVOICEGENERATION_H_
#define _MODEL_INVOICEGENERATION_H_

#include "db_settings.h"
#include "model_zone.h"
#include "model_invoice.h"
#include "model_registrar.h"

class ModelInvoiceGeneration:
    public Model::Base {
public:
    ModelInvoiceGeneration()
    { }
    virtual ~ModelInvoiceGeneration()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    const Database::Date &getFromDate() const
    {
        return m_fromDate.get();
    }
    void setFromDate(const Database::Date &fromDate)
    {
        m_fromDate = fromDate;
    }
    const Database::Date &getToDate() const
    {
        return m_toDate.get();
    }
    void setToDate(const Database::Date &toDate)
    {
        m_toDate = toDate;
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
    Field::Field<unsigned long long>      m_id;
    Field::Field<Database::Date>    m_fromDate;
    Field::Field<Database::Date>    m_toDate;
    Field::Field<unsigned long long>      m_registrarId;
    Field::Field<unsigned long long>      m_zoneId;
    Field::Field<unsigned long long>      m_invoiceId;

    Field::Lazy::Field<ModelRegistrar *>    m_registrar;
    Field::Lazy::Field<ModelZone *>         m_zone;
    Field::Lazy::Field<ModelInvoice *>      m_invoice;

    typedef Model::Field::List<ModelInvoiceGeneration> field_list;
public:
    static Model::Field::PrimaryKey<ModelInvoiceGeneration, unsigned long long>   id;
    static Model::Field::Basic<ModelInvoiceGeneration, Database::Date>      fromDate;
    static Model::Field::Basic<ModelInvoiceGeneration, Database::Date>      toDate;
    static Model::Field::ForeignKey<ModelInvoiceGeneration, unsigned long long, ModelRegistrar> registrarId;
    static Model::Field::ForeignKey<ModelInvoiceGeneration, unsigned long long, ModelZone>      zoneId;
    static Model::Field::ForeignKey<ModelInvoiceGeneration, unsigned long long, ModelInvoice>   invoiceId;

    static Model::Field::Related::OneToOne<ModelInvoiceGeneration, unsigned long long, ModelRegistrar>    registrar;
    static Model::Field::Related::OneToOne<ModelInvoiceGeneration, unsigned long long, ModelZone>         zone;
    static Model::Field::Related::OneToOne<ModelInvoiceGeneration, unsigned long long, ModelInvoice>      invoice;


    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;

}; // class ModelInvoiceGeneration

#endif // _MODEL_INVOICEGENERATION_H_
