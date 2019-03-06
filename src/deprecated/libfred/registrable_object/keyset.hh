/*
 * Copyright (C) 2008-2019  CZ.NIC, z. s. p. o.
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
#ifndef KEYSET_HH_578ADA392786428BB3807D637FFC5AC1
#define KEYSET_HH_578ADA392786428BB3807D637FFC5AC1

#include <string>
#include <vector>
#include "src/deprecated/libfred/object.hh"
#include "src/deprecated/libfred/exceptions.hh"
#include "libfred/db_settings.hh"
#include "src/deprecated/model/model_filters.hh"
#include "src/deprecated/util/dbsql.hh"

/// forward declared parameter type
class DB;

namespace LibFred {
namespace Keyset {

/// member identification (i.e. for sorting)
enum MemberType {
    MT_HANDLE, ///< nsset identificator
    MT_CRDATE, ///< create date
    MT_ERDATE, ///< delete date
    MT_REGISTRAR_HANDLE, ///< registrar handle
};

class Keyset;

class DSRecord {
public:
    /// public d-tor
    virtual ~DSRecord() {}
    /// return dsrecord's id
    virtual const unsigned int &getId() const = 0;
    /// return keytag
    virtual const unsigned int &getKeyTag() const = 0;
    /// return used algorithm
    virtual const unsigned int &getAlg() const = 0;
    /// return digest type
    virtual const unsigned int &getDigestType() const = 0;
    /// return digest
    virtual const std::string &getDigest() const = 0;
    /// return record's time to live
    virtual const unsigned int &getMaxSigLife() const = 0;
    // comparison operators
    virtual bool operator==(const DSRecord& _other) const = 0;
    virtual bool operator!=(const DSRecord& _other) const = 0;
};

class DNSKey {
public:
    virtual ~DNSKey() {}
    /* return id of dnskey */
    virtual const Database::ID &getId() const = 0;
    /* following refer to RFC 4034 - DNSKEY RDATA format */
    /* return dnskey flags */
    virtual const unsigned int &getFlags() const = 0;
    /* return protocol */
    virtual const unsigned int &getProtocol() const = 0;
    /* return used algorithm */
    virtual const unsigned int &getAlg() const = 0;
    /* return public key */
    virtual const std::string  &getKey() const = 0;
    /* comparison operators */
    virtual bool operator==(const DNSKey& _other) const = 0;
    virtual bool operator!=(const DNSKey& _other) const = 0;
};

class Keyset: virtual public LibFred::Object {
public:
    /// public d-tor
    virtual ~Keyset() {}
    /// return keyset handle
    virtual const std::string &getHandle() const = 0;
    /// return count of admin contacts
    virtual unsigned int getAdminCount() const = 0;
    /// return admin's handle by index
    virtual std::string getAdminByIdx(unsigned int idx) const = 0;
    /// return handle of admin contact by index
    virtual const std::string& getAdminHandleByIdx(unsigned idx) const = 0;
    /// return id of admin contact by index
    virtual TID getAdminIdByIdx(unsigned idx) const = 0;
    /// return count of DS records
    virtual unsigned int getDSRecordCount() const = 0;
    /// return appropriate DS record by index
    virtual const DSRecord *getDSRecordByIdx(unsigned int idx) const = 0;
    /// return count of DNSKey records
    virtual unsigned int getDNSKeyCount() const = 0;
    /// return appropriate DNSKey record by index
    virtual const DNSKey *getDNSKeyByIdx(unsigned int idx) const = 0;
};

/// list of keysets
class List: virtual public ObjectList {
public:
    virtual ~List() {}
    /// get details of loaded keyset
    virtual Keyset *getKeyset(unsigned int index) const = 0;
    /// set filter for handle
    virtual void setHandleFilter(const std::string &handle) = 0;
    /// set filter for tech admin
    virtual void setAdminFilter(const std::string &handle) = 0;
    /// reload list with current filter
    virtual void reload() = 0;
    /// reload list with current filter
    virtual void reload(Database::Filters::Union &uf) = 0;
    /// clear filter data
    virtual void clearFilter() = 0;
    /// sort by column
    virtual void sort(MemberType _member, bool _asc) = 0;
};

/// main entry class
class Manager {
public:
    /// d-tor
    virtual ~Manager() {}
    /// return list of keysets
    virtual List *createList() = 0;

    /// type for check
    enum CheckAvailType {
        /// handle cannot be keyset
        CA_INVALID_HANDLE,
        /// handle is already registered
        CA_REGISTRED,
        /// handle is in protected period
        CA_PROTECTED,
        /// handle is free for registration
        CA_FREE
    };

    /// check proper format of handle
    virtual bool checkHandleFormat(const std::string &handle) const = 0;
    /// check possibilities for registration
    virtual CheckAvailType checkAvail(
            const std::string &handle,
            NameIdPair &conflict,
            bool lock = false) const = 0;

    static Manager *create(DBSharedPtr db, bool restrictedHandle);
};

} // namespace Keyset
} // namespace LibFred

#endif
