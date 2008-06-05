#ifndef SIMPLE_FILTERS_H_
#define SIMPLE_FILTERS_H_

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>

#include "simple_filter.h"
#include "dbexceptions.h"
#include "log/logger.h"
#include "util.h"

namespace DBase {
namespace Filters {

// TODO Null value
template<class Tp> class Interval : public Simple {
public:
  Interval(const Column& _col, const Tp& _value_beg, const Tp& _value_end,
      const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col), value_beg(_value_beg), value_end(_value_end) {
  }
  virtual ~Interval() {
  }

  virtual void setValueBeg(const Tp& _value) {
    TRACE("[CALL] Interval::setValueBeg()");
    active = true;
    value_beg = _value;
  }

  virtual void setValueEnd(const Tp& _value) {
    TRACE("[CALL] Interval::setValueEnd()");
    active = true;
    value_end = _value;
  }

  Interval<Tp>* clone() const {
    return 0;
  }

  virtual void serialize(DBase::SelectQuery& _sq) {
    std::stringstream &prep = _sq.where_prepared_string();
    std::vector<std::string> &store = _sq.where_prepared_values();

    prep << getConjuction() << "( ";
    prep << column.str() << SQL_OP_GE << "'%" << store.size() + 1 << "%'";
    store.push_back(Util::stream_cast<std::string>(value_beg));
    prep << SQL_OP_AND << column.str() << SQL_OP_LT << "'" << store.size() + 1
        << "' )";
    store.push_back(Util::stream_cast<std::string>(value_end));
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Simple);
    _ar & BOOST_SERIALIZATION_NVP(column);
    _ar & BOOST_SERIALIZATION_NVP(value_beg);
    _ar & BOOST_SERIALIZATION_NVP(value_end);
  }

protected:
  Column column;
  Tp value_beg;
  Tp value_end;
};

template<class DTp> class _BaseDTInterval : public Simple {
public:
  virtual ~_BaseDTInterval() {
  }

  virtual void setValue(const DTp& _value) {
    TRACE("[CALL] _BaseDTInterval::setValue()");
    active = true;
    value = _value;
  }

  virtual const DTp& getValue() const {
    return value.getValue();
  }

  virtual _BaseDTInterval<DTp>* clone() const {
    return 0;
  }

  virtual void serialize(DBase::SelectQuery& _sq) {
    TRACE("[CALL] _BaseDTInterval::serialize()");
    std::stringstream &prep = _sq.where_prepared_string();
    std::vector<std::string> &store = _sq.where_prepared_values();
    const DTp& t_value = value.getValue();

    if (value.isNull()) {
      LOGGER("tracer").trace("[IN] _BaseDTInterval::serialize(): value is 'NULL'");
      prep << getConjuction() << "( ";
      prep << column.str() << SQL_OP_IS << value;
      prep << " )";
    } else if (t_value.isSpecial() && (t_value.getSpecial() != DAY
        && t_value.getSpecial() != INTERVAL)) {
      LOGGER("tracer").trace(boost::format("[IN] _BaseDTInterval::serialize(): value is special (special_flag='%1%')")
          % t_value.getSpecial());
      std::stringstream beg, end;
      std::string what;

      switch (t_value.getSpecial()) {
      case LAST_HOUR:
      case PAST_HOUR:
        what = "hour";
        break;
      case LAST_DAY:
      case PAST_DAY:
        what = "day";
        break;
      case LAST_WEEK:
      case PAST_WEEK:
        what = "week";
        break;
      case LAST_MONTH:
      case PAST_MONTH:
        what = "month";
        break;
      case LAST_YEAR:
      case PAST_YEAR:
        what = "year";
        break;
      default:
        throw Exception("Interval: not valid special value");
      }

      if (t_value.getSpecial() < PAST_HOUR) {
        beg << "date_trunc('" << what << "', current_timestamp + interval '"
            << t_value.getSpecialOffset() << " " << what <<"')";
        end << beg.str() << " + interval '1 "<< what << "'";
      } else {
        if (t_value.getSpecialOffset() < 0) {
          beg << "current_timestamp + interval '" << t_value.getSpecialOffset()
              << " " << what << "'";
          end << "current_timestamp";
        } else {
          end << "current_timestamp + interval '" << t_value.getSpecialOffset()
              << " " << what <<"'";
          beg << "current_timestamp";
        }

      }
      prep << getConjuction() << "( ";
      prep << column.str() << SQL_OP_GE << beg.str();
      prep << SQL_OP_AND << column.str() << SQL_OP_LT << end.str();
      prep << " )";
    } else {
      LOGGER("tracer").trace(boost::format("[IN] _BaseDTInterval::serialize(): value is normal (special_flag='%1%')")
          % t_value.getSpecial());
      bool b = false;

      if (!t_value.begin().is_special()) {
        prep << getConjuction() << "( ";
        prep << column.str() << SQL_OP_GE << "'%" << store.size() + 1 << "%'";
        store.push_back(t_value.begin().iso_str());
        b = true;
      }
      if (!t_value.end().is_special()) {
        prep << (b ? SQL_OP_AND : getConjuction() + "( ") << column.str()
            << SQL_OP_LT << "'%" << store.size() + 1 << "%'";
        prep << " )";
        store.push_back(t_value.end().iso_str());
      } else if (b) {
        prep << " )";
      }
    }
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Simple);
    _ar & BOOST_SERIALIZATION_NVP(column);
    _ar & BOOST_SERIALIZATION_NVP(value);
  }

public:
  _BaseDTInterval(const Column& _col, const DBase::Null<DTp>& _value,
      const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col), value(_value) {
  }
  _BaseDTInterval(const Column& _col, const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col) {
  }

  _BaseDTInterval() {
  }

protected:
  Column column;
  DBase::Null<DTp> value;
};

#define BOOST_SERIALIZATION_BASE_TEMPLATE(name, base)               \
  boost::serialization::make_nvp(name,                                     \
  boost::serialization::base_object<base >(*this));

template<> class Interval<DateTimeInterval> :
  public _BaseDTInterval<DateTimeInterval> {
public:
  Interval(const Column& _col, const DBase::Null<DateTimeInterval>& _value,
      const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateTimeInterval>(_col, _value, _conj) {
    column.castTo("timestamptz", "CET");
  }
  Interval(const Column& _col, const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateTimeInterval>(_col, _conj) {
    column.castTo("timestamptz", "CET");
  }
  
  Interval() {
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_TEMPLATE(
        "_BaseDTInterval_DateTimeInterval",
        _BaseDTInterval<DateTimeInterval>);
  }
};

template<> class Interval<DateInterval> : public _BaseDTInterval<DateInterval> {
public:
  Interval(const Column& _col, const DBase::Null<DateInterval>& _value,
      const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateInterval>(_col, _value, _conj) {
  }
  Interval(const Column& _col, const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateInterval>(_col, _conj) {
  }
  
  Interval() {
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_TEMPLATE(
        "_BaseDTInterval_DateInterval",
        _BaseDTInterval<DateInterval>);
  }
};

template<class Tp> class Value : public Simple {
public:
  Value(const Column& _col, const Null<Tp>& _value,
      const std::string& _op = SQL_OP_EQ, const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col), op(_op), value(_value) {
  }
  Value(const Column& _col, const std::string& _op = SQL_OP_EQ, 
        const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col), op(_op) {
  }

  Value() {
  }

  virtual ~Value() {
  }

  virtual void setOperator(const std::string& _op) {
    op = _op;
  }

  virtual void setValue(const Null<Tp>& _value) {
    TRACE("[CALL] Value<Tp>::setValue()");
    active = true;
    value = _value;
  }

  virtual const Null<Tp>& getValue() const {
    return value;
  }

  virtual Value<Tp>* clone() const {
    return 0;
  }

  virtual void serialize(DBase::SelectQuery& _sq) {
    std::stringstream &prep = _sq.where_prepared_string();
    std::vector<std::string> &store = _sq.where_prepared_values();

    prep << getConjuction() << "( " << column.str();
    if (value.isNull()) {
      prep << SQL_OP_IS << value;
    } else {
      prep << op << "'%" << store.size() + 1 << "%'";
      store.push_back(Util::stream_cast<std::string>(value.getValue()));
    }
    prep << " )";
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Simple);
    _ar & BOOST_SERIALIZATION_NVP(column);
    _ar & BOOST_SERIALIZATION_NVP(op);
    _ar & BOOST_SERIALIZATION_NVP(value);
  }

protected:
  Column column;
  std::string op;
  DBase::Null<Tp> value;
};

template<> class Value<std::string> : public Simple {
public:
  Value(const Column& _col, const Null<std::string>& _value,
      const std::string& _op = SQL_OP_EQ, const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col), op(_op), value(_value) {
  }
  Value(const Column& _col, const std::string& _op = SQL_OP_EQ, 
        const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col), op(_op) {
  }

  Value() {
  }

  virtual ~Value() {
  }

  virtual void setValue(const std::string& _value) {
    TRACE("[CALL] Value<std::sting>::setValue()");
    active = true;
    if (op == SQL_OP_LIKE) {
      value = "%%" + _value + "%%";
    }
    else {
      value = _value;
    }
  }

  virtual const std::string& getValue() const {
    return value.getValue();
  }

  virtual Value<std::string>* clone() const {
    return 0;
  }

  virtual void serialize(DBase::SelectQuery& _sq) {
    std::stringstream &prep = _sq.where_prepared_string();
    std::vector<std::string> &store = _sq.where_prepared_values();

    prep << getConjuction() << "( " << column.str();
    if (value.isNull()) {
      prep << SQL_OP_IS << value;
    } 
    else {
      std::string v = value.getValue();
      if (allowed_wildcard && (v.find('*') != std::string::npos || v.find('?') != std::string::npos)) {
        prep << " ILIKE TRANSLATE('%" << store.size() + 1 << "%','*?','%%_')";
        store.push_back(v);
      }
      else {
        prep << op << "'%" << store.size() + 1 << "%'";
        store.push_back(v);
      }
    }
    prep << " )";
  }

  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_OBJECT_NVP(Simple);
    _ar & BOOST_SERIALIZATION_NVP(column);
    _ar & BOOST_SERIALIZATION_NVP(op);
    _ar & BOOST_SERIALIZATION_NVP(value);
  }

protected:
  Column column;
  std::string op;
  DBase::Null<std::string> value;
};

template<class Tp> class Null : public Simple {
public:
  Null(const Column& _col, const std::string& _op = SQL_OP_IS,
      const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col) {
  }

  virtual ~Null() {
  }

  virtual void serialize(DBase::SelectQuery& _sq) {
    _sq.where_prepared_string() << getConjuction() << column.str() << SQL_OP_IS
        << "NULL";
  }
protected:
  Column column;
  DBase::Null<Tp> value;
};

}
}

#endif /*SIMPLE_FILTERS_H_*/
