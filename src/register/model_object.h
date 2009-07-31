#ifndef MODEL_OBJECT_H_
#define MODEL_OBJECT_H_

#include "db_settings.h"
#include "model_object_registry.h"
#include "model.h"

class ModelBObject : public ModelObjectRegistry {
public:
  ModelBObject() { }
  virtual ~ModelBObject() { }

  const unsigned long long getClid() const {
    return clid_.get();
  }


  void setClid(const unsigned long long &_clid) {
    clid_ = _clid;
  }


  const unsigned long long getUpid() const {
    return upid_.get();
  }


  void setUpid(const unsigned long long &_upid) {
    upid_ = _upid;
  }


  const ptime& getTrdate() const {
    return tr_date_.get();
  }


  void setTrdate(const ptime &_trdate) {
    tr_date_ = _trdate;
  }


  const ptime& getUpdate() const {
    return up_date_.get();
  }


  void setUpdate(const ptime &_update) {
    up_date_ = _update;
  }


  const std::string& getAuthinfopw() const {
    return authinfopw_.get();
  }


  void setAuthinfopw(const std::string &_authinfopw) {
    authinfopw_ = _authinfopw;
  }


protected:
  Field::Field<unsigned long long>    clid_;
  Field::Field<unsigned long long>    upid_;
  Field::Field<ptime>                 tr_date_;
  Field::Field<ptime>                 up_date_;
  Field::Field<std::string>           authinfopw_;


public:
  static const field_list getFields()
  {
    return fields;
  }

private:
  static std::string table_name;
  static field_list  fields;
};



class ModelObject : public ModelBObject {
public:
  friend class Model::Base;

  void insert() {
    Database::Connection c = Database::Manager::acquire();
    Database::Transaction tx(c);
    ModelObjectRegistry::insert();
    Model::Base::insert(this);
    tx.commit();
  }


  void update() {
    Database::Connection c = Database::Manager::acquire();
    Database::Transaction tx(c);
    ModelObjectRegistry::update();
    Model::Base::update(this);
    tx.commit();
  }

    void reload()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction transaction(c);
        Model::Base::reload(this);
        transaction.commit();
    }

  static Model::Field::PrimaryKey<ModelObject, unsigned long long>             id;
  static Model::Field::ForeignKey<ModelObject, unsigned long long, ModelRegistrar>  clid;
  static Model::Field::ForeignKey<ModelObject, unsigned long long, ModelRegistrar>  upid;
  static Model::Field::Basic<ModelObject, ptime>                               tr_date;
  static Model::Field::Basic<ModelObject, ptime>                               up_date;
  static Model::Field::Basic<ModelObject, std::string>                         authinfopw;


protected:
  typedef Model::Field::List<ModelObject>  field_list;


public:
  static const field_list getFields()
  {
    return fields;
  }

private:
  static std::string table_name;
  static field_list  fields;
};



class ModelObjectHistory : public ModelBObject {
public:
  const unsigned long long getHistoryId() const {
    return history_id_.get();
  }


protected:
  Field::Field<unsigned long long>    history_id_;


public:
  friend class Model::Base;

  void insert() {
    ModelObjectRegistry::insert();
    Model::Base::insert(this);
  }


  void update() {
    ModelObjectRegistry::update();
    Model::Base::update(this);
  }

    void reload()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction transaction(c);
        Model::Base::reload(this);
        transaction.commit();
    }


  static Model::Field::PrimaryKey<ModelObjectHistory, unsigned long long> history_id;
  static Model::Field::ForeignKey<ModelObjectHistory, unsigned long long, ModelObjectRegistry> id;
  static Model::Field::ForeignKey<ModelObjectHistory, unsigned long long, ModelRegistrar>  clid;
  static Model::Field::ForeignKey<ModelObjectHistory, unsigned long long, ModelRegistrar>  upid;
  static Model::Field::Basic<ModelObjectHistory, ptime>                               tr_date;
  static Model::Field::Basic<ModelObjectHistory, ptime>                               up_date;
  static Model::Field::Basic<ModelObjectHistory, std::string>                         authinfopw;



protected:
  typedef Model::Field::List<ModelObjectHistory>  field_list;

public:
  static const field_list getFields()
  {
    return fields;
  }

private:
  static std::string table_name;
  static field_list  fields;
};


#endif /*MODEL_OBJECT_H_*/

