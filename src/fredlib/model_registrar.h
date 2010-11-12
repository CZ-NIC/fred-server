#ifndef MODEL_REGISTRAR_H_
#define MODEL_REGISTRAR_H_

#include "db_settings.h"
#include "model.h"

//class ModelRegistrarAcl;
//class ModelObject;

class ModelRegistrar : public Model::Base {
public:
  ModelRegistrar(); 
  virtual ~ModelRegistrar();

  const unsigned long long& getId() const {
    return id_.get();
  }


  void setId(const unsigned long long &_id) {
    id_ = _id;
  }


  const std::string& getIco() const {
    return ico_.get();
  }


  void setIco(const std::string &_ico) {
    ico_ = _ico;
  }


  const std::string& getDic() const {
    return dic_.get();
  }


  void setDic(const std::string &_dic) {
    dic_ = _dic;
  }


  const std::string& getVarsymb() const {
    return varsymb_.get();
  }


  void setVarsymb(const std::string &_varsymb) {
    varsymb_ = _varsymb;
  }


  const bool& getVat() const {
    return vat_.get();
  }


  void setVat(const bool &_vat) {
    vat_ = _vat;
  }


  const std::string& getHandle() const {
    return handle_.get();
  }


  void setHandle(const std::string &_handle) {
    handle_ = _handle;
  }


  const std::string& getName() const {
    return name_.get();
  }


  void setName(const std::string &_name) {
    name_ = _name;
  }


  const std::string& getOrganization() const {
    return organization_.get();
  }


  void setOrganization(const std::string &_organization) {
    organization_ = _organization;
  }


  const std::string& getStreet1() const {
    return street1_.get();
  }


  void setStreet1(const std::string &_street1) {
    street1_ = _street1;
  }


  const std::string& getStreet2() const {
    return street2_.get();
  }


  void setStreet2(const std::string &_street2) {
    street2_ = _street2;
  }


  const std::string& getStreet3() const {
    return street3_.get();
  }


  void setStreet3(const std::string &_street3) {
    street3_ = _street3;
  }


  const std::string& getCity() const {
    return city_.get();
  }


  void setCity(const std::string &_city) {
    city_ = _city;
  }


  const std::string& getStateorprovince() const {
    return stateorprovince_.get();
  }


  void setStateorprovince(const std::string &_stateorprovince) {
    stateorprovince_ = _stateorprovince;
  }


  const std::string& getPostalcode() const {
    return postalcode_.get();
  }


  void setPostalcode(const std::string &_postalcode) {
    postalcode_ = _postalcode;
  }


  const std::string& getCountry() const {
    return country_.get();
  }


  void setCountry(const std::string &_country) {
    country_ = _country;
  }


  const std::string& getTelephone() const {
    return telephone_.get();
  }


  void setTelephone(const std::string &_telephone) {
    telephone_ = _telephone;
  }


  const std::string& getFax() const {
    return fax_.get();
  }


  void setFax(const std::string &_fax) {
    fax_ = _fax;
  }


  const std::string& getEmail() const {
    return email_.get();
  }


  void setEmail(const std::string &_email) {
    email_ = _email;
  }


  const std::string& getUrl() const {
    return url_.get();
  }


  void setUrl(const std::string &_url) {
    url_ = _url;
  }


  const bool& getSystem() const {
    return system_.get();
  }


  void setSystem(const bool &_system) {
    system_ = _system;
  }

/*
  void addAcl(ModelRegistrarAcl* _acl) {
    acls.addRelated(this, _acl);
  }
*/

#if 0
  const Field::Lazy::List<ModelRegistrarAcl>& getAcls() const {
    acls.getRelated(this);
    return acls_;
  }
#endif


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
  Field::Field<unsigned long long>  id_;
  Field::Field<std::string>         ico_;
  Field::Field<std::string>         dic_;
  Field::Field<std::string>         varsymb_;
  Field::Field<bool>                vat_;
  Field::Field<std::string>         handle_;
  Field::Field<std::string>         name_;
  Field::Field<std::string>         organization_;
  Field::Field<std::string>         street1_;
  Field::Field<std::string>         street2_;
  Field::Field<std::string>         street3_;
  Field::Field<std::string>         city_;
  Field::Field<std::string>         stateorprovince_;
  Field::Field<std::string>         postalcode_;
  Field::Field<std::string>         country_;
  Field::Field<std::string>         telephone_;
  Field::Field<std::string>         fax_;
  Field::Field<std::string>         email_;
  Field::Field<std::string>         url_;
  Field::Field<bool>                system_;

  //Field::Lazy::List<ModelObject>         cl_objects_;
  //Field::Lazy::List<ModelRegistrarAcl>   acls_;
 

  
  typedef Model::Field::List<ModelRegistrar>  field_list;


public:
  static Model::Field::PrimaryKey<ModelRegistrar, unsigned long long>  id;
  static Model::Field::Basic<ModelRegistrar, std::string>              ico;
  static Model::Field::Basic<ModelRegistrar, std::string>              dic;
  static Model::Field::Basic<ModelRegistrar, std::string>              varsymb;
  static Model::Field::Basic<ModelRegistrar, bool>                     vat;
  static Model::Field::Basic<ModelRegistrar, std::string>              handle;
  static Model::Field::Basic<ModelRegistrar, std::string>              name;
  static Model::Field::Basic<ModelRegistrar, std::string>              organization;
  static Model::Field::Basic<ModelRegistrar, std::string>              street1;
  static Model::Field::Basic<ModelRegistrar, std::string>              street2;
  static Model::Field::Basic<ModelRegistrar, std::string>              street3;
  static Model::Field::Basic<ModelRegistrar, std::string>              city;
  static Model::Field::Basic<ModelRegistrar, std::string>              stateorprovince;
  static Model::Field::Basic<ModelRegistrar, std::string>              postalcode;
  static Model::Field::Basic<ModelRegistrar, std::string>              country;
  static Model::Field::Basic<ModelRegistrar, std::string>              telephone;
  static Model::Field::Basic<ModelRegistrar, std::string>              fax;
  static Model::Field::Basic<ModelRegistrar, std::string>              email;
  static Model::Field::Basic<ModelRegistrar, std::string>              url;
  static Model::Field::Basic<ModelRegistrar, bool>                     system;

  //static Model::Field::Related::OneToMany<ModelRegistrar, unsigned long long, ModelObject>   cl_objects;
  //static Model::Field::Related::OneToMany<ModelRegistrar, unsigned long long, ModelRegistrarAcl>   acls;


    static const field_list &getFields()
    {
        return fields;
    }

private:
  static std::string table_name;
  static field_list  fields;
};



#endif /*MODEL_REGISTRAR_H_*/

