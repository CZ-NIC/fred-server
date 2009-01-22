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
 *  @file mem_PTR.h
 *  Class member pointer
 */

#ifndef MEM_PTR_H_
#define MEM_PTR_H_


template<class _class, class _type>
class MemPtr {
public:
  typedef _class          class_name;
  typedef _type           value_type;
  typedef value_type class_name::* pointer_type;


  /**
   * Constructor
   * @param _ptr  member pointer address
   */
  MemPtr(pointer_type _ptr) : ptr_(_ptr) {
  }


  /**
   * @param _obj  concrete instance of class_name
   * @return      value of stored pointer in given instance
   */
  value_type& operator()(class_name &_obj) const {
    return _obj.*ptr_;
  }


  /**
   * @param _obj  concrete instance of class_name
   * @return      value reference of stored pointer in given instance
   */
  const value_type& operator()(const class_name &_obj) const {
    return _obj.*ptr_;
  }


private:
  pointer_type ptr_;
};


#endif /*MEM_PTR_H_*/

