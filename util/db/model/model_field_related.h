#ifndef MODEL_FIELD_RELATED_H_
#define MODEL_FIELD_RELATED_H_

#include "model_field.h"
#include "log/logger.h"


namespace Model {
namespace Field {
namespace Related {


template<class _class, class _type, class _class_ref>
class OneToOne {
public:
  OneToOne(ForeignKey<_class, _type, _class_ref> &_fk,
           const MemPtr<_class, ::Field::Field<_type> > &_related_to,
           const MemPtr<_class, ::Field::Lazy::Field<_class_ref*> > &_related) 
         : fk_(_fk),
           related_to_(_related_to),
           related_(_related) {
  }


  virtual ~OneToOne() {
  }


  void setRelatedTo(_class *_obj, const _type &_value) {
    related_to_(*_obj) = _value;
    related_(*_obj) = 0;
  }


  void setRelated(_class *_obj, _class_ref *_value) {
    related_(*_obj) = _value;
    related_to_(*_obj) = fk_.getReferencedField().getValue(_value);
  }


  _class_ref* getRelated(_class *_obj) {
    LOGGER(PACKAGE).debug("<CALL> getRelated()");
    if (related_to_(*_obj).isSet() && related_(*_obj).get() == 0) {
      LOGGER(PACKAGE).debug("<CALL> getRelated() condition true");

      _class_ref *data = new _class_ref();
      data->load(data, related_to_(*_obj).get());
      setRelated(_obj, data);
    }
    return related_(*_obj);
  }


protected:
  ForeignKey<_class, _type, _class_ref>               &fk_;
  MemPtr<_class, ::Field::Field<_type> >               related_to_;
  MemPtr<_class, ::Field::Lazy::Field<_class_ref*> >   related_;
};



template<class _class, class _type, class _class_ref>
class OneToMany {
public:
  OneToMany(ForeignKey<_class_ref, _type, _class> &_fk,
            const MemPtr<_class, ::Field::Lazy::List<_class_ref> > &_related)
          : fk_(_fk),
            related_(_related) {
  }


  virtual ~OneToMany() {
  }


  void addRelated(_class *_obj, _class_ref *_value) {
    related_(*_obj).push_back(_value);
  }


  void getRelated(const _class *_obj) {
    /* do lazy list load propably based on filter */
  }


protected:
  ForeignKey<_class_ref, _type, _class>              &fk_;
  MemPtr<_class, ::Field::Lazy::List<_class_ref> >    related_;
};

}
}
}


#endif /*MODEL_FIELD_RELATED_H_*/

