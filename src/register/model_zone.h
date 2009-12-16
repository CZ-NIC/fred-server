#ifndef _MODEL_ZONE_H_
#define _MODEL_ZONE_H_

#include "db_settings.h"
#include "model.h"

class ModelZone:
    public Model::Base {
public:
    ModelZone()
    { }
    virtual ~ModelZone()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    const std::string &getFqdn() const
    {
        return m_fqdn.get();
    }
    void setFqdn(const std::string &fqdn)
    {
        m_fqdn = fqdn;
    }
    const int &getExPeriodMin() const
    {
        return m_exPeriodMin.get();
    }
    void setExPeriodMin(const int &exPeriodMin)
    {
        m_exPeriodMin = exPeriodMin;
    }
    const int &getExPeriodMax() const
    {
        return m_exPeriodMax.get();
    }
    void setExPeriodMax(const int &exPeriodMax)
    {
        m_exPeriodMax = exPeriodMax;
    }
    const int &getValPeriod() const
    {
        return m_valPeriod.get();
    }
    void setValPeriod(const int &valPeriod)
    {
        m_valPeriod = valPeriod;
    }
    const int &getDotsMax() const
    {
        return m_dotsMax.get();
    }
    void setDotsMax(const int &dotsMax)
    {
        m_dotsMax = dotsMax;
    }
    const bool &getEnumZone() const
    {
        return m_enumZone.get();
    }
    void setEnumZone(const bool &enumZone)
    {
        m_enumZone = enumZone;
    }

    friend class Model::Base;

    void insert()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction tx(c);
        Model::Base::insert(this);
        tx.commit();
    }
    void update()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction tx(c);
        Model::Base::update(this);
        tx.commit();
    }

    void reload()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction tx(c);
        Model::Base::reload(this);
        tx.commit();
    }

protected:
    Field::Field<unsigned long long>    m_id;
    Field::Field<std::string>           m_fqdn;
    Field::Field<int>                   m_exPeriodMin;
    Field::Field<int>                   m_exPeriodMax;
    Field::Field<int>                   m_valPeriod;
    Field::Field<int>                   m_dotsMax;
    Field::Field<bool>                  m_enumZone;

    typedef Model::Field::List<ModelZone>    field_list;

public:
    static Model::Field::PrimaryKey<ModelZone, unsigned long long>   id;
    static Model::Field::Basic<ModelZone, std::string>               fqdn;
    static Model::Field::Basic<ModelZone, int>                       exPeriodMin;
    static Model::Field::Basic<ModelZone, int>                       exPeriodMax;
    static Model::Field::Basic<ModelZone, int>                       valPeriod;
    static Model::Field::Basic<ModelZone, int>                       dotsMax;
    static Model::Field::Basic<ModelZone, bool>                      enumZone;


    static const field_list &getFields()
    {
        return fields;
    }

private:
    static std::string  table_name;
    static field_list   fields;
}; // class ModelZone;

#endif // _MODEL_ZONE_H_
