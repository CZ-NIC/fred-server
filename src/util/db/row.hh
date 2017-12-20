/*  
 * Copyright (C) 2007  CZ.NIC, z.s.p.o.
 * 
 * This file is part of FRED.
 * 
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 2 of the License.
 * 
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 *  @file row.h
 *  Result row definition.
 */


#ifndef ROW_HPP_
#define ROW_HPP_

#include <vector>
#include <sstream>

namespace Database {


/**
 * \class Row_
 * \brief Base Row class
 *
 *  @param _RestultType   type of concrete implementation of database 
 *                        result object
 *  @param _FieldType     type of field element
 */
template <class _ResultType, class _FieldType>
class Row_ {
public:
  typedef _ResultType result_type;
  typedef _FieldType field_type;
  typedef typename _ResultType::size_type size_type;

  
  /**
   * Iterator interface
   */
  class Iterator;
  friend class Iterator;
  class Iterator : public std::iterator<std::bidirectional_iterator_tag, field_type> {
  protected:
    const result_type *data_ptr_;
    size_type         row_;
    size_type         col_;

  public:
    Iterator(const result_type *_data_ptr, 
             size_type _row, 
             size_type _col = 0) : data_ptr_(_data_ptr),
                                   row_(_row),
                                   col_(_col) {
    }


    Iterator(const Iterator &_other) {
      data_ptr_ = _other.data_ptr_;
      row_ = _other.row_;
      col_ = _other.col_;
    }


    field_type operator*() const {
      return field_type(data_ptr_->value_(row_, col_), data_ptr_->value_is_null_(row_, col_));
    }


    Iterator& operator+(int _n) {
      col_ += _n;
      return *this;
    }


    Iterator& operator++() {
      ++col_;
      return *this;
    }


    Iterator& operator+=(int _n) {
      col_ += _n;
      return *this;
    }


    Iterator& operator-(int _n) {
      col_ -= _n;
      return *this;
    }


    Iterator& operator--() {
      --col_;
      return *this;
    }


    bool operator==(const Iterator& _other) const {
      return (data_ptr_ == _other.data_ptr_ && row_ == _other.row_ && col_ == _other.col_);
    }


    bool operator!=(const Iterator& _other) const {
      return !(*this == _other);
    }
  };

  
  /**
   * Constructors and destructors
   */
  Row_(const result_type *_result_ptr, 
       size_type _row) : result_ptr_(_result_ptr),
                         row_(_row) {
  }


  virtual ~Row_() {
  }


  /**
   * @return size of row (number of columns)
   */
  size_type size() const {
    return result_ptr_->cols_();
  }


  /**
   * @param _n  column number
   * @return    field value at position _n
   */
  field_type operator[](int _n) const {
    return field_type(result_ptr_->value_(row_, _n), result_ptr_->value_is_null_(row_, _n));
  }


  /**
   * @param _cn  column name
   * @return     field value at position _cn
   */
  field_type operator[](const std::string& _cn) const {
    return field_type(result_ptr_->value_(row_, _cn), result_ptr_->value_is_null_(row_, _cn));
  }


  /**
   * @param _n  column number
   * @return    field value at position _n
   */
  field_type at(int _n) const {
    return field_type(result_ptr_->value_(row_, _n), result_ptr_->value_is_null_(row_, _n));
  }


  /**
   * @param _cn  column name
   * @return     field value at position _cn
   */
  field_type at(const std::string& _cn) const {
    return field_type(result_ptr_->value_(row_, _cn), result_ptr_->value_is_null_(row_, _cn));
  }


  Iterator begin() const {
    return Iterator(result_ptr_, row_);
  }


  Iterator end() const {
    return Iterator(result_ptr_, row_, size());
  }


private:
  const result_type *result_ptr_; /**< result pointer */
  size_type row_;                 /**< row number */
};



}

#endif /*ROW_HPP_*/
