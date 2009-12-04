#ifndef _MODEL_INVOICEPREFIX_H_
#define _MODEL_INVOICEPREFIX_H_

#include "db_settings.h"
#include "model.h"

class ModelInvoicePrefix:
    public Model::Base {
public:
    ModelInvoicePrefix()
    { }
    virtual ~ModelInvoicePrefix()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }

    const unsigned long long &getZoneId() const
    {
        return m_zoneId.get();
    }
    void setZoneId(const unsigned long long &id)
    {
        m_zoneId = id;
    }
/*
    ModelZone *getZone()
    {
        return zone.getRelated(this);
    }
    void setZone(ModelZone *zon)
    {
        zone.setRelated(this, zon);
    }
    */
    const int &getType() const
    {
        return m_type.get();
    }
    void setType(const int &type)
    {
        m_type = type;
    }
    const int &getYear() const
    {
        return m_year.get();
    }
    void setYear(const int &year)
    {
        m_year = year;
    }
    const unsigned long long getPrefix() const
    {
        return m_prefix.get();
    }
    void setPrefix(const unsigned long long &prefix)
    {
        m_prefix = prefix;
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
    Field::Field<unsigned long long>    m_id;
    Field::Field<unsigned long long>    m_zoneId;
    Field::Field<int>                   m_type;
    Field::Field<int>                   m_year;
    Field::Field<unsigned long long>    m_prefix;

    //Field::Lazy::Field<ModelZone *>          m_zone;

    typedef Model::Field::List<ModelInvoicePrefix>   field_list;
public:
    static Model::Field::PrimaryKey<ModelInvoicePrefix, unsigned long long>  id;
    static Model::Field::Basic<ModelInvoicePrefix, unsigned long long> zoneId;
    static Model::Field::Basic<ModelInvoicePrefix, int>                      type;
    static Model::Field::Basic<ModelInvoicePrefix, int>                      year;
    static Model::Field::Basic<ModelInvoicePrefix, unsigned long long>       prefix;

    //static Model::Field::Related::OneToOne<ModelInvoicePrefix, unsigned long long, ModelZone> zone;


    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;
}; // class ModelInvoicePrefix;

#endif // _MODEL_INVOICEPREFIX_H_
