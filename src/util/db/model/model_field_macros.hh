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
#ifndef MODEL_FIELD_MACROS_HH_D5B542C568EA42AFBF29A6AC924DA165
#define MODEL_FIELD_MACROS_HH_D5B542C568EA42AFBF29A6AC924DA165

#include "src/util/db/model/model_field.hh"
// #include "model_field_related.h"


/**
 * Awful defining macros (but it saves some typing anyway)
 */

#define DEFINE_BASIC_FIELD(_class, _type, _name, _param1, _param2, _param3, _param4)     \
Model::Field::Basic<_class, _type>                                                       \
  _class::_name = Model::Field::Basic<_class, _type>(&_class::_param1,                   \
                                                     _class::_param2,                    \
                                                     _param3,                            \
                                                     Model::Field::Attributes()_param4);


#define DEFINE_PRIMARY_KEY(_class, _type, _name, _param1, _param2, _param3, _param4)          \
Model::Field::PrimaryKey<_class, _type>                                                       \
  _class::_name = Model::Field::PrimaryKey<_class, _type>(&_class::_param1,                   \
                                                          _class::_param2,                    \
                                                          _param3,                            \
                                                          Model::Field::Attributes()_param4);


#define DEFINE_FOREIGN_KEY(_class1, _class2, _type, _name, _param1, _param2, _param3, _param4, _param5) \
Model::Field::ForeignKey<_class1, _type, _class2>                                                       \
  _class1::_name = Model::Field::ForeignKey<_class1, _type, _class2>(&_class1::_param1,                 \
                                                                    _class1::_param2,                   \
                                                                    _param3, _class2::_param4,          \
                                                                    Model::Field::Attributes()_param5);


/*
#define DEFINE_ONE_TO_ONE(_class, _type1, _name, _field1name, _type2, _fk_name, _field2name)     \
Model::Field::Related::OneToOne<_class, _type2, _type1>                                          \
  _class::_name = Model::Field::Related::OneToOne<_class, _type2, _type1>(_class::_fk_name,      \
                                                                          &_class::_field2name,  \
                                                                          &_class::_field1name);


#define DEFINE_ONE_TO_MANY(_class, _name, _type1, _field1name, _class2, _field2name)               \
Model::Field::Related::OneToMany<_class, _type1, _class2>                                          \
  _class::_name = Model::Field::Related::OneToMany<_class, _type1, _class2>(_class2::_field2name,  \
                                                                            &_class::_field1name);



#define DEFINE_MANY_TO_MANY(_class, _type1, _field1name, _class2, _type2, _field2name, _name, _datafield, _table, _left, _right) \
Model::Field::Related::ManyToMany<_class, _type1, _class2, _type2>                                                               \
  _class::_name = Model::Field::Related::ManyToMany<_class, _type1, _class2, _type2>(_class::_field1name,                        \
                                                                                     _class2::_field2name,                       \
                                                                                     _table,                                     \
                                                                                     _left,                                      \
                                                                                     _right,                                     \
                                                                                     &_class::_datafield);
*/
#endif /*MODEL_FIELD_MACROS_H_*/

