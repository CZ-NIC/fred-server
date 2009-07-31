#ifndef _COMMON_NEW_H_
#define _COMMON_NEW_H_

namespace Register {

class CommonObjectNew {
public:
    virtual ~CommonObjectNew()
    { }
};

class CommonListNew {
public:
    virtual ~CommonListNew()
    { }
    virtual CommonObjectNew *get(unsigned int index) const = 0;
    virtual unsigned int getSize() const = 0;
    virtual void clear() = 0;
    virtual bool isEmpty() = 0;
    virtual void appendToList(CommonObjectNew *object) = 0;
    virtual const char *getTempTableName() const = 0;
};

} // namespace Register

#endif  // _COMMON_NEW_H_
