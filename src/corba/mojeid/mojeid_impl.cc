#include "mojeid_impl.h"

#include "log/logger.h"
#include "log/context.h"

#include <boost/lexical_cast.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/variate_generator.hpp>


boost::mt19937 gen;

const std::string create_ctx_name(const std::string &_name)
{
    boost::uniform_int<unsigned int> ruint(std::numeric_limits<unsigned int>::min(),
                                           std::numeric_limits<unsigned int>::max());
    boost::variate_generator<boost::mt19937&, boost::uniform_int<unsigned int> > gen_uint(gen, ruint);

    return str(boost::format("<%2%> %1%") % _name % gen_uint());
}


namespace Registry {


MojeIDImpl::MojeIDImpl(const std::string &_server_name)
    : server_name_(_server_name)
{
}


MojeIDImpl::~MojeIDImpl()
{
}


CORBA::ULongLong MojeIDImpl::contactCreate(const Contact &_contact)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("contact-create"));


    return 0;

}


CORBA::ULongLong MojeIDImpl::contactUpdatePrepare(const Contact &_contact,
                                                  const char* _trans_id)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("contact-update"));

    LOGGER(PACKAGE).debug("transaction id = " + std::string(_trans_id));
    return 0;
}


Contact* MojeIDImpl::contactInfo(const char* _handle)
{
    return 0;
}


void MojeIDImpl::commitPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("commit-prepared"));

    LOGGER(PACKAGE).debug("transaction id = " + std::string(_trans_id));
}


void MojeIDImpl::rollbackPreparedTransaction(const char* _trans_id)
{
    Logging::Context ctx_server(server_name_);
    Logging::Context ctx(create_ctx_name("rollback-prepared"));

    LOGGER(PACKAGE).debug("transaction id = " + std::string(_trans_id));
}


}

