#ifndef _MODEL_REGISTRAR_GROUP_MAP_H_
#define _MODEL_REGISTRAR_GROUP_MAP_H_

#include "db_settings.h"
#include "model.h"

class ModelRegistrarGroupMap:
    public Model::Base {
public:
    ModelRegistrarGroupMap()
    { }
    virtual ~ModelRegistrarGroupMap()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getRegistrarId() const
    {
        return m_registrar_id.get();
    }
    const unsigned long long &getRegistrarGroupId() const
    {
        return m_registrar_group_id.get();
    }
    const Database::Date &getMemberFrom() const
    {
        return m_member_from.get();
    }
    const Database::Date &getMemberUntil() const
    {
        return m_member_until.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setRegistrarId(const unsigned long long &registrarId)
    {
        m_registrar_id = registrarId;
    }
    void setRegistrarGroupId(const unsigned long long &registrarGroupId)
    {
        m_registrar_group_id = registrarGroupId;
    }
    void setMemberFrom(const Database::Date &memberFrom)
    {
        m_member_from = memberFrom;
    }
    void setMemberUntil(const Database::Date &memberUntil)
    {
        m_member_until = memberUntil;
    }

    friend class Model::Base;

    void insert()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        //serialization of constraint check in trigger function
        conn.exec("LOCK TABLE registrar_group_map IN ACCESS EXCLUSIVE MODE");
        Model::Base::insert(this);
        tx.commit();
    }

    void update()
    {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        //serialization of constraint check in trigger function
        conn.exec("LOCK TABLE registrar_group_map IN ACCESS EXCLUSIVE MODE");
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

    typedef Model::Field::List<ModelRegistrarGroupMap>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_registrar_id;
    Field::Field<unsigned long long> m_registrar_group_id;
    Field::Field<Database::Date> m_member_from;
    Field::Field<Database::Date> m_member_until;

public:
    static Model::Field::PrimaryKey<ModelRegistrarGroupMap, unsigned long long> id;
    static Model::Field::Basic<ModelRegistrarGroupMap, unsigned long long> registrar_id;
    static Model::Field::Basic<ModelRegistrarGroupMap, unsigned long long> registrar_group_id;
    static Model::Field::Basic<ModelRegistrarGroupMap, Database::Date> member_from;
    static Model::Field::Basic<ModelRegistrarGroupMap, Database::Date> member_until;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRegistrarGroupMap

#endif // _MODEL_REGISTRAR_GROUP_MAP_H_
