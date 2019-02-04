#ifndef EPP_CORBA_CLIENT_HH_1C8AE669378C4EA19C051EDC1780309A
#define EPP_CORBA_CLIENT_HH_1C8AE669378C4EA19C051EDC1780309A

#include "util/types/id.hh"

class  EppCorbaClient
{
public:
    virtual ~EppCorbaClient() {
    }

    virtual void callDestroyAllRegistrarSessions(Database::ID reg_id) const = 0;

};

#endif
