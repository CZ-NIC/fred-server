#ifndef EXCEPTION_H_2AF3B81D96F803132F940CFC8A5B5EEE//date "+%s"|md5sum|tr "[a-f]" "[A-F]"
#define EXCEPTION_H_2AF3B81D96F803132F940CFC8A5B5EEE

#include "src/fredlib/opexception.h"
namespace Fred {
namespace Poll {

struct Exception:OperationException
{
    virtual const char* what()const throw() = 0;
};

}//namespace Fred::Poll
}//namespace Fred

#endif//EXCEPTION_H_2AF3B81D96F803132F940CFC8A5B5EEE
