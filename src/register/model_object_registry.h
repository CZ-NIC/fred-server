#ifndef MODEL_OBJECT_REGISTRY_H_
#define MODEL_OBJECT_REGISTRY_H_

#include "db_settings.h"
#include "model.h"
#include "model_registrar.h"


class ModelObjectRegistry : public Model::Base {
public:
  ModelObjectRegistry() { }
  virtual ~ModelObjectRegistry() { }


  const unsigned long long& getId() const {
    return id_.get();
  }


  void setId(const unsigned long long &_id) {
    id_ = _id;
  }


  const std::string& getRoid() const {
    return roid_.get();
  }


  void setRoid(const std::string &_roid) {
    roid_ = _roid;
  }


  const int& getType() const {
    return type_.get();
  }

  
  void setType(const int &_type) {
    type_ = _type;
  }


  const std::string& getName() const {
    return name_.get();
  }


  void setName(const std::string &_name) {
    name_ = _name;
  }


  const ptime& getCrdate() const {
    return crdate_.get();
  }


  void setCrdate(const ptime &_crdate) {
    crdate_ = _crdate;
  }


  const ptime& getErdate() const {
    return erdate_.get();
  }


  void setErdate(const ptime &_erdate) {
    erdate_ = _erdate;
  }


  const unsigned long long& getCrid() const {
    return crid_.get();
  }


  void setCrid(const unsigned long long &_crid) {
    cr.setRelatedTo(this, _crid);
  }


  ModelRegistrar* getCr() {
    return cr.getRelated(this);
  }


  void setCr(ModelRegistrar *_cr) {
    cr.setRelated(this, _cr);
  }




  friend class Model::Base;

  void insert() {
    Model::Base::insert(this);
  }


  void update() {
    Model::Base::update(this);
  }


    void reload()
    {
        Database::Connection c = Database::Manager::acquire();
        Database::Transaction transaction(c);
        Model::Base::reload(this);
        transaction.commit();
    }



protected:
  Field::Field<unsigned long long>    id_;
  Field::Field<std::string>           roid_;
  Field::Field<int>                   type_;
  Field::Field<std::string>           name_;
  Field::Field<unsigned long long>    crid_;
  Field::Field<ptime>                 crdate_;
  Field::Field<ptime>                 erdate_;

  Field::Lazy::Field<ModelRegistrar*>            cr_;


  typedef Model::Field::List<ModelObjectRegistry>  field_list;


public:
  static Model::Field::PrimaryKey<ModelObjectRegistry, unsigned long long>            id;
  static Model::Field::Basic<ModelObjectRegistry, std::string>                        roid;
  static Model::Field::Basic<ModelObjectRegistry, int>                                type;
  static Model::Field::Basic<ModelObjectRegistry, std::string>                        name;
  static Model::Field::ForeignKey<ModelObjectRegistry, unsigned long long, ModelRegistrar> crid;
  static Model::Field::Basic<ModelObjectRegistry, ptime>                              crdate;
  static Model::Field::Basic<ModelObjectRegistry, ptime>                              erdate;

  static Model::Field::Related::OneToOne<ModelObjectRegistry, unsigned long long, ModelRegistrar> cr;

    static const field_list &getFields()
    {
        return fields;
    }


private:
  static std::string table_name;
  static field_list  fields;
};


#endif /*MODEL_OBJECT_REGISTRY_H_*/

