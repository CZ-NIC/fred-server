/*
 * Copyright (C) 2009-2019  CZ.NIC, z. s. p. o.
 *
 * This file is part of FRED.
 *
 * FRED is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * FRED is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with FRED.  If not, see <https://www.gnu.org/licenses/>.
 */
/**
 *  @file model_field_attrs.h
 *  Model field attributes encapsulation 
 */

#ifndef MODEL_FIELD_ATTRS_HH_99BFFF2F50EA42A38D50490C61DFBE7F
#define MODEL_FIELD_ATTRS_HH_99BFFF2F50EA42A38D50490C61DFBE7F


namespace Model {
namespace Field {


class Attributes {
public:
  Attributes() : not_null_(false),
                 unique_(false),
                 pk_(false),
                 fk_(false),
                 default_(false) { }


  /**
   * Set NOT NULL attribut for model field
   * @return  self
   */
  Attributes& setNotNull() {
    not_null_ = true;
    return *this;
  }


  /**
   * Set UNIQUE attribut for model field
   * @return  self
   */
  Attributes& setUnique() {
    unique_ = true;
    return *this;
  }


  /**
   * Set model field to be Primary Key
   * @return  self
   */
  Attributes& setPrimaryKey() {
    pk_ = true;
    return setUnique().setNotNull();
  }


  /**
   * Set model field to be Foreign Key
   * @return  self
   */
  Attributes& setForeignKey() {
    fk_ = true;
    return *this;
  }


  /**
   * Set DEFAULT attribut for model field
   * @return  self
   */
  Attributes& setDefault() {
    default_ = true;
    return *this;
  }


  /**
   * Getter for appropriate attributes follows
   */
  bool isNotNull() const {
    return not_null_;
  }


  bool isUnique() const {
    return unique_;
  }


  bool isPrimaryKey() const {
    return pk_;
  }


  bool isForeignKey() const {
    return fk_;
  }


  bool isDefault() const {
    return default_;
  }


protected:
  bool not_null_; /**< NOT NULL */
  bool unique_;   /**< UNIQUE */
  bool pk_;       /**< Primary Key */
  bool fk_;       /**< Foreign Key */
  bool default_;  /**< DEFAULT */
};


}
}


#endif /*FIELD_MODEL_ATTRS_H_*/

