#ifndef _MODEL_MAILHANDLES_H_
#define _MODEL_MAILHANDLES_H_

#include "db_settings.h"
#include "model.h"
#include "model_mailarchive.h"


class ModelMailHandles:
    public Model::Base {
public:
    ModelMailHandles()
    { }
    virtual ~ModelMailHandles()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getMailId() const
    {
        return m_mailId.get();
    }
    ModelMailArchive *getMail()
    {
        return mail.getRelated(this);
    }
    const std::string &getAssoc() const
    {
        return m_assoc.get();
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setMailId(const unsigned long long &mailId)
    {
        m_mailId = mailId;
    }
    void setMail(ModelMailArchive *foreign_value)
    {
        mail.setRelated(this, foreign_value);
    }
    void setAssoc(const std::string &assoc)
    {
        m_assoc = assoc;
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

    typedef Model::Field::List<ModelMailHandles>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_mailId;
    Field::Field<std::string> m_assoc;

    Field::Lazy::Field<ModelMailArchive *> m_mail;

public:
    static Model::Field::PrimaryKey<ModelMailHandles, unsigned long long> id;
    static Model::Field::ForeignKey<ModelMailHandles, unsigned long long, ModelMailArchive> mailId;
    static Model::Field::Basic<ModelMailHandles, std::string> assoc;

    static Model::Field::Related::OneToOne<ModelMailHandles, unsigned long long, ModelMailArchive> mail;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelMailHandles

#endif // _MODEL_MAILHANDLES_H_

