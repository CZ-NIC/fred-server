#ifndef _MODEL_ZONESOA_H_
#define _MODEL_ZONESOA_H_

#include "db_settings.h"
#include "model.h"
#include "model_zone.h"


class ModelZoneSoa
	: public ModelZone {

public:
    ModelZoneSoa()
    : ModelZone()
    { }
    ModelZoneSoa(const ModelZone& mzn)
    :ModelZone(mzn)
    { }
    virtual ~ModelZoneSoa()
    { }
    const unsigned long long &getZone() const
    {
        return m_zone.get();
    }
    void setZone(const unsigned long long &zone)
    {
        m_zone = zone;
    }
    /*
    ModelZone *getZone()
    {
        return ftab_zone.getRelated(this);
    }
    */
    const int &getTtl() const
    {
        return m_ttl.get();
    }
    const std::string &getHostmaster() const
    {
        return m_hostmaster.get();
    }
    const int &getSerial() const
    {
        return m_serial.get();
    }
    const int &getRefresh() const
    {
        return m_refresh.get();
    }
    const int &getUpdateRetr() const
    {
        return m_updateRetr.get();
    }
    const int &getExpiry() const
    {
        return m_expiry.get();
    }
    const int &getMinimum() const
    {
        return m_minimum.get();
    }
    const std::string &getNsFqdn() const
    {
        return m_nsFqdn.get();
    }
    /*
    void setZone(ModelZone *foreign_value)
    {
        ftab_zone.setRelated(this, foreign_value);
    }
    */
    void setTtl(const int &ttl)
    {
        m_ttl = ttl;
    }
    void setHostmaster(const std::string &hostmaster)
    {
        m_hostmaster = hostmaster;
    }
    void setSerial(const int &serial)
    {
        m_serial = serial;
    }
    void setRefresh(const int &refresh)
    {
        m_refresh = refresh;
    }
    void setUpdateRetr(const int &updateRetr)
    {
        m_updateRetr = updateRetr;
    }
    void setExpiry(const int &expiry)
    {
        m_expiry = expiry;
    }
    void setMinimum(const int &minimum)
    {
        m_minimum = minimum;
    }
    void setNsFqdn(const std::string &nsFqdn)
    {
        m_nsFqdn = nsFqdn;
    }

    friend class Model::Base;

    void insert()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        ModelZone::insert();
        this->setZone(this->getId());
        Model::Base::insert(this);
        tx.commit();
    }

    void update()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        ModelZone::update();
        this->setZone(this->getId());
        Model::Base::update(this);
        tx.commit();
    }

    void reload()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        ModelZone::reload();
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

    typedef Model::Field::List<ModelZoneSoa>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_zone;
    Field::Field<int> m_ttl;
    Field::Field<std::string> m_hostmaster;
    Field::Field<int> m_serial;
    Field::Field<int> m_refresh;
    Field::Field<int> m_updateRetr;
    Field::Field<int> m_expiry;
    Field::Field<int> m_minimum;
    Field::Field<std::string> m_nsFqdn;

    //Field::Lazy::Field<ModelZone*> m_ftab_zone;//ModelZone by ptr


public:
    static Model::Field::PrimaryKey<ModelZoneSoa, unsigned long long> zone;
    static Model::Field::Basic<ModelZoneSoa, int> ttl;
    static Model::Field::Basic<ModelZoneSoa, std::string> hostmaster;
    static Model::Field::Basic<ModelZoneSoa, int> serial;
    static Model::Field::Basic<ModelZoneSoa, int> refresh;
    static Model::Field::Basic<ModelZoneSoa, int> updateRetr;
    static Model::Field::Basic<ModelZoneSoa, int> expiry;
    static Model::Field::Basic<ModelZoneSoa, int> minimum;
    static Model::Field::Basic<ModelZoneSoa, std::string> nsFqdn;

    //static Model::Field::Related::OneToOne<ModelZoneSoa, unsigned long long, ModelZone> ftab_zone;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelZoneSoa

#endif // _MODEL_ZONESOA_H_

