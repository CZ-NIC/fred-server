/*
 * Copyright (C) 2014  CZ.NIC, z.s.p.o.
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
 *  @file
 *  optys mail upload client impl
 */

#include <string>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <iterator>
#include <iomanip>
#include <boost/shared_ptr.hpp>

#include <libssh/libssh.h>
#include <zip.h>


#include "upload_client.h"


    struct SshSessionDeleter
    {
        void operator()(ssh_session s)
        {
            if(s != NULL)
            {
                ssh_disconnect(s);
                ssh_free(s);
            }
        }
    };

    class SshSession
    {
        boost::shared_ptr<ssh_session_struct>  ssh_session_;

        void check_ssh_known_hosts(ssh_session session)
        {
            int state = ssh_is_server_known(session);

            boost::shared_ptr<unsigned char> hash_ptr;
            int hlen = 0;
            {
                unsigned char *hash = NULL;
                hlen = ssh_get_pubkey_hash(session, &hash);

                if(hash != NULL && hlen > 0)
                {
                     hash_ptr = boost::shared_ptr<unsigned char>(hash,free);
                }
                else
                {
                    throw std::runtime_error("ssh_get_pubkey_hash failed, unable to get buffer with the hash of the public key");
                }
            }
            std::ostringstream current_public_key_hash;
            current_public_key_hash <<  std::setw(2) << std::setfill('0') << std::hex << std::uppercase;
            std::copy(hash_ptr.get(), hash_ptr.get()+hlen, std::ostream_iterator<unsigned int>(current_public_key_hash, " "));

            switch (state)
            {
            case SSH_SERVER_KNOWN_OK:
                break; /* ok */
            case SSH_SERVER_KNOWN_CHANGED:
                {
                    std::string errmsg("Host key for server has changed, new public key hash is: ");
                    errmsg += current_public_key_hash.str();
                    throw std::runtime_error(errmsg);
                }
                break;
            case SSH_SERVER_FOUND_OTHER:
                {
                    throw std::runtime_error("The host key for this server was not found but an other type of key exists. "
                        "An attacker might change the default server key to confuse your client into thinking the key does not exist.");
                }
                break;
            case SSH_SERVER_FILE_NOT_FOUND:
                {
                    throw std::runtime_error("Could not find known host file.");
                }
                break;
            case SSH_SERVER_NOT_KNOWN:
                {
                    std::string errmsg("The server is unknown. Current public key hash is: ");
                    errmsg += current_public_key_hash.str();
                    throw std::runtime_error(errmsg);
                }
                break;
            case SSH_SERVER_ERROR:
                {
                    std::string errmsg("ssh_is_server_known failed: ");
                    errmsg += ssh_get_error(session);
                    throw std::runtime_error(errmsg);
                }
                break;
            default :
                {
                    throw std::runtime_error("ssh_is_server_known failed");
                }
                break;
            }
        }
    public:
        SshSession(const std::string& host, int port, const std::string& user, const std::string& password)
        : ssh_session_( ssh_new(), SshSessionDeleter())
        {
            if(ssh_session_.get() == 0) throw std::runtime_error("ssh_new failed");

            int log_verbosity = SSH_LOG_PROTOCOL;
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_LOG_VERBOSITY, &log_verbosity);
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_HOST, host.c_str());
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_PORT, &port);
            ssh_options_set(ssh_session_.get(), SSH_OPTIONS_USER, user.c_str());

            if (ssh_connect(ssh_session_.get()) != SSH_OK)
            {
                std::string errmsg("ssh_connect failed: ");
                errmsg += ssh_get_error(ssh_session_.get());
                throw std::runtime_error(errmsg);
            }

            check_ssh_known_hosts(ssh_session_.get());

           if (ssh_userauth_password(ssh_session_.get(), NULL, password.c_str()) != SSH_AUTH_SUCCESS)
           {
               std::string errmsg("ssh_userauth_password failed: ");
               errmsg += ssh_get_error(ssh_session_.get());
               throw std::runtime_error(errmsg);
           }
        }
    };

    OptysUploadClient::OptysUploadClient(const std::string& host, int port, const std::string& user, const std::string& password)
    {
        SshSession s(host, port, user, password);
    }

