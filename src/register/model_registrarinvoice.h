#ifndef _MODEL_REGISTRARINVOICE_H_
#define _MODEL_REGISTRARINVOICE_H_

#include "db_settings.h"
#include "model.h"
#include "model_registrar.h"
#include "model_zone.h"


class ModelRegistrarinvoice:
    public Model::Base {
public:
    ModelRegistrarinvoice()
    { }
    virtual ~ModelRegistrarinvoice()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getRegistrarId() const
    {
        return m_registrarId.get();
    }
    ModelRegistrar *getRegistrar()
    {
        return ftab_registrar.getRelated(this);
    }
    const unsigned long long &getZoneId() const
    {
        return m_zoneId.get();
    }
    ModelZone *getZone()
    {
        return ftab_zone.getRelated(this);
    }
    const Database::Date &getFromDate() const
    {
        return m_fromDate.get();
    }
    const Database::Date &getLastDate() const
    {
        return m_lastDate.get();
    }
    const Database::Date &getToDate() const
    {
        return m_toDate.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setRegistrarId(const unsigned long long &registrarId)
    {
        m_registrarId = registrarId;
    }
    void setRegistrar(ModelRegistrar *foreign_value)
    {
        ftab_registrar.setRelated(this, foreign_value);
    }
    void setZoneId(const unsigned long long &zoneId)
    {
        m_zoneId = zoneId;
    }
    void setZone(ModelZone *foreign_value)
    {
        ftab_zone.setRelated(this, foreign_value);
    }
    void setFromDate(const Database::Date &fromDate)
    {
        m_fromDate = fromDate;
    }
    void setLastDate(const Database::Date &lastDate)
    {
        m_lastDate = lastDate;
    }
    void setToDate(const Database::Date &toDate)
    {
        m_toDate = toDate;
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

    typedef Model::Field::List<ModelRegistrarinvoice>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_registrarId;
    Field::Field<unsigned long long> m_zoneId;
    Field::Field<Database::Date> m_fromDate;
    Field::Field<Database::Date> m_lastDate;
    Field::Field<Database::Date> m_toDate;

    Field::Lazy::Field<ModelRegistrar *> m_ftab_registrar;
    Field::Lazy::Field<ModelZone *> m_ftab_zone;

public:
    static Model::Field::PrimaryKey<ModelRegistrarinvoice, unsigned long long> id;
    static Model::Field::ForeignKey<ModelRegistrarinvoice, unsigned long long, ModelRegistrar> registrarId;
    static Model::Field::ForeignKey<ModelRegistrarinvoice, unsigned long long, ModelZone> zoneId;
    static Model::Field::Basic<ModelRegistrarinvoice, Database::Date> fromDate;
    static Model::Field::Basic<ModelRegistrarinvoice, Database::Date> lastDate;
    static Model::Field::Basic<ModelRegistrarinvoice, Database::Date> toDate;

    static Model::Field::Related::OneToOne<ModelRegistrarinvoice, unsigned long long, ModelRegistrar> ftab_registrar;
    static Model::Field::Related::OneToOne<ModelRegistrarinvoice, unsigned long long, ModelZone> ftab_zone;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRegistrarinvoice

#endif // _MODEL_REGISTRARINVOICE_H_

