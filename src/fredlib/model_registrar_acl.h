#ifndef MODEL_REGISTRAR_ACL_H_
#define MODEL_REGISTRAR_ACL_H_

#include "db_settings.h"
#include "model.h"

class ModelRegistrarAcl : public Model::Base {
public:
  ModelRegistrarAcl() { }
  virtual ~ModelRegistrarAcl() { }


  const unsigned long long& getId() const {
    return id_.get();
  }


  void setId(const unsigned long long &_id) {
    id_ = _id;
  }


  const unsigned long long& getRegistarId() const {
    return registrar_id_.get();
  }


  void setRegistrarId(const unsigned long long &_registrar_id) {
    registrar_id_ = _registrar_id;
  }


  const std::string& getCert() const {
    return cert_.get();
  }


  void setCert(const std::string &_cert) {
    cert_ = _cert;
  }


  const std::string& getPassword() const {
    return password_.get();
  }


  void setPassword(const std::string &_password) {
    password_ = _password;
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
  Field::Field<unsigned long long>   id_;
  Field::Field<unsigned long long>   registrar_id_;
  Field::Field<std::string>          cert_;
  Field::Field<std::string>          password_;


  typedef Model::Field::List<ModelRegistrarAcl>  field_list;


public:
  static Model::Field::PrimaryKey<ModelRegistrarAcl, unsigned long long>            id;
  static Model::Field::Basic<ModelRegistrarAcl, unsigned long long> registrar_id;
  static Model::Field::Basic<ModelRegistrarAcl, std::string>                        cert;
  static Model::Field::Basic<ModelRegistrarAcl, std::string>                        password;


    static const field_list &getFields()
    {
        return fields;
    }


private:
  static std::string table_name;
  static field_list  fields;
};



#endif /*MODEL_REGISTRAR_H_*/

