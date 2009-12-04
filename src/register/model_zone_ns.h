#ifndef _MODEL_ZONENS_H_
#define _MODEL_ZONENS_H_

#include "db_settings.h"
#include "model.h"

class ModelZoneNs:
    public Model::Base {
public:
    ModelZoneNs()
    { }
    virtual ~ModelZoneNs()
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
        return zone.getRelated(this);
    }
    */
    const std::string &getFqdn() const
    {
        return m_fqdn.get();
    }
    const std::string &getAddrs() const
    {
        return m_addrs.get();
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
        zone.setRelated(this, foreign_value);
    }
    */
    void setFqdn(const std::string &fqdn)
    {
        m_fqdn = fqdn;
    }
    void setAddrs(const std::string &addrs)
    {
        m_addrs = addrs;
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

    typedef Model::Field::List<ModelZoneNs>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_zoneId;
    Field::Field<std::string> m_fqdn;
    Field::Field<std::string> m_addrs;

    //Field::Lazy::Field<ModelZone *> m_zone;

public:
    static Model::Field::PrimaryKey<ModelZoneNs, unsigned long long> id;
    static Model::Field::Basic<ModelZoneNs, unsigned long long> zoneId;
    static Model::Field::Basic<ModelZoneNs, std::string> fqdn;
    static Model::Field::Basic<ModelZoneNs, std::string> addrs;

    //static Model::Field::Related::OneToOne<ModelZoneNs, unsigned long long, ModelZone> zone;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelZoneNs

#endif // _MODEL_ZONENS_H_

