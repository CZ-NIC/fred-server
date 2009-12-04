#ifndef _MODEL_REQUESTDATA_H_
#define _MODEL_REQUESTDATA_H_

#include "db_settings.h"
#include "model.h"
//#include "model_request.h"


class ModelRequestData:
    public Model::Base {
public:
    ModelRequestData()
    { }
    virtual ~ModelRequestData()
    { }
    const Database::DateTime &getEntryTimeBegin() const {
        return m_entryTimeBegin.get();
    }
    const int &getEntryService() const {
        return m_entryService.get();
    }
    const bool &getEntryMonitoring() const {
        return m_entryMonitoring.get();
    }
    const unsigned long long &getEntryId() const {
        return m_entryId.get();
    }
/*
    ModelRequest *getEntry() {
        return entry.getRelated(this);
    }
    */
    const std::string &getContent() const {
        return m_content.get();
    }
    const bool &getIsResponse() const {
        return m_isResponse.get();
    }
    void setEntryTimeBegin(const Database::DateTime &entryTimeBegin) {
        m_entryTimeBegin = entryTimeBegin;
    }
    void setEntryService(const int &entryService) {
        m_entryService = entryService;
    }
    void setEntryMonitoring(const bool &entryMonitoring) {
        m_entryMonitoring = entryMonitoring;
    }
    void setEntryId(const unsigned long long &entryId) {
        m_entryId = entryId;
    }
    /*
    void setEntry(ModelRequest *foreign_value) {
        entry.setRelated(this, foreign_value);
    }
    */
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

    typedef Model::Field::List<ModelRequestData>  field_list;
    static const field_list& getFields() {
        return fields;
    }

protected:
    Field::Field<Database::DateTime> m_entryTimeBegin;
    Field::Field<int> m_entryService;
    Field::Field<bool> m_entryMonitoring;
    Field::Field<unsigned long long> m_entryId;
    Field::Field<std::string> m_content;
    Field::Field<bool> m_isResponse;

    //Field::Lazy::Field<ModelRequest *> m_entry;

public:
    static Model::Field::Basic<ModelRequestData, Database::DateTime> entryTimeBegin;
    static Model::Field::Basic<ModelRequestData, int> entryService;
    static Model::Field::Basic<ModelRequestData, bool> entryMonitoring;
    //static Model::Field::ForeignKey<ModelRequestData, unsigned long long, ModelRequest> entryId;
    static Model::Field::Basic<ModelRequestData, std::string> content;
    static Model::Field::Basic<ModelRequestData, bool> isResponse;

    //static Model::Field::Related::OneToOne<ModelRequestData, unsigned long long, ModelRequest> entry;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequestData

#endif // _MODEL_REQUESTDATA_H_

