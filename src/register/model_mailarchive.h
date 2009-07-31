#ifndef _MODEL_MAILARCHIVE_H_
#define _MODEL_MAILARCHIVE_H_

#include "db_settings.h"
#include "model.h"
#include "model_mailtype.h"
#include "model_mailattachments.h"


class ModelMailArchive:
    public Model::Base {
public:
    ModelMailArchive()
    { }
    virtual ~ModelMailArchive()
    { }
    const unsigned long long &getId() const
    {
        return m_id.get();
    }
    const unsigned long long &getMailTypeId() const
    {
        return m_mailTypeId.get();
    }
    ModelMailType *getMailType()
    {
        return mailType.getRelated(this);
    }
    const Database::DateTime &getCrDate() const
    {
        return m_crDate.get();
    }
    const Database::DateTime &getModDate() const
    {
        return m_modDate.get();
    }
    const int &getStatus() const
    {
        return m_status.get();
    }
    const std::string &getMessage() const
    {
        return m_message.get();
    }
    const short int &getAttempt() const
    {
        return m_attempt.get();
    }
    const std::string &getResponse() const
    {
        return m_response.get();
    }
    const Field::Lazy::List<ModelMailAttachments> &getAttachments()
    {
        attachments.getRelated(this);
        return m_attachments;
    }
    void setId(const unsigned long long &id)
    {
        m_id = id;
    }
    void setMailTypeId(const unsigned long long &mailTypeId)
    {
        m_mailTypeId = mailTypeId;
    }
    void setMailType(ModelMailType *foreign_value)
    {
        mailType.setRelated(this, foreign_value);
    }
    void setCrDate(const Database::DateTime &crDate)
    {
        m_crDate = crDate;
    }
    void setModDate(const Database::DateTime &modDate)
    {
        m_modDate = modDate;
    }
    void setStatus(const int &status)
    {
        m_status = status;
    }
    void setMessage(const std::string &message)
    {
        m_message = message;
    }
    void setAttempt(const short int &attempt)
    {
        m_attempt = attempt;
    }
    void setResponse(const std::string &response)
    {
        m_response = response;
    }
    void addAttachment(ModelMailAttachments *attach)
    {
        attachments.addRelated(this, attach);
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

    typedef Model::Field::List<ModelMailArchive>  field_list;
    static const field_list& getFields()
    {
        return fields;
    }

protected:
    Field::Field<unsigned long long> m_id;
    Field::Field<unsigned long long> m_mailTypeId;
    Field::Field<Database::DateTime> m_crDate;
    Field::Field<Database::DateTime> m_modDate;
    Field::Field<int> m_status;
    Field::Field<std::string> m_message;
    Field::Field<short int> m_attempt;
    Field::Field<std::string> m_response;

    Field::Lazy::Field<ModelMailType *> m_mailType;

    Field::Lazy::List<ModelMailAttachments> m_attachments;

public:
    static Model::Field::PrimaryKey<ModelMailArchive, unsigned long long> id;
    static Model::Field::ForeignKey<ModelMailArchive, unsigned long long, ModelMailType> mailTypeId;
    static Model::Field::Basic<ModelMailArchive, Database::DateTime> crDate;
    static Model::Field::Basic<ModelMailArchive, Database::DateTime> modDate;
    static Model::Field::Basic<ModelMailArchive, int> status;
    static Model::Field::Basic<ModelMailArchive, std::string> message;
    static Model::Field::Basic<ModelMailArchive, short int> attempt;
    static Model::Field::Basic<ModelMailArchive, std::string> response;

    static Model::Field::Related::OneToOne<ModelMailArchive, unsigned long long, ModelMailType> mailType;

    static Model::Field::Related::OneToMany<ModelMailArchive, unsigned long long, ModelMailAttachments> attachments;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelMailArchive

#endif // _MODEL_MAILARCHIVE_H_

