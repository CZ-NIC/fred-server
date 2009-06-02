#ifndef _MODEL_LOGRAWCONTENT_H_
#define _MODEL_LOGRAWCONTENT_H_

/* << include database library settings here >> */
#include "db_settings.h"
#include "model.h"
#include "model_log_entry.h"


class ModelLogRawContent:
    public Model::Base {
public:
    ModelLogRawContent()
    { }
    virtual ~ModelLogRawContent()
    { }
    const Database::DateTime &getEntryTimeBegin() const {
        return m_entryTimeBegin.get();
    }
    const unsigned long long &getEntryId() const {
        return m_entryId.get();
    }
    ModelLogEntry *getEntry() {
        return entry.getRelated(this);
    }
    const std::string &getContent() const {
        return m_content.get();
    }
    const bool &getIsResponse() const {
        return m_isResponse.get();
    }
    void setEntryTimeBegin(const Database::DateTime &entryTimeBegin) {
        m_entryTimeBegin = entryTimeBegin;
    }
    void setEntryId(const unsigned long long &entryId) {
        m_entryId = entryId;
    }
    void setEntry(ModelLogEntry *foreign_value) {
        entry.setRelated(this, foreign_value);
    }
    void setContent(const std::string &content) {
        m_content = content;
    }
    void setIsResponse(const bool &isResponse) {
        m_isResponse = isResponse;
    }

    friend class Model::Base;

    void insert() {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::insert(this);
        tx.commit();
    }

    void update() {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::update(this);
        tx.commit();
    }

    void reload() {
        Database::Connection conn = Database::Manager::acquire();
        Database::Transaction tx(conn);
        Model::Base::reload(this);
        tx.commit();
    }

    void load(const Database::Row &_data) {
        Model::Base::load(this, _data);
    }

    std::string toString() const {
        return Model::Base::toString(this);
    }

    typedef Model::Field::List<ModelLogRawContent>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<Database::DateTime> m_entryTimeBegin;
    Field::Field<unsigned long long> m_entryId;
    Field::Field<std::string> m_content;
    Field::Field<bool> m_isResponse;

    Field::Lazy::Field<ModelLogEntry *> m_entry;

public:
    static Model::Field::Basic<ModelLogRawContent, Database::DateTime> entryTimeBegin;
    static Model::Field::ForeignKey<ModelLogRawContent, unsigned long long, ModelLogEntry> entryId;
    static Model::Field::Basic<ModelLogRawContent, std::string> content;
    static Model::Field::Basic<ModelLogRawContent, bool> isResponse;

    static Model::Field::Related::OneToOne<ModelLogRawContent, unsigned long long, ModelLogEntry> entry;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelLogRawContent

#endif // _MODEL_LOGRAWCONTENT_H_

