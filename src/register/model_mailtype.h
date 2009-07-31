#ifndef _MODEL_MAILTYPE_H_
#define _MODEL_MAILTYPE_H_

#include "db_settings.h"
#include "model.h"


class ModelMailType:
    public Model::Base {
public:
    ModelMailType()
    { }
    virtual ~ModelMailType()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const std::string &getName() const
    {
        return m_name.get();
    }
    const std::string &getSubject() const
    {
        return m_subject.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setName(const std::string &name)
    {
        m_name = name;
    }
    void setSubject(const std::string &subject)
    {
        m_subject = subject;
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

    typedef Model::Field::List<ModelMailType>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<std::string> m_name;
    Field::Field<std::string> m_subject;


public:
    static Model::Field::PrimaryKey<ModelMailType, unsigned long long> id;
    static Model::Field::Basic<ModelMailType, std::string> name;
    static Model::Field::Basic<ModelMailType, std::string> subject;


private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelMailType

#endif // _MODEL_MAILTYPE_H_

