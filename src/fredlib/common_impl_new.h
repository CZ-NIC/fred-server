#ifndef _COMMON_IMPL_NEW_H_
#define _COMMON_IMPL_NEW_H_

#include "db_settings.h"
#include "common_new.h"
#include "model/model_filters.h"
#include "exceptions.h"

#include "log/logger.h"
#include "log/context.h"


#include <vector>

namespace Fred {

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

} // namespace Fred

#endif // _COMMON_IMPL_NEW_H_
