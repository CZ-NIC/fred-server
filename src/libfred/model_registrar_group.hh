#ifndef MODEL_REGISTRAR_GROUP_HH_663AFB6873DC47B5A8FB94D6BD0807AC
#define MODEL_REGISTRAR_GROUP_HH_663AFB6873DC47B5A8FB94D6BD0807AC

#include "src/libfred/db_settings.hh"
#include "src/util/db/model/model.hh"

class ModelRegistrarGroup:
    public Model::Base {
public:
    ModelRegistrarGroup()
    { }
    virtual ~ModelRegistrarGroup()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }

    const std::string &getShortName() const
    {
        return m_short_name.get();
    }
    const Database::DateTime &getCancelled() const
    {
        return m_cancelled.get();
    }

    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setShortName(const std::string &short_name)
    {
        m_short_name = short_name;
    }
    void setCancelled(const Database::DateTime &cancelled)
    {
        m_cancelled = cancelled;
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

    typedef Model::Field::List<ModelRegistrarGroup>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_short_name;
    Field::Field<Database::DateTime> m_cancelled;

public:
    static Model::Field::PrimaryKey<ModelRegistrarGroup, unsigned long long> id;
    static Model::Field::Basic<ModelRegistrarGroup, std::string> short_name;
    static Model::Field::Basic<ModelRegistrarGroup, Database::DateTime> cancelled;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRegistrarGroup

#endif
