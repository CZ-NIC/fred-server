#ifndef SEQUENCE_H_
#define SEQUENCE_H_

#include <string>
#include "connection.h"
#include "data_types.h"
#include "query.h"

namespace DBase {

class Sequence {
private:
  Connection *conn_;
  std::string name_;
  
  DBase::ID execute_(Query& _query) {
    std::auto_ptr<Result> r(conn_->exec(_query));
    std::auto_ptr<ResultIterator> it(r->getIterator());
    return it->getNextValue();
  }
  
public:
  Sequence(Connection *_conn, const std::string _name) : conn_(_conn),
                                                         name_(_name) {
  }
  
  DBase::ID getCurrent() {
    Query q;
    q.buffer() << "SELECT currval('" << name_ << "')";
    return execute_(q);
  }
  
  DBase::ID getNext() {
    Query q;
    q.buffer() << "SELECT nextval('" << name_ << "')";
    return execute_(q);
  }
  
};

}

#endif /*SEQUENCE_H_*/
