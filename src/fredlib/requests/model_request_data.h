#ifndef _MODEL_REQUESTDATA_H_
#define _MODEL_REQUESTDATA_H_

#include "db_settings.h"
#include "model.h"

class ModelRequestData:
    public Model::Base {
public:
    ModelRequestData()
    { }
    virtual ~ModelRequestData()
    { }
    const unsigned long long &getId() const {
        return m_id.get();
    }
    const Database::DateTime &getRequestTimeBegin() const {
        return m_requestTimeBegin.get();
    }
    const int &getRequestServiceId() const {
        return m_requestServiceId.get();
    }
    const bool &getRequestMonitoring() const {
        return m_requestMonitoring.get();
    }
    const unsigned long long &getRequestId() const {
        return m_requestId.get();
    }
/*
    ModelRequest *getRequest() {
        return request.getRelated(this);
    }
    */
    const std::string &getContent() const {
        return m_content.get();
    }
    const bool &getIsResponse() const {
        return m_isResponse.get();
    }
    void setId(const unsigned long long &id) {
        m_id = id;
    }
    void setRequestTimeBegin(const Database::DateTime &requestTimeBegin) {
        m_requestTimeBegin = requestTimeBegin;
    }
    void setRequestServiceId(const int &requestServiceId) {
        m_requestServiceId = requestServiceId;
    }
    void setRequestMonitoring(const bool &requestMonitoring) {
        m_requestMonitoring = requestMonitoring;
    }
    void setRequestId(const unsigned long long &requestId) {
        m_requestId = requestId;
    }
    /*
    void setRequest(ModelRequest *foreign_value) {
        request.setRelated(this, foreign_value);
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
        Model::Base::insert(this);
    }

    void update() {
        Database::Connection conn = Database::Manager::acquire();
        Model::Base::update(this);
    }

    void reload() {
        Database::Connection conn = Database::Manager::acquire();
        Model::Base::reload(this);
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
    Field::Field<unsigned long long> m_id;
    Field::Field<Database::DateTime> m_requestTimeBegin;
    Field::Field<int> m_requestServiceId;
    Field::Field<bool> m_requestMonitoring;
    Field::Field<unsigned long long> m_requestId;
    Field::Field<std::string> m_content;
    Field::Field<bool> m_isResponse;

    //Field::Lazy::Field<ModelRequest *> m_request;

public:
    static Model::Field::PrimaryKey<ModelRequestData, unsigned long long> id;
    static Model::Field::Basic<ModelRequestData, Database::DateTime> requestTimeBegin;
    static Model::Field::Basic<ModelRequestData, int> requestServiceId;
    static Model::Field::Basic<ModelRequestData, bool> requestMonitoring;
    static Model::Field::Basic<ModelRequestData, unsigned long long> requestId;
    static Model::Field::Basic<ModelRequestData, std::string> content;
    static Model::Field::Basic<ModelRequestData, bool> isResponse;

    //static Model::Field::Related::OneToOne<ModelRequestData, unsigned long long, ModelRequest> request;

private:
    static std::string table_name;  /** < model table name */
    static field_list  fields;      /** < list of all model fields */
}; // class ModelRequestData

#endif // _MODEL_REQUESTDATA_H_

