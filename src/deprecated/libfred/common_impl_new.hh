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
#ifndef COMMON_IMPL_NEW_HH_FA1D5E66C657470BA8D0BC349EC85E35
#define COMMON_IMPL_NEW_HH_FA1D5E66C657470BA8D0BC349EC85E35

#include "libfred/db_settings.hh"
#include "src/deprecated/libfred/common_new.hh"
#include "src/deprecated/model/model_filters.hh"
#include "src/deprecated/libfred/exceptions.hh"
#include "src/util/db/query/query.hh"

#include "util/log/logger.hh"
#include "util/log/context.hh"


#include <vector>

namespace LibFred {

class CommonObjectImplNew:
    virtual public CommonObjectNew {
public:
    CommonObjectImplNew();
    ~CommonObjectImplNew();
};

#define COMPARE_CLASS_IMPL_NEW(_object_type, _by_what)                      \
class Compare##_by_what {                                                   \
public:                                                                     \
  Compare##_by_what(bool _asc) : asc_(_asc) { }                             \
  bool operator()(CommonObjectNew *_left, CommonObjectNew *_right) const {  \
    _object_type *l_casted = dynamic_cast<_object_type *>(_left);           \
    _object_type *r_casted = dynamic_cast<_object_type *>(_right);          \
    if (l_casted == 0 || r_casted == 0) {                                   \
      /* this should never happen */                                        \
      throw std::bad_cast();                                                \
    }                                                                       \
                                                                            \
    return (asc_ ? l_casted->get##_by_what() < r_casted->get##_by_what()    \
                 : l_casted->get##_by_what() > r_casted->get##_by_what());  \
    }                                                                       \
private:                                                                    \
  bool asc_;                                                                \
};

class CommonListImplNew:
    virtual public CommonListNew {
protected:
    bool m_realSizeInitialized;
    unsigned long long m_realSize;
    bool m_loadLimitActive;
    std::vector<CommonObjectNew *>    m_data;
    unsigned int                    m_limit;
    void makeRealCount(Database::Filters::Union &filter);
public:
    CommonListImplNew();
    ~CommonListImplNew();
    CommonObjectNew * get(unsigned int index) const;
    unsigned int getSize() const;
    unsigned int size() const;
    void clear();
    bool isEmpty();
    void appendToList(CommonObjectNew * object);
    void setLimit(unsigned int limit);
    unsigned int getLimit() const;
    void fillTempTable(Database::InsertQuery &query);
    unsigned long long getRealCount(Database::Filters::Union &filter);
    void reload();
    bool isLimited() const;
    void setTimeout(unsigned _timeout);

};

} // namespace LibFred

#endif
