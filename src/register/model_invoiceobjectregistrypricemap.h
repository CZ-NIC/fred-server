#ifndef _MODEL_INVOICEOBJECTREGISTRYPRICEMAP_H_
#define _MODEL_INVOICEOBJECTREGISTRYPRICEMAP_H_

#include "db_settings.h"
#include "model_invoice.h"
#include "model_invoiceobjectregistry.h"

class ModelInvoiceObjectRegistryPriceMap:
    public Model::Base {
public:
    ModelInvoiceObjectRegistryPriceMap()
    { }
    virtual ~ModelInvoiceObjectRegistryPriceMap()
    { }
    const unsigned long long &getInvoiceObjectRegistryId() const
    {
        return m_invoiceObjectRegistryId.get();
    }
    void setInvoiceObjectRegistryId(const unsigned long long &invoiceObjectRegistry)
    {
        m_invoiceObjectRegistryId = invoiceObjectRegistry;
    }
#if 0
    ModelInvoiceObjectRegistry *getInvoiceObjectRegistry()
    {
        return invoiceObjectRegistry.getRelated(this);
    }
    void setInvoiceObjectRegistry(ModelInvoiceObjectRegistry *mior)
    {
        invoiceObjectRegistry.setRelated(this, mior);
    }
#endif
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
    const Database::Money &getPrice() const
    {
        return m_price.get();
    }
    void setPrice(const Database::Money &price)
    {
        m_price = price;
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
    Field::Field<unsigned long long>          m_invoiceObjectRegistryId;
    Field::Field<unsigned long long>          m_invoiceId;
    Field::Field<Database::Money>       m_price;

    // Field::Lazy::Field<ModelInvoiceObjectRegistry *>    m_invoiceObjectRegistry;
    Field::Lazy::Field<ModelInvoice *>                  m_invoice;

    typedef Model::Field::List<ModelInvoiceObjectRegistryPriceMap> field_list;

public:
    // static Model::Field::ForeignKey<ModelInvoiceObjectRegistryPriceMap, unsigned long long, ModelInvoiceObjectRegistry>   invoiceObjectRegistryId;
    static Model::Field::PrimaryKey<ModelInvoiceObjectRegistryPriceMap, unsigned long long> invoiceObjectRegistryId;
    static Model::Field::ForeignKey<ModelInvoiceObjectRegistryPriceMap, unsigned long long, ModelInvoice> invoiceId;
    static Model::Field::Basic<ModelInvoiceObjectRegistryPriceMap, Database::Money> price;

    // static Model::Field::Related::OneToOne<ModelInvoiceObjectRegistryPriceMap, unsigned long long, ModelInvoiceObjectRegistry> invoiceObjectRegistry;
    static Model::Field::Related::OneToOne<ModelInvoiceObjectRegistryPriceMap, unsigned long long, ModelInvoice>  invoice;


    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;
}; // class ModelInvoiceObjectRegistryPriceMap
    
#endif // _MODEL_INVOICEOBJECTREGISTRYPRICEMAP_H_
