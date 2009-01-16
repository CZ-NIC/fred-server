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
 *  @file singleton.h
 *  \class Singleton
 *  \brief Template for simple implementing singleton pattern
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_


template<class _Tp>
class Singleton {
public:
  /**
   * Getter
   * @return  stored instance pointer
   */
	static _Tp* instance_ptr() {
		if (instance_ == 0) {
			instance_ = new _Tp();
		}
		return instance_;
	}


  /**
   * Getter
   * @return  stored instance reference
   */
	static _Tp& instance_ref() {
		if (instance_ == 0) {
			instance_ = new _Tp();
		}
		return *instance_;
	}


  /**
   * Explicit destruction of inner instance
   */
	void destroy() {
		if (instance_) {
			delete instance_;
			instance_ = 0;
		}
	}

private:
  /**
   * Constructors, destructors, assigment operator
   * keeping as private
   */
	Singleton() {
	}


	Singleton(const Singleton<_Tp> &_s) {
	}


	~Singleton() {
	}


	Singleton<_Tp>& operator=(Singleton<_Tp> const &_s) {
		return *this;
	}

	static _Tp* instance_;
};


template<class _Tp> _Tp* Singleton<_Tp>::instance_ = 0;


#endif /*SINGLETON_H_*/

