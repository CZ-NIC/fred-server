/*
 *  Copyright (C) 2008  CZ.NIC, z.s.p.o.
 *
 *  This file is part of FRED.
 *
 *  FRED is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, version 2 of the License.
 *
 *  FRED is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with FRED.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <boost/date_time/posix_time/time_parsers.hpp>
#include <boost/regex.hpp>
#include <vector>
#include "src/deprecated/libfred/registrable_object/keyset.hh"

#include "src/deprecated/libfred/object_impl.hh"
#include "src/deprecated/libfred/sql.hh"
#include "src/deprecated/util/dbsql.hh"
#include "src/deprecated/util/util.hh"
#include "src/deprecated/model/model_filters.hh"
#include "util/log/logger.hh"

#define KEYSET_REGEX_RESTRICTED "[kK][eE][yY][iI][dD]:[a-zA-Z0-9_:.-]{1,57}"
#define KEYSET_REGEX "[a-zA-Z0-9_:.-]{1,63}"

namespace LibFred {
namespace Keyset {

static boost::regex format(KEYSET_REGEX);
static boost::regex formatRestricted(KEYSET_REGEX_RESTRICTED);

class DSRecordImpl: public virtual DSRecord {
    /// dsrecord`s id
    unsigned int    m_id;
    /// for details see RFC 4034 (http://rfc-ref.org/RFC-TEXTS/4034/)
    unsigned int    m_keyTag;
    /// algorithm identifier
    unsigned int    m_alg;
    /// digest type identifier
    unsigned int    m_digestType;
    /// string with digest
    std::string     m_digest;
    /// time to live
    unsigned int    m_maxSigLife;
public:
    DSRecordImpl(
            unsigned int id,
            unsigned int keyTag,
            unsigned int alg,
            unsigned int digestType,
            std::string  &digest,
            unsigned int maxSigLife):
        m_id(id), m_keyTag(keyTag), m_alg(alg),
        m_digestType(digestType), m_digest(digest),
        m_maxSigLife(maxSigLife)
    {
    }
    virtual const unsigned int &getId() const
    {
        return m_id;
    }
    virtual const unsigned int &getKeyTag() const
    {
        return m_keyTag;
    }
    virtual const unsigned int &getAlg() const
    {
        return m_alg;
    }
    virtual const unsigned int &getDigestType() const
    {
        return m_digestType;
    }
    virtual const std::string &getDigest() const
    {
        return m_digest;
    }
    virtual const unsigned int &getMaxSigLife() const
    {
        return m_maxSigLife;
    }
    bool operator==(const DSRecord& _other) const
    {
        /* this should be enough */
        return (m_id == _other.getId());
    }
    bool operator!=(const DSRecord& _other) const
    {
        return !(*this == _other);
    }
};

class DNSKeyImpl : public virtual DNSKey {
private:
    Database::ID id_;
    unsigned int flags_;
    unsigned int protocol_;
    unsigned int alg_;
    std::string  key_;

public:
    DNSKeyImpl(const Database::ID& _id,
               const unsigned int  _flags,
               const unsigned int  _protocol,
               const unsigned int  _alg,
               const std::string   _key) : id_(_id),
                                           flags_(_flags),
                                           protocol_(_protocol),
                                           alg_(_alg),
                                           key_(_key) {}
    virtual ~DNSKeyImpl() {}
    virtual const Database::ID &getId() const {
        return id_;
    }

    virtual const unsigned int &getFlags() const {
        return flags_;
    }

    virtual const unsigned int &getProtocol() const {
        return protocol_;
    }

    virtual const unsigned int &getAlg() const {
        return alg_;
    }

    virtual const std::string  &getKey() const {
        return key_;
    }

    virtual bool operator==(const DNSKey& _other) const {
        /* this should be enough */
        return (id_ == _other.getId());
    }

    virtual bool operator!=(const DNSKey& _other) const {
        return !(*this == _other);
    }
};

class KeysetImpl : public ObjectImpl, public virtual Keyset {
    struct AdminInfo {
      TID id;
      std::string handle;

      AdminInfo(TID _id, const std::string& _handle) :
        id(_id), handle(_handle) {
      }

      bool operator==(const AdminInfo& _ai) const {
        return (id == _ai.id && handle == _ai.handle);
      }
    };

    typedef std::vector<AdminInfo>    ContactListType;
    typedef std::vector<DSRecordImpl> DSRecordListType;
    typedef std::vector<DNSKeyImpl>   DNSKeyListType;


    std::string         m_handle;
    ContactListType     m_admins;
    DSRecordListType    m_DSRecords;
    DNSKeyListType      m_DNSKeys;

public:

    KeysetImpl(TID _id, const Database::ID &_history_id, const std::string& _handle, TID _registrar,
            const std::string& _registrarHandle, const ptime &_crDate, const ptime &_trDate,
            const ptime &_upDate, const ptime &_erDate, TID _createRegistrar,
            const std::string& _createRegistrarHandle, TID _updateRegistrar,
            const std::string& _updateRegistrarHandle,
            const std::string& _authPw, const std::string& _roid) :
        ObjectImpl(_id, _history_id, _crDate, _trDate, _upDate, _erDate, _registrar,
                _registrarHandle, _createRegistrar, _createRegistrarHandle,
                _updateRegistrar, _updateRegistrarHandle,
                _authPw, _roid), m_handle(_handle)
    {
    }

    const std::string &getHandle() const
    {
        return m_handle;
    }

    unsigned int getAdminCount() const
    {
        return m_admins.size();
    }

    std::string getAdminByIdx(unsigned int idx) const
    {
        if (idx >= m_admins.size())
            return "";
        else
            return m_admins[idx].handle;
    }

    const std::string& getAdminHandleByIdx(unsigned idx) const
    {
        if (idx >= getAdminCount())
           throw NOT_FOUND();
        return m_admins[idx].handle;
    }

    TID getAdminIdByIdx(unsigned idx) const
    {
        if (idx >= getAdminCount())
            throw NOT_FOUND();
        return m_admins[idx].id;
    }

    unsigned int getDSRecordCount() const
    {
        return m_DSRecords.size();
    }

    unsigned int getDNSKeyCount() const
    {
        return m_DNSKeys.size();
    }

    const DSRecord *getDSRecordByIdx(unsigned int idx) const
    {
        if (idx >= m_DSRecords.size())
            throw NOT_FOUND();
            // return NULL; <-- it is not checked anywhere!!
        else
            return &m_DSRecords[idx];
    }

    const DNSKey *getDNSKeyByIdx(unsigned int idx) const
    {
        if (idx >= m_DNSKeys.size())
            throw NOT_FOUND();
            // return NULL; <-- it is not checked anywhere!!
        else
            return &m_DNSKeys[idx];
    }

    void addAdminHandle(TID id, const std::string &admin)
    {
        m_admins.push_back(AdminInfo(id, admin));
    }

    DSRecordImpl *addDSRecord(
            unsigned int id,
            unsigned int keytag,
            unsigned int alg,
            unsigned int digestType,
            std::string digest,
            unsigned int maxSigLife
            )
    {
        m_DSRecords.push_back(
                DSRecordImpl(id, keytag, alg,
                    digestType, digest, maxSigLife));
        return &m_DSRecords.at(m_DSRecords.size() - 1);
    }

    DNSKeyImpl *addDNSKey(const DNSKeyImpl& _dnskey)
    {
        m_DNSKeys.push_back(_dnskey);
        return &m_DNSKeys.at(m_DNSKeys.size() - 1);
    }

    /// id lookup method
    bool hasId(TID id)
    {
        return id_ == id;
    }
};

COMPARE_CLASS_IMPL(KeysetImpl, Handle)
COMPARE_CLASS_IMPL(KeysetImpl, CreateDate)
COMPARE_CLASS_IMPL(KeysetImpl, DeleteDate)
COMPARE_CLASS_IMPL(KeysetImpl, RegistrarHandle)

class ListImpl: public virtual List, public ObjectListImpl {
    std::string     m_handle;
    std::string     m_admin;
public:
    ListImpl(DBSharedPtr _db): ObjectListImpl(_db)
    {
    }
    Keyset *getKeyset(unsigned int index) const
    {
        return dynamic_cast<KeysetImpl *>(get(index));
    }
    void setHandleFilter(const std::string &handle)
    {
        m_handle = handle;
    }
    void setAdminFilter(const std::string &handle)
    {
        m_admin = handle;
    }
    void makeQuery(bool, bool, std::stringstream &) const;
    void reload();
    virtual void reload(Database::Filters::Union &uf);
    void clearFilter();
    virtual const char *getTempTableName() const;
    virtual void sort(MemberType, bool);
};

void
ListImpl::makeQuery(
        bool count,
        bool limit,
        std::stringstream &sql) const
{
    std::stringstream from, where;
    sql.str("");
    if (!count)
        sql << "INSERT INTO " << getTempTableName() << " ";
    sql << "SELECT " << (count ? "COUNT(" : "") << "DISTINCT k.id"
        << (count ? ") " : " ");
    from << "FROM keyset k ";
    where << "WHERE 1=1 ";
    SQL_ID_FILTER(where, "k.id", idFilter);
    if (registrarFilter || !registrarHandleFilter.empty() ||
            updateRegistrarFilter || !updateRegistrarHandleFilter.empty() ||
            TIME_FILTER_SET(updateIntervalFilter) ||
            TIME_FILTER_SET(trDateIntervalFilter)) {
        from << ", object o ";
        where << "AND k.id=o.id ";
        SQL_ID_FILTER(where, "o.clid", registrarFilter);
        SQL_ID_FILTER(where, "o.upid", updateRegistrarFilter);
        SQL_DATE_FILTER(where, "o.upData", updateIntervalFilter);
        SQL_DATE_FILTER(where, "o.trData", trDateIntervalFilter);
        if (!registrarHandleFilter.empty()) {
            from << ", registrar reg ";
            where << "AND o.clid=reg.id ";
            SQL_HANDLE_WILDCHECK_FILTER(where, "reg.handle",
                    registrarHandleFilter, wcheck, false);
        }
        if (!updateRegistrarHandleFilter.empty()) {
            from << ", registrar ureg ";
            where << "AND o.upid=ureg.id ";
            SQL_HANDLE_WILDCHECK_FILTER(where, "ureg.handle",
                    updateRegistrarHandleFilter, wcheck, false);
        }
    }
    if (createRegistrarFilter || !createRegistrarHandleFilter.empty() ||
            TIME_FILTER_SET(crDateIntervalFilter) || !m_handle.empty()) {
        from << ", object_registry obr ";
        where << "AND obr.id=k.id AND obr.type=4 ";
        SQL_ID_FILTER(where, "obr.crid", createRegistrarFilter);
        SQL_DATE_FILTER(where, "obr.crdate", crDateIntervalFilter);
        SQL_HANDLE_WILDCHECK_FILTER(where, "obr.name", m_handle, wcheck, true);
        if (!createRegistrarHandleFilter.empty()) {
            from << ", registrar creg ";
            where << "AND obr.crid=creg.id ";
            SQL_HANDLE_WILDCHECK_FILTER(where, "creg.handle",
                    createRegistrarHandleFilter, wcheck, false);
        }
    }
    if (!m_admin.empty()) {
        from << ", keyset_contact_map kcm ";
        where << "AND k.id=kcm.keysetid ";
        // prepared for addition of admin ID filter
        if (!m_admin.empty()) {
            from << ", object_registry ncor ";
            where << "AND kcm.contactid=ncor.id AND ncor.type=1 ";
            SQL_HANDLE_WILDCHECK_FILTER(where, "ncor.name", m_admin, wcheck, true);
        }
    }
    if (!count)
        where << "ORDER BY k.id ASC ";
    if (limit)
        where << "LIMIT " << load_limit_ << " ";
    sql << from.rdbuf();
    sql << where.rdbuf();
}

void
ListImpl::reload()
{
    std::map<TID, std::string> registrars;
    std::ostringstream sql;
    sql << "SELECT id, handle FROM registrar";
    if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
    for (unsigned int i = 0; i < (unsigned int)db->GetSelectRows(); i++)
        registrars[STR_TO_ID(db->GetFieldValue(i, 0))] = db->GetFieldValue(i, 1);

    db->FreeSelect();
    sql.str("");
    clear();
    bool useTempTable = nonHandleFilterSet || m_handle.empty();
    if (useTempTable)
        fillTempTable(true);
    sql << "SELECT obr.id, obr.name, o.clid, "
        << "obr.crdate, o.trdate, o.update, "
        << "obr.crid, o.upid, o.authinfopw, obr.roid FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "keyset k, object_registry obr, object o "
        << "WHERE tmp.id=k.id AND k.id=o.id AND obr.id=o.id ";
    if (!useTempTable)
        sql << "AND tmp.name=UPPER('" << db->Escape2(m_handle) << "') "
            << "AND tmp.erdate ISNULL AND tmp.type=4 ";
    sql << "ORDER BY tmp.id ";
    if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
    for (unsigned int i = 0; i < (unsigned int)db->GetSelectRows(); i++) {
        data_.push_back(
                new KeysetImpl(
                    STR_TO_ID(db->GetFieldValue(i, 0)),     // keyset id
                    Database::ID(0),                        // dummy history_id
                    db->GetFieldValue(i, 1),                // keyset handle
                    STR_TO_ID(db->GetFieldValue(i, 2)),     // registrar id
                    registrars[STR_TO_ID(db->GetFieldValue(i, 2))], // reg.handle
                    MAKE_TIME(i, 3),                        // reg. crdate
                    MAKE_TIME(i, 4),                        // reg. trdate
                    MAKE_TIME(i, 5),                        // reg. update
                    ptime(not_a_date_time),
                    STR_TO_ID(db->GetFieldValue(i, 6)),     // crid
                    registrars[STR_TO_ID(db->GetFieldValue(i, 6))], // crid handle
                    STR_TO_ID(db->GetFieldValue(i, 7)),     // upid
                    registrars[STR_TO_ID(db->GetFieldValue(i, 7))], // upid handle
                    db->GetFieldValue(i, 8),                // authinfo pass
                    db->GetFieldValue(i, 9)                // roid
                    )
                );
    }
    db->FreeSelect();
    // no need to proceed when nothing was loaded
    if (!getCount())
        return;
    resetIDSequence();
    sql.str("");
    sql << "SELECT k.keysetid, cor.name FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "keyset_contact_map k, object_registry cor "
        << "WHERE tmp.id=k.keysetid AND k.contactid=cor.id ";
    if (!useTempTable)
        sql << "AND tmp.name=UPPER('" << db->Escape2(m_handle) << "') "
            << "AND tmp.erdate ISNULL AND tmp.type=4 ";
    sql << "ORDER BY tmp.id, cor.id ";
    if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
    for (unsigned int i = 0; i < (unsigned int)db->GetSelectRows(); i++) {
        KeysetImpl *ks =
            dynamic_cast<KeysetImpl *>(findIDSequence(STR_TO_ID(db->GetFieldValue(i, 0))));
        if (!ks)
            throw SQL_ERROR();
        ks->addAdminHandle(0 /* dummy id - for history */, db->GetFieldValue(i, 1));
    }
    db->FreeSelect();
    resetIDSequence();
    sql.str("");
    sql << "SELECT d.keysetid, d.id, d.keytag, d.alg, d.digesttype, d.digest, d.maxsiglife FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "dsrecord d "
        << "WHERE tmp.id=d.keysetid ";
    if (!useTempTable)
        sql << "AND tmp.name=UPPER('" << db->Escape2(m_handle) << "') "
            << "AND tmp.erdate ISNULL AND tmp.type=4 ";
    sql << "ORDER BY tmp.id, d.id ";
    if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
    for (unsigned int i = 0; i < (unsigned int)db->GetSelectRows(); i++) {
        KeysetImpl *ks =
            dynamic_cast<KeysetImpl *>(findIDSequence(STR_TO_ID(
                            db->GetFieldValue(i, 0))));
        if (!ks)
            throw SQL_ERROR();
        // XXX what is returned if maxsiglife is NULL? and can i atoi it?
        // so, null is represented by 0
        ks->addDSRecord(
                STR_TO_ID(db->GetFieldValue(i, 1)),
                atoi(db->GetFieldValue(i, 2)),
                atoi(db->GetFieldValue(i, 3)),
                atoi(db->GetFieldValue(i, 4)),
                db->GetFieldValue(i, 5),
                atoi(db->GetFieldValue(i, 6)));
    }
    db->FreeSelect();

    /* load dnskey records */
    resetIDSequence();
    sql.str("");
    sql << "SELECT d.keysetid, d.id, d.flags, d.protocol, d.alg, d.key FROM "
        << (useTempTable ? getTempTableName() : "object_registry ") << " tmp, "
        << "dnskey d "
        << "WHERE tmp.id=d.keysetid ";
    if (!useTempTable)
        sql << "AND tmp.name=UPPER('" << db->Escape2(m_handle) << "') "
            << "AND tmp.erdate ISNULL AND tmp.type=4 ";
    sql << "ORDER BY tmp.id, d.id ";
    if (!db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
    for (unsigned int i = 0; i < (unsigned int)db->GetSelectRows(); i++) {
        KeysetImpl *ks =
            dynamic_cast<KeysetImpl *>(findIDSequence(STR_TO_ID(
                            db->GetFieldValue(i, 0))));
        if (!ks)
            throw SQL_ERROR();
        ks->addDNSKey(DNSKeyImpl(STR_TO_ID(db->GetFieldValue(i, 1)),
                                 atoi(db->GetFieldValue(i, 2)),
                                 atoi(db->GetFieldValue(i, 3)),
                                 atoi(db->GetFieldValue(i, 4)),
                                 db->GetFieldValue(i, 5)));
    }
    db->FreeSelect();

    ObjectListImpl::reload(useTempTable ? NULL : m_handle.c_str(), 4);
}

void
ListImpl::reload(Database::Filters::Union &uf)
{
    TRACE("[CALL] Keyset::ListImpl::reload(Database::Filters::Union &)");
    clear();
    uf.clearQueries();

    // TEMP: should be cached to work quicker
    std::map<Database::ID, std::string> registrars_table;

    bool at_least_one = false;
    Database::SelectQuery id_query;
    std::unique_ptr<Database::Filters::Iterator> fit(uf.createIterator());
    Database::SelectQuery *tmp;

    for (fit->first(); !fit->isDone(); fit->next()) {
        Database::Filters::KeySet *kf =
            dynamic_cast<Database::Filters::KeySetHistoryImpl *>(fit->get());
        if (!kf)
            continue;
        tmp = new Database::SelectQuery();
        tmp->addSelect(new Database::Column(
                    "historyid",
                    kf->joinKeySetTable(),
                    "DISTINCT"));
        uf.addQuery(tmp);
        at_least_one = true;
    }
    if (!at_least_one) {
        LOGGER.error("wrong filter passed for reload!");
        return;
    }

    id_query.limit(load_limit_);
    uf.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery(
            getTempTableName(), id_query);

    LOGGER.debug(boost::format("temporary table '%1%' generated sql = %2%")
            % getTempTableName() % tmp_table_query.str());

    Database::SelectQuery object_info_query;

    /* fill query */
    object_info_query.select()
        << "t_1.id, tmp.id, t_1.name, t_2.clid, t_1.crdate, "
        << "t_2.trdate, t_2.update, t_1.erdate, t_1.crid, t_2.upid, "
        << "t_2.authinfopw, t_1.roid";

    object_info_query.from()
        << getTempTableName()
        << " tmp JOIN keyset_history t_3 ON (tmp.id=t_3.historyid) "
        << "JOIN object_history t_2 ON (t_3.historyid=t_2.historyid) "
        << "JOIN object_registry t_1 ON (t_1.id=t_2.id)";
    object_info_query.order_by()
        << "tmp.id";

    try {
        Database::Connection conn = Database::Manager::acquire();

        Database::Query create_tmp_table("SELECT create_tmp_table('"
                + std::string(getTempTableName()) + "')");
        conn.exec(create_tmp_table);
        conn.exec(tmp_table_query);

        // TEMP: should by cached somewhere
        Database::Query registrars_query("SELECT id, handle FROM registrar");
        Database::Result r_registrars = conn.exec(registrars_query);
        Database::Result::Iterator it = r_registrars.begin();
        for (; it != r_registrars.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            Database::ID id         = *col;
            std::string handle      = *(++col);
            registrars_table[id]    = handle;
        }

        Database::Result r_info = conn.exec(object_info_query);
        for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            Database::ID kid                = *col;
            Database::ID history_id         = *(++col);
            std::string handle              = *(++col);
            Database::ID registrar_id       = *(++col);
            std::string registrar_handle    = registrars_table[registrar_id];
            Database::DateTime cr_date      = *(++col);
            Database::DateTime tr_date      = *(++col);
            Database::DateTime up_date      = *(++col);
            Database::DateTime er_date      = *(++col);
            Database::ID crid               = *(++col);
            std::string crid_handle         = registrars_table[crid];
            Database::ID upid               = *(++col);
            std::string upid_handle         = registrars_table[upid];
            std::string authinfo            = *(++col);
            std::string roid                = *(++col);

            data_.push_back(
                    new KeysetImpl(
                        kid,
                        history_id,
                        handle,
                        registrar_id,
                        registrar_handle,
                        cr_date,
                        tr_date,
                        up_date,
                        er_date,
                        crid,
                        crid_handle,
                        upid,
                        upid_handle,
                        authinfo,
                        roid)
                    );
        }

        if (data_.empty()) {
            return;
        }

        resetHistoryIDSequence();
        Database::SelectQuery contacts_query;
        contacts_query.select()
            << "tmp.id, t_1.keysetid, t_2.id, t_2.name";
        contacts_query.from()
            << getTempTableName() << " tmp"
            << " JOIN keyset_contact_map_history t_1 ON (tmp.id = t_1.historyid) "
            << "JOIN object_registry t_2 ON (t_1.contactid = t_2.id)";
        contacts_query.order_by()
            << "t_1.keysetid, tmp.id ";


        Database::Result r_contacts = conn.exec(contacts_query);
        for (Database::Result::Iterator it = r_contacts.begin(); it != r_contacts.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            Database::ID keyset_historyid   = *col;
                                               (++col);//Database::ID keyset_id
            Database::ID contact_id         = *(++col);
            std::string  contact_handle     = *(++col);

            KeysetImpl *keyset_ptr = dynamic_cast<KeysetImpl *>(findHistoryIDSequence(keyset_historyid));
            if (keyset_ptr)
                keyset_ptr->addAdminHandle(contact_id, contact_handle);
        }

        resetHistoryIDSequence();
        Database::SelectQuery dsrecord_query;
        dsrecord_query.select()
            << "tmp.id, t_1.keysetid, t_1.id, t_1.keytag, t_1.alg, "
            << "t_1.digesttype, t_1.digest, t_1.maxsiglife ";
        dsrecord_query.from()
            << getTempTableName() << " tmp "
            << "JOIN dsrecord_history t_1 ON (tmp.id = t_1.historyid)";
        dsrecord_query.order_by()
            << "t_1.keysetid, tmp.id, t_1.id";

        Database::Result r_dsrecords = conn.exec(dsrecord_query);
        for (Database::Result::Iterator it = r_dsrecords.begin(); it != r_dsrecords.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            Database::ID keyset_historyid  = *col;
                                           (++col);//Database::ID keyset_id
            Database::ID dsrecord_id    = *(++col);
            unsigned int keytag         = *(++col);
            unsigned int alg            = *(++col);
            unsigned int digesttype     = *(++col);
            std::string digest          = *(++col);
            unsigned int maxsiglife     = *(++col);

            KeysetImpl *keyset_ptr = dynamic_cast<KeysetImpl *>(findHistoryIDSequence(keyset_historyid));
            if (keyset_ptr) {
                // LOGGER.debug(boost::format("dsrec: id: %1% digest: %2%") %
                        // dsrecord_id % digest);
                keyset_ptr->addDSRecord(
                        dsrecord_id,
                        keytag,
                        alg,
                        digesttype,
                        digest,
                        maxsiglife);
            }
        }

        /* load dnskey records */
        resetHistoryIDSequence();
        Database::SelectQuery dnskey_query;
        dnskey_query.select()
            << "tmp.id, t_1.keysetid, t_1.id, t_1.flags, t_1.protocol, t_1.alg, t_1.key ";
        dnskey_query.from()
            << getTempTableName() << " tmp "
            << "JOIN dnskey_history t_1 ON (tmp.id = t_1.historyid)";
        dnskey_query.order_by()
            << "t_1.keysetid, tmp.id, t_1.id";

        Database::Result r_dnskeys = conn.exec(dnskey_query);
        for (Database::Result::Iterator it = r_dnskeys.begin(); it != r_dnskeys.end(); ++it) {
            Database::Row::Iterator col = (*it).begin();

            Database::ID keyset_historyid  = *col;
                                              (++col);//Database::ID keyset_id
            Database::ID dnskey_id         = *(++col);
            unsigned int flags             = *(++col);
            unsigned int protocol          = *(++col);
            unsigned int alg               = *(++col);
            std::string  key               = *(++col);

            KeysetImpl *keyset_ptr = dynamic_cast<KeysetImpl *>(findHistoryIDSequence(keyset_historyid));
            if (keyset_ptr) {
                // LOGGER.debug(boost::format("dsrec: id: %1% digest: %2%") %
                        // dsrecord_id % digest);
                keyset_ptr->addDNSKey(DNSKeyImpl(dnskey_id,
                                                 flags,
                                                 protocol,
                                                 alg,
                                                 key));
            }
        }


        bool history = false;
        if (uf.settings()) {
          history = uf.settings()->get("filter.history") == "on";
        }

        /// load object state
        ObjectListImpl::reload(history);
        // check if row number result load limit is active and flag is set
        CommonListImpl::reload();
    }
    catch (Database::Exception &ex) {
        std::string message = ex.what();
        if (message.find(Database::Connection::getTimeoutString())
                != std::string::npos) {
            LOGGER.info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER.error(boost::format("%1%") % ex.what());
            clear();
        }
    }
    catch (std::exception &ex) {
        LOGGER.error(boost::format("%1%") % ex.what());
    }
}

void
ListImpl::clearFilter()
{
    ObjectListImpl::clearFilter();
    m_handle = "";
    m_admin = "";
}

const char *
ListImpl::getTempTableName() const
{
    return "tmp_keyset_filter_result";
}

void
ListImpl::sort(MemberType _member, bool _asc)
{
    switch (_member) {
        case MT_HANDLE:
            stable_sort(data_.begin(), data_.end(), CompareHandle(_asc));
            break;
        case MT_CRDATE:
            stable_sort(data_.begin(), data_.end(), CompareCreateDate(_asc));
            break;
        case MT_ERDATE:
            stable_sort(data_.begin(), data_.end(), CompareDeleteDate(_asc));
            break;
        case MT_REGISTRAR_HANDLE:
            stable_sort(data_.begin(), data_.end(), CompareRegistrarHandle(_asc));
            break;
    }
}

class ManagerImpl: public virtual Manager {
    DBSharedPtr m_db; ///< connection to db
    bool    m_restrictedHandle;
    bool checkHandleFormat(const std::string &handle) const;
    bool checkHandleRegistration(const std::string &, NameIdPair &,
            bool) const;
    bool checkProtection(const std::string &, unsigned int,
            const std::string &) const;
public:
    ManagerImpl(DBSharedPtr db, bool restrictedHandle):
        m_db(db), m_restrictedHandle(restrictedHandle)
    {
    }
    virtual List *createList()
    {
        return new ListImpl(m_db);
    }
    virtual CheckAvailType checkAvail(
            const std::string &handle,
            NameIdPair &conflict,
            bool lock) const
    {
        conflict.id = 0;
        conflict.name = "";
        if (!checkHandleFormat(handle))
            return CA_INVALID_HANDLE;
        if (checkHandleRegistration(handle, conflict, lock))
            return CA_REGISTRED;
        if (checkProtection(handle, 4, "(SELECT val FROM enum_parameters WHERE id = 12) || ' month'"))
            return CA_PROTECTED;
        return CA_FREE;
    }
};

bool
ManagerImpl::checkHandleFormat(const std::string &handle) const
{
    try {
        return boost::regex_match(
            handle, m_restrictedHandle ? formatRestricted : format);
    } catch (...) {
        // TODO log error
        return false;
    }
}

/// check if object is in database
bool
ManagerImpl::checkHandleRegistration(
        const std::string &handle,
        NameIdPair &conflict,
        bool lock) const
{
    std::ostringstream sql;
    sql << "SELECT id, name FROM object_registry "
        << "WHERE type=4 AND erdate ISNULL AND "
        << "UPPER(name)=UPPER('"
        << handle << "')";
    if (lock)
        sql << " FOR UPDATE ";
    if (!m_db->ExecSelect(sql.str().c_str()))
        throw SQL_ERROR();
    bool result = m_db->GetSelectRows() >= 1;
    conflict.id = result ? STR_TO_ID(m_db->GetFieldValue(0, 0)) : 0;
    conflict.name = result ? m_db->GetFieldValue(0, 1) : "";
    m_db->FreeSelect();
    return result;
}

/// check if object handle is in protection period (true means protected)
bool
ManagerImpl::checkProtection(
        const std::string &name,
        unsigned int type,
        const std::string &monthPeriodSQL) const
{
    std::stringstream sql;
    sql << "SELECT COALESCE(MAX(erdate) + ("
        << monthPeriodSQL << ")::interval "
        << "> CURRENT_TIMESTAMP, false) FROM object_registry "
        << "WHERE NOT(erdate ISNULL) AND type="
        << type
        << " AND UPPER(name)=UPPER('"
        << name << "')";
    if (!m_db->ExecSelect(sql.str().c_str())) {
        m_db->FreeSelect();
        throw SQL_ERROR();
    }
    bool ret = (m_db->GetFieldValue(0, 0)[0] == 't');
    m_db->FreeSelect();
    return ret;
}

Manager *Manager::create(DBSharedPtr db, bool restrictedHandle)
{
    return new ManagerImpl(db, restrictedHandle);
}

} // namespace Keyset
} // namespace LibFred
