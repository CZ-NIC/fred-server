#ifndef NULL_H_
#define NULL_H_

namespace Database {

template<class Tp> Tp& SQLNULL(Tp& _obj) {
  _obj.null();
  return _obj;
}

class NullStrRep {
public:
  static std::ostream& toStr(std::ostream& _os) {
    return _os << "NULL";
  }
};

template<class Tp, class Rep =NullStrRep> class Null {
public:
  Null(const Tp& _value, bool _null = false) :
    is_null(_null), t_value(_value) {
  }
  Null() :
    is_null(true), t_value() {
  }
  Null(const Null<Tp>& _src) {
    t_value = _src.t_value;
    is_null = _src.is_null;
  }
  virtual ~Null() {
  }
  friend std::ostream& operator<<(std::ostream& _os, const Null<Tp>& _v) {
    if (_v.is_null) {
      return Rep::toStr(_os);
    } else {
      return _os << _v.t_value;
    }
  }
  virtual bool isNull() const {
    return is_null;
  }
  virtual void null() {
    is_null = true;
  }
  virtual void setNull(bool _n) {
    is_null = _n;
  }
  virtual void setValue(const Tp& _v) {
    t_value = _v;
  }
  virtual const Tp& getValue() const {
    return t_value;
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_NVP(is_null);
    _ar & BOOST_SERIALIZATION_NVP(t_value);
  }

protected:
  bool is_null;
  Tp t_value;
};

template<> class Null<void, NullStrRep> {
public:
  Null(bool _null = true) :
    is_null(_null) {
  }
  virtual ~Null() {
  }
  friend std::ostream& operator<<(std::ostream& _os, const Null<void>& _v) {
    return NullStrRep::toStr(_os);
  }
  virtual bool isNull() {
    return is_null;
  }
  virtual void null() {
    is_null = true;
  }

protected:
  bool is_null;
};

}

#endif /*NULL_H_*/

