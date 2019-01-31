#ifndef SIMPLE_FILTERS_HH_9AB05463EDE54FDF8DCA9C0D0F17307C
#define SIMPLE_FILTERS_HH_9AB05463EDE54FDF8DCA9C0D0F17307C

#include <boost/serialization/nvp.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/version.hpp>
#include <boost/serialization/string.hpp>

#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/local_timezone_defs.hpp>
#include <boost/date_time/local_time_adjustor.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

#include "src/util/db/query/simple_filter.hh"
#include "src/util/types/null.hh"
#include "util/log/logger.hh"
#include "util/util.hh"
#include "util/base_exception.hh"
#include "util/types/convert_sql_db_types.hh"

namespace Database {
namespace Filters {

// TODO Null value
template<class Tp> class Interval : public Simple {
public:
  Interval(const Column& _col, const Tp& _value_beg, const Tp& _value_end,
      const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col), value_beg(_value_beg), value_end(_value_end), beg_set(false),
    end_set(false) {
  }

  Interval(const Column& _col, const std::string& _conj = SQL_OP_AND) : Simple(_conj),
                                                                        column(_col),
                                                                        beg_set(false),
                                                                        end_set(false) {
  }
/*
  /// copy constructor
  Interval(const Interval<Tp>& i) :
      Simple(i),
      column(i.column),
      value_beg(i.value_beg), value_end(i.value_end),
      beg_set(i.beg_set), end_set(i.end_set),
      column(i.column){
  }
*/

  virtual ~Interval() {
  }

  virtual void setValueBeg(const Tp& _value) {
    TRACE("[CALL] Interval::setValueBeg()");
    active = true;
    value_beg = _value;
    beg_set = true;
  }

  virtual void setValueEnd(const Tp& _value) {
    TRACE("[CALL] Interval::setValueEnd()");
    active = true;
    value_end = _value;
    end_set = true;
  }

  virtual void setValue(const Tp& _beg, const Tp& _end) {
    TRACE("[CALL] Interval::setValue()");
    active = true;
    value_beg = _beg;
    value_end = _end;
    beg_set = end_set = true;
  }

  void setColumn(const Column &c) {
    column = c;
  }

  virtual Tp getValueBeg() const {
    return value_beg;
  }

  virtual Tp getValueEnd() const {
    return value_end;
  }

  Interval<Tp>* clone() const {
    return 0;
  }

  virtual void serialize(Database::SelectQuery& _sq, const Settings *_settings) {
    Database::SelectQuery::prepared_values_string  &prep  = _sq.where_prepared_string();
    Database::SelectQuery::prepared_values_storage &store = _sq.where_prepared_values();

    prep << getConjuction() << "( ";
    if (beg_set) {
      prep << column.str() << SQL_OP_GE << "%" << store.size() + 1 << "%";
      store.push_back(value_beg);
    }
    if (beg_set && end_set) {
      prep << SQL_OP_AND;
    }
    if (end_set) {
      prep << column.str() << SQL_OP_LT << "%" << store.size() + 1 << "% )";
      store.push_back(value_end);
    }
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
  bool beg_set;
  bool end_set;
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

  void setColumn(const Column &c) {
    column = c;
  }

  virtual const DTp& getValue() const {
    return value.getValue();
  }

  virtual _BaseDTInterval<DTp>* clone() const {
    return 0;
  }
  
  virtual std::string special2str(DateTimeIntervalSpecial _spec) const {
    switch (_spec) {
    case LAST_HOUR:
    case PAST_HOUR:  return "hour";
    case LAST_DAY:
    case PAST_DAY:   return "day";
    case LAST_WEEK:
    case PAST_WEEK:  return "week";
    case LAST_MONTH:
    case PAST_MONTH: return "month";
    case LAST_YEAR:
    case PAST_YEAR:  return "year";
    default:
      throw Exception("Interval: not valid special value");
    }
  }

  virtual void serialize(Database::SelectQuery& _sq, const Settings *_settings, bool plain_date = false) {
    TRACE("[CALL] _BaseDTInterval::serialize()");
    Database::SelectQuery::prepared_values_string  &prep  = _sq.where_prepared_string();
    Database::SelectQuery::prepared_values_storage &store = _sq.where_prepared_values();
    const DTp& t_value = value.getValue();

    if (value.isNull()) {
      LOGGER.trace("[IN] _BaseDTInterval::serialize(): value is 'NULL'");
      prep << getConjuction() << "( ";
      prep << column.str() << SQL_OP_IS << value;
      prep << " )";
    } else {
      LOGGER.trace(boost::format("[IN] _BaseDTInterval::serialize(): value is normal (special_flag='%1%')")
          % t_value.getSpecial());
      
      std::string second_operator = (t_value.getSpecial() == INTERVAL ? SQL_OP_LE : SQL_OP_LT); 
      bool b = false;

      if (!t_value.begin().is_special()) {
        prep << getConjuction() << "( ";
        prep << column.str() << SQL_OP_GE << "(%" << store.size() + 1;

        if(plain_date) {
            prep << "%::date)" + value_post_;
        } else {
            prep << "%::timestamp AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC')" + value_post_;
        }

        store.push_back(t_value.begin());
        b = true;
      }
      if (!t_value.end().is_special()) {
        prep << (b ? SQL_OP_AND : getConjuction() + "( ") << column.str()
            << second_operator << "(%" << store.size() + 1;

        if(plain_date) {
            prep << "%::date)" + value_post_;
        } else {
            prep << "%::timestamp AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC')" + value_post_;
        }
        prep << " )";
        store.push_back(t_value.end());
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
  _BaseDTInterval(const Column& _col, const Database::Null<DTp>& _value,
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
  Database::Null<DTp> value;
};

#define BOOST_SERIALIZATION_BASE_TEMPLATE(name, base)    \
  boost::serialization::make_nvp(name, boost::serialization::base_object<base >(*this));

template<> class Interval<DateTimeInterval> :
  public _BaseDTInterval<DateTimeInterval> {
public:
  Interval(const Column& _col, const Database::Null<DateTimeInterval>& _value,
      const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateTimeInterval>(_col, _value, _conj) {
  }
  Interval(const Column& _col, const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateTimeInterval>(_col, _conj) {
  }
  
  Interval() {
  }
  
  void setNULL() {
    value = Database::Null<DateTimeInterval>();
    active = true;
  }
  
  void serialize(Database::SelectQuery& _sq, const Settings *_settings) {
    TRACE("[CALL] Interval<DateTime>::serialize()");
    Database::SelectQuery::prepared_values_string  &prep  = _sq.where_prepared_string();
    Database::SelectQuery::prepared_values_storage &store = _sq.where_prepared_values();

    const DateTimeInterval& t_value = value.getValue();
  
    if (!value.isNull() 
        && t_value.isSpecial() 
        && (t_value.getSpecial() != DAY && t_value.getSpecial() != INTERVAL)) {
      
      LOGGER.trace(boost::format("[IN] Interval<DateTime>::serialize(): value is special (special_flag='%1%')")
          % t_value.getSpecial());
      
      std::string time = (boost::format("'%1% Europe/Prague'::timestamp")
                % boost::posix_time::to_iso_string(microsec_clock::local_time())).str();
      
      std::stringstream beg, end;
      std::string what = special2str(t_value.getSpecial());
  
      if (t_value.getSpecial() < PAST_HOUR) {
        beg << "(date_trunc('" << what << "', " << time << " + interval '"
            << t_value.getSpecialOffset() << " " << what <<"')  AT TIME ZONE 'Europe/Prague'  AT TIME ZONE 'UTC')";
        end << "(" << beg.str() << " + interval '1 "<< what << "')";
        beg << value_post_;
        end << value_post_;
      } else {
        if (t_value.getSpecialOffset() < 0) {            
          beg << "((" << time << " + interval '" << t_value.getSpecialOffset()
              << " " << what << "') AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC')" + value_post_;
          end << "(" << time << " AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC')" + value_post_;
        } else {
          end << "((" << time << " + interval '" << t_value.getSpecialOffset()
              << " " << what <<"') AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC')" + value_post_;
          beg << "(" << time <<" AT TIME ZONE 'Europe/Prague'  AT TIME ZONE 'UTC')" + value_post_;

        }
  
      }
      prep << getConjuction() << "( ";
      prep << column.str() << SQL_OP_GE << beg.str();
      prep << SQL_OP_AND << column.str() << SQL_OP_LT << end.str();
      prep << " )";
    }
    else {
      /* notice: took from parent class and added time conversion */
      if (value.isNull()) {
        LOGGER.trace("[IN] _BaseDTInterval::serialize(): value is 'NULL'");
        prep << getConjuction() << "( ";
        prep << column.str() << SQL_OP_IS << value;
        prep << " )";
      } else {
        LOGGER.trace(boost::format("[IN] _BaseDTInterval::serialize(): value is normal (special_flag='%1%')")
            % t_value.getSpecial());
        
        std::string second_operator = (t_value.getSpecial() == INTERVAL ? SQL_OP_LE : SQL_OP_LT); 
        bool b = false;
  
        if (!t_value.begin().is_special()) {
          prep << getConjuction() << "( ";
          prep << column.str() << SQL_OP_GE << "('%" << store.size() + 1 << "%'::timestamp AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC')" + value_post_;
          // we want to format date in the DateTime in the right way but without quotes
          // so we need to bypass the Database::Value(DateTime) constructor
          std::string conversion  = SqlConvert<DateTime>::to(t_value.begin());
          store.push_back(Value(conversion, false , false, true));

          b = true;
        }
        if (!t_value.end().is_special()) {
          prep << (b ? SQL_OP_AND : getConjuction() + "( ") << column.str()
              << second_operator << "('%" << store.size() + 1 << "%'::timestamp AT TIME ZONE 'Europe/Prague' AT TIME ZONE 'UTC')" + value_post_;
          prep << " )";
          // we want to format date in the DateTime in the right way but without quotes
          // so we need to bypass the Database::Value(DateTime) constructor
          std::string conversion = SqlConvert<DateTime>::to(t_value.end());
          store.push_back(Value(conversion, false, false, true));
        } else if (b) {
          prep << " )";
        }
      }
    }
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
  Interval(const Column& _col, const Database::Null<DateInterval>& _value,
      const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateInterval>(_col, _value, _conj) {
  }
  Interval(const Column& _col, const std::string& _conj = SQL_OP_AND) :
    _BaseDTInterval<DateInterval>(_col, _conj) {
  }
  
  Interval() {
  }
  
  void setNULL() {
    value = Database::Null<DateInterval>();
    active = true;
  }

  void serialize(Database::SelectQuery& _sq, const Settings *_settings) {
    TRACE("[CALL] Interval<Date>::serialize()");
    Database::SelectQuery::prepared_values_string  &prep  = _sq.where_prepared_string();

    const DateInterval& t_value = value.getValue();
  
    if (!value.isNull() 
        && t_value.isSpecial() 
        && (t_value.getSpecial() != DAY && t_value.getSpecial() != INTERVAL)) {
      
      LOGGER.trace(boost::format("[IN] Interval<Date>::serialize(): value is special (special_flag='%1%')")
          % t_value.getSpecial());
      
      std::stringstream beg, end;
      std::string what = special2str(t_value.getSpecial());

      // TODO convert to iso_string
      std::string date = (boost::format("'%1%'::date AT TIME ZONE 'Europe/Prague'")
              % day_clock::local_day()).str();
  
      if (t_value.getSpecial() < PAST_HOUR) {
        beg << "(date_trunc('" << what << "', " << date << " + interval '"
            << t_value.getSpecialOffset() << " " << what <<"') AT TIME ZONE 'UTC')";
        end << "(" << beg.str() << " + interval '1 "<< what << "')";
        beg << value_post_;
        end << value_post_;
      } else {
        if (t_value.getSpecialOffset() < 0) {
          beg << "((" << date << " + interval '" << t_value.getSpecialOffset()
              << " " << what << "') AT TIME ZONE 'UTC')" + value_post_;
          end <<  "(" << date << " AT TIME ZONE 'UTC')" << value_post_;
        } else {
          end << "((" << date << " + interval '" << t_value.getSpecialOffset()
              << " " << what <<"') AT TIME ZONE 'UTC')" + value_post_;
          beg << "(" << date << " AT TIME ZONE 'UTC')" << value_post_;
        }
  
      }
      prep << getConjuction() << "( ";
      prep << column.str() << SQL_OP_GE << beg.str();
      prep << SQL_OP_AND << column.str() << SQL_OP_LT << end.str();
      prep << " )";
    }
    else {
      _BaseDTInterval<DateInterval>::serialize(_sq, _settings, true);
    }
  }
  
  friend class boost::serialization::access;
  template<class Archive> void serialize(Archive& _ar,
      const unsigned int _version) {
    _ar & BOOST_SERIALIZATION_BASE_TEMPLATE(
        "_BaseDTInterval_DateInterval",
        _BaseDTInterval<DateInterval>);
  }
};


template<class Tp> class ValueModifier;
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

  void setNULL() {
    value = Database::Null<Tp>();
    active = true;
  }
  
  void setColumn(const Column &col) {
    column = col;
  }

  virtual void setValue(const Null<Tp>& _value) {
    TRACE("[CALL] Value<Tp>::setValue()");
    active = true;
    value = _value;

    if (!modifiers_.empty()) {
      for (unsigned i = 0; i < modifiers_.size(); ++i) {
        modifiers_[i]->modify(*this);
      }
    }
  }

  virtual const Null<Tp>& getValue() const {
    return value;
  }

  virtual Value<Tp>* clone() const {
    return 0;
  }

  virtual void serialize(Database::SelectQuery& _sq, const Settings *_settings) {
    Database::SelectQuery::prepared_values_string  &prep  = _sq.where_prepared_string();
    Database::SelectQuery::prepared_values_storage &store = _sq.where_prepared_values();

    prep << getConjuction() << "( " << column.str();
    if (value.isNull()) {
      prep << SQL_OP_IS << value;
    } else {
      prep << op << "%" << store.size() + 1 << "%";
      store.push_back(value.getValue());
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

  void addModifier(ValueModifier<Tp> *_modifier) {
    modifiers_.push_back(_modifier);
  }

protected:
  Column column;
  std::string op;
  Database::Null<Tp> value;

  std::vector<ValueModifier<Tp> *> modifiers_;
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

  virtual void serialize(Database::SelectQuery& _sq, const Settings *_settings) {
    Database::SelectQuery::prepared_values_string  &prep  = _sq.where_prepared_string();
    Database::SelectQuery::prepared_values_storage &store = _sq.where_prepared_values();

    prep << getConjuction() << "( " << column.str();
    if (value.isNull()) {
      prep << SQL_OP_IS << value;
    } 
    else {
      std::string v = value.getValue();
      if (allowed_wildcard && (v.find('*') != std::string::npos || v.find('?') != std::string::npos)) {
        prep << " ILIKE TRANSLATE(%" << store.size() + 1 << "%,'*?','%%_')";
        store.push_back(v);
      }
      else {
        prep << op << " " << value_pre_ << "%" << store.size() + 1 << "%" << value_post_;
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
  Database::Null<std::string> value;
};

template<class Tp> class Null : public Simple {
public:
  Null(const Column& _col, const std::string& _op = SQL_OP_IS,
      const std::string& _conj = SQL_OP_AND) :
    Simple(_conj), column(_col) {
  }

  virtual ~Null() {
  }

  virtual void serialize(Database::SelectQuery& _sq, const Settings *_settings) {
    _sq.where_prepared_string() << getConjuction() << column.str() << SQL_OP_IS << "NULL";
  }
protected:
  Column column;
  Database::Null<Tp> value;
};


template<class Tp>
class Modifier {
public:
  Modifier(const Tp& _value) : value_cmp_(_value) {
  }

protected:
  Tp value_cmp_;
};

template<class Tp>
class ValueModifier : public Modifier<Tp> {
public:
  ValueModifier(const Tp& _value,
                const Tp& _to_value, 
                const std::string& _to_operator) : Modifier<Tp>(_value),
                                                   to_value_(_to_value),
                                                   to_operator_(_to_operator) {
  }

  void modify(Value<Tp>& _filter) {
    if (_filter.getValue().getValue() == this->value_cmp_) {
      _filter.setOperator(to_operator_);
      _filter.setValue(to_value_);
    }
  }

private:
  Tp to_value_;
  std::string to_operator_;
};

}
}

#endif /*SIMPLE_FILTERS_H_*/
