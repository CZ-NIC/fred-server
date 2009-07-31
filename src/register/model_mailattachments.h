#ifndef _MODEL_MAILATTACHMENTS_H_
#define _MODEL_MAILATTACHMENTS_H_

#include "db_settings.h"
#include "model.h"
#include "model_files.h"


class ModelMailArchive;

class ModelMailAttachments:
    public Model::Base {
public:
    ModelMailAttachments()
    { }
    virtual ~ModelMailAttachments()
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
    const unsigned long long &getAttachId() const
    {
        return m_attachId.get();
    }
    ModelFiles *getAttach()
    {
        return attach.getRelated(this);
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
    void setAttachId(const unsigned long long &attachId)
    {
        m_attachId = attachId;
    }
    void setAttach(ModelFiles *foreign_value)
    {
        attach.setRelated(this, foreign_value);
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

    typedef Model::Field::List<ModelMailAttachments>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_mailId;
    Field::Field<unsigned long long> m_attachId;

    Field::Lazy::Field<ModelMailArchive *> m_mail;
    Field::Lazy::Field<ModelFiles *> m_attach;

public:
    static Model::Field::PrimaryKey<ModelMailAttachments, unsigned long long> id;
    static Model::Field::ForeignKey<ModelMailAttachments, unsigned long long, ModelMailArchive> mailId;
    static Model::Field::ForeignKey<ModelMailAttachments, unsigned long long, ModelFiles> attachId;

    static Model::Field::Related::OneToOne<ModelMailAttachments, unsigned long long, ModelMailArchive> mail;
    static Model::Field::Related::OneToOne<ModelMailAttachments, unsigned long long, ModelFiles> attach;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelMailAttachments

#endif // _MODEL_MAILATTACHMENTS_H_

