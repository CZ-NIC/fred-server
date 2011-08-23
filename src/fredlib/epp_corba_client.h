#ifndef _EPP_CORBA_CLIENT_
#define _EPP_CORBA_CLIENT_

#include "util/types/id.h"

class  EppCorbaClient
{
public:
    virtual ~EppCorbaClient() {
    }

    virtual void callDestroyAllRegistrarSessions(Database::ID reg_id) const = 0;

};

#endif // _EPP_CORBA_CLIENT_
