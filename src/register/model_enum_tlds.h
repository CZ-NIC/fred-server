#ifndef _MODEL_ENUMTLDS_H_
#define _MODEL_ENUMTLDS_H_

#include "db_settings.h"
#include "model.h"


class ModelEnumTlds:
    public Model::Base {
public:
    ModelEnumTlds()
    { }
    virtual ~ModelEnumTlds()
    { }
    std::string getTld() const
    {
        return m_tld;
    }
    void setTld(const std::string &tld)
    {
        m_tld = tld;
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

    typedef Model::Field::List<ModelEnumTlds>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<std::string> m_tld;


public:
    static Model::Field::PrimaryKey<ModelEnumTlds, std::string> tld;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelEnumTlds

#endif // _MODEL_ENUMTLDS_H_

