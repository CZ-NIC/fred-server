#include "public_request.h"
#include "public_request_impl.h"
#include "public_request_authinfo_impl.h"
#include "public_request_block_impl.h"
#include "public_request_verification_impl.h"


namespace Fred {
namespace PublicRequest {



class ListImpl : public Fred::CommonListImpl,
                 virtual public List {
private:
  Manager *manager_;

public:
  ListImpl(Manager *_manager) : CommonListImpl(), manager_(_manager) {
  }

  virtual ~ListImpl() {
  }

  virtual const char* getTempTableName() const {
    return "tmp_public_request_filter_result";
  }

  virtual PublicRequest* get(unsigned _idx) const {
    try {
      PublicRequest *request = dynamic_cast<PublicRequest* >(data_.at(_idx));
      if (request)
        return request;
      else
        throw std::runtime_error("Wrong object type of public reuqest data from DB.");
    } catch (std::exception) {
        throw;
    } catch (...) {
      throw std::runtime_error("Unknown exception when retrieving public reuqest data from DB.");
    }
  }

  virtual void reload(Database::Filters::Union& _filter) {
    TRACE("[CALL] Fred::Request::ListImpl::reload()");
    clear();
    _filter.clearQueries();

    bool at_least_one = false;
    Database::SelectQuery id_query;
    Database::Filters::Compound::iterator fit = _filter.begin();
    for (; fit != _filter.end(); ++fit) {
      Database::Filters::PublicRequest *rf = dynamic_cast<Database::Filters::PublicRequest* >(*fit);
      if (!rf)
        continue;

      Database::SelectQuery *tmp = new Database::SelectQuery();
      tmp->addSelect(new Database::Column("id", rf->joinRequestTable(), "DISTINCT"));
      _filter.addQuery(tmp);
      at_least_one = true;
    }
    if (!at_least_one) {
      LOGGER(PACKAGE).error("wrong filter passed for reload!");
      return;
    }

    id_query.order_by() << "id DESC";
    id_query.limit(load_limit_);
    _filter.serialize(id_query);

    Database::InsertQuery tmp_table_query = Database::InsertQuery(getTempTableName(),
                                                            id_query);
    LOGGER(PACKAGE).debug(boost::format("temporary table '%1%' generated sql = %2%")
        % getTempTableName() % tmp_table_query.str());

    Database::SelectQuery object_info_query;
    object_info_query.select() << "t_1.request_type, t_1.id, t_1.create_request_id, t_1.resolve_request_id, "
                               << "t_1.create_time, t_1.status, t_1.resolve_time, "
                               << "t_1.reason, t_1.email_to_answer, t_1.answer_email_id, "
                               << "t_4.id, t_4.handle, t_4.name, t_4.url, "
                               << "t_5.identification, t_5.password";
    object_info_query.from() << getTempTableName() << " tmp "
                             << "JOIN public_request t_1 ON (t_1.id = tmp.id) "
                             << "LEFT JOIN registrar t_4 ON (t_1.registrar_id = t_4.id) "
                             << "LEFT JOIN public_request_auth t_5 ON (t_5.id = t_1.id) ";
    object_info_query.order_by() << "t_1.id DESC";

    Database::Connection conn = Database::Manager::acquire();
    try {
      fillTempTable(tmp_table_query);

      Database::Result r_info = conn.exec(object_info_query);
      for (Database::Result::Iterator it = r_info.begin(); it != r_info.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Fred::PublicRequest::Type type = (Fred::PublicRequest::Type)(int)*col;
        PublicRequest* request = manager_->createRequest(type);
        request->init(++col);
        data_.push_back(request);
      }

      if (data_.empty())
        return;

      /*
       * load objects details for requests
       */
      resetIDSequence();

      Database::SelectQuery objects_query;
      objects_query.select() << "tmp.id, t_1.object_id, t_2.name, t_2.type";
      objects_query.from() << getTempTableName() << " tmp "
                           << "JOIN public_request_objects_map t_1 ON (tmp.id = t_1.request_id) "
                           << "JOIN object_registry t_2 ON (t_1.object_id = t_2.id)";
      objects_query.order_by() << "tmp.id";

      Database::Result r_objects = conn.exec(objects_query);
      for (Database::Result::Iterator it = r_objects.begin(); it != r_objects.end(); ++it) {
        Database::Row::Iterator col = (*it).begin();

        Database::ID request_id    = *col;
        Database::ID object_id     = *(++col);
        std::string  object_handle = *(++col);
        ObjectType   object_type   = (ObjectType)(int)*(++col);

        PublicRequestImpl *request_ptr = dynamic_cast<PublicRequestImpl *>(findIDSequence(request_id));
        if (request_ptr)
          request_ptr->addObject(OID(object_id, object_handle, object_type));
      }
      /* checks if row number result load limit is active and set flag */
      CommonListImpl::reload();
    }
    catch (Database::Exception& ex) {
        std::string message = ex.what();
        if (message.find(Database::Connection::getTimeoutString())
                != std::string::npos) {
            LOGGER(PACKAGE).info("Statement timeout in request list.");
            clear();
            throw;
        } else {
            LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
            clear();
        }
    }
    catch (std::exception& ex) {
      LOGGER(PACKAGE).error(boost::format("%1%") % ex.what());
      clear();
    }


  }

  virtual void sort(MemberType _member, bool _asc) {
    switch(_member) {
      case MT_CRDATE:
        stable_sort(data_.begin(), data_.end(), CompareCreateTime(_asc));
    break;
      case MT_RDATE:
        stable_sort(data_.begin(), data_.end(), CompareResolveTime(_asc));
    break;
      case MT_TYPE:
        stable_sort(data_.begin(), data_.end(), CompareType(_asc));
    break;
      case MT_STATUS:
        stable_sort(data_.begin(), data_.end(), CompareStatus(_asc));
    break;
    }
  }

  virtual void reload() {
  }

  virtual void makeQuery(bool, bool, std::stringstream&) const {
  }
};


class ManagerImpl : virtual public Manager {
private:
  Domain::Manager   *domain_manager_;
  Contact::Manager  *contact_manager_;
  NSSet::Manager    *nsset_manager_;
  KeySet::Manager   *keyset_manager_;
  Mailer::Manager   *mailer_manager_;
  Document::Manager *doc_manager_;
  Messages::ManagerPtr messages_manager;

  std::string identification_mail_auth_hostname_;
  bool demo_mode_;

public:
  ManagerImpl(Domain::Manager   *_domain_manager,
              Contact::Manager  *_contact_manager,
              NSSet::Manager    *_nsset_manager,
              KeySet::Manager   *_keyset_manager,
              Mailer::Manager   *_mailer_manager,
              Document::Manager *_doc_manager,
              Messages::ManagerPtr _messages_manager)
      : domain_manager_(_domain_manager),
        contact_manager_(_contact_manager),
        nsset_manager_(_nsset_manager),
        keyset_manager_(_keyset_manager),
        mailer_manager_(_mailer_manager),
        doc_manager_(_doc_manager),
        messages_manager(_messages_manager),
        identification_mail_auth_hostname_(std::string()),
        demo_mode_(false)
  {
  }

  virtual ~ManagerImpl() {
  }

  virtual Mailer::Manager* getMailerManager() const {
    return mailer_manager_;
  }

  virtual Document::Manager* getDocumentManager() const {
    return doc_manager_;
  }

  Messages::ManagerPtr getMessagesManager() const
  {
      return messages_manager;
  }

  virtual List* createList() const {
    TRACE("[CALL] Fred::Request::Manager::createList()");
    /*
     * can't use this way; connection will not be properly closed when
     * List is destroyed. Also can't closing connection in CommonLisrImpl
     * because it can be passed as existing connection and then used after
     * List destruction.
     */
    // return new ListImpl(db_manager_->getConnection(), (Manager *)this);
    return new ListImpl((Manager *)this);
  }

  virtual void getPdf(Database::ID _id,
                      const std::string& _lang,
                      std::ostream& _output) const
                                                    {
    TRACE(boost::format("[CALL] Fred::Request::Manager::getPdf(%1%, '%2%')") %
          _id % _lang);
    std::auto_ptr<List> l(loadRequest(_id));
    PublicRequest* p = l->get(0);
    std::auto_ptr<Document::Generator> g(
      doc_manager_->createOutputGenerator(
        Document::GT_PUBLIC_REQUEST_PDF,
        _output,
        _lang
      )
    );

    g->getInput() << "<?xml version='1.0' encoding='utf-8'?>"
                  << "<enum_whois>"
                  << "<public_request>"
                  << "<type>" << p->getPDFType() << "</type>"
                  << "<handle type='"
                  << (p->getObjectSize() ? p->getObject(0).type : 0)
                  << "'>"
                  << (p->getObjectSize() ? p->getObject(0).handle : "")
                  << "</handle>"
                  << "<date>"
                  << stringify(p->getCreateTime().date())
                  << "</date>"
                  << "<id>"
                  << p->getId()
                  << "</id>"
                  << "<replymail>"
                  << p->getEmailToAnswer()
                  << "</replymail>"
                  << "</public_request>"
                  << "</enum_whois>";
    g->closeInput();
  }

  virtual PublicRequest* createRequest(
    Type _type
  ) const {
    // TRACE("[CALL] Fred::Request::Manager::createRequest()");
    PublicRequestImpl *request = 0;
    switch(_type) {
      case PRT_AUTHINFO_AUTO_RIF :
        request = new AuthInfoRequestEPPImpl(); break;
      case PRT_AUTHINFO_AUTO_PIF :
        request = new AuthInfoRequestPIFAutoImpl(); break;
      case PRT_AUTHINFO_EMAIL_PIF :
        request = new AuthInfoRequestPIFEmailImpl(); break;
      case PRT_AUTHINFO_POST_PIF :
        request = new AuthInfoRequestPIFPostImpl(); break;
      case PRT_BLOCK_TRANSFER_EMAIL_PIF :
        request = new BlockTransferRequestPIFImpl(); break;
      case PRT_BLOCK_CHANGES_EMAIL_PIF :
        request = new BlockUpdateRequestPIFImpl(); break;
      case PRT_UNBLOCK_TRANSFER_EMAIL_PIF :
        request = new UnBlockTransferRequestPIFImpl(); break;
      case PRT_UNBLOCK_CHANGES_EMAIL_PIF :
        request = new UnBlockUpdateRequestPIFImpl(); break;
      case PRT_BLOCK_TRANSFER_POST_PIF :
        request = new BlockTransferRequestPIFPostImpl(); break;
      case PRT_BLOCK_CHANGES_POST_PIF :
        request = new BlockUpdateRequestPIFPostImpl(); break;
      case PRT_UNBLOCK_TRANSFER_POST_PIF :
        request = new UnBlockTransferRequestPIFPostImpl(); break;
      case PRT_UNBLOCK_CHANGES_POST_PIF :
        request = new UnBlockUpdateRequestPIFPostImpl(); break;
      case PRT_CONDITIONAL_CONTACT_IDENTIFICATION:
        request = new ConditionalContactIdentificationImpl(); break;
      case PRT_CONTACT_IDENTIFICATION:
        request = new ContactIdentificationImpl(); break;
      case PRT_CONTACT_VALIDATION:
        request = new ValidationRequestImpl(); break;
      default:
        throw std::runtime_error("Unknown public request type specified.");
    }
    request->setType(_type);
    request->setManager((Manager *)this);
    return request;
  }

  List *loadRequest(Database::ID id) const {
    Database::Filters::PublicRequest *prf = new Database::Filters::PublicRequestImpl();
    prf->addId().setValue(id);
    Database::Filters::Union uf;
    uf.addFilter(prf);
    List *l = createList();
    l->reload(uf);
    if (l->getCount() != 1) throw NOT_FOUND();
    return l;
  }

  virtual void processRequest(Database::ID _id, bool _invalidate,
                              bool check,
                              const unsigned long long &_request_id) const
  {
    TRACE(boost::format("[CALL] Fred::Request::Manager::processRequest(%1%, %2%)") %
          _id % _invalidate);
    try {
      std::auto_ptr<List> l(loadRequest(_id));
      l->get(0)->process(_invalidate, check, _request_id);
    }
    catch (Database::Exception) { throw SQL_ERROR(); }
  }

  virtual unsigned long long processAuthRequest(
          const std::string &_identification,
          const std::string &_password,
          const unsigned long long &_request_id)
  {
      Database::Connection conn = Database::Manager::acquire();
      Database::Transaction tx(conn);
      Database::Result rid = conn.exec_params(
              "SELECT pr.id FROM public_request_auth pra"
              " JOIN public_request pr ON pr.id = pra.id"
              " WHERE pra.identification = $1::text"
              " FOR UPDATE",
              Database::query_param_list(_identification));
      if (rid.size() != 1)
          throw NOT_FOUND();

      unsigned long long id = rid[0][0];
      std::auto_ptr<List> list(loadRequest(id));
      PublicRequestAuth *request = dynamic_cast<PublicRequestAuth*>(list->get(0));
      if (!request) {
          throw NOT_FOUND();
      }

      request->authenticate(_password);
      request->process(false, true, _request_id);
      tx.commit();
      return request->getObject(0).id;
  }

  virtual std::string getPublicRequestAuthIdentification(
          unsigned long long &_contact_id,
          std::vector<unsigned int> &_request_type_list)
  {
      Database::Connection conn = Database::Manager::acquire();
      Database::Result rid = conn.exec_params(
              "SELECT identification FROM public_request_auth pra"
              " JOIN public_request pr ON (pra.id=pr.id)"
              " JOIN public_request_objects_map prom ON (prom.request_id=pr.id)"
              " WHERE pr.resolve_time IS NULL AND pr.status = 0"
              " AND object_id = $1::integer"
              " AND pr.request_type =ANY ($2::int[])",
              Database::query_param_list
                (_contact_id)
                ("{" + Util::container2comma_list(_request_type_list) + "}"));
      if (rid.size() != 1)
          throw NOT_FOUND();

      return rid[0][0];
  }

  const std::string& getIdentificationMailAuthHostname() const
  {
      if (identification_mail_auth_hostname_.empty()) {
          throw std::runtime_error("not configured");
      }
      else {
          return identification_mail_auth_hostname_;
      }
  }

  bool getDemoMode() const
  {
      return demo_mode_;
  }

  void setIdentificationMailAuthHostname(const std::string &_hostname)
  {
      identification_mail_auth_hostname_ = _hostname;
  }

  void setDemoMode(bool _demo_mode)
  {
      demo_mode_ = _demo_mode;
  }


};

Manager* Manager::create(Domain::Manager    *_domain_manager,
                         Contact::Manager   *_contact_manager,
                         NSSet::Manager     *_nsset_manager,
                         KeySet::Manager    *_keyset_manager,
                         Mailer::Manager    *_mailer_manager,
                         Document::Manager  *_doc_manager,
                         Messages::ManagerPtr _messages_manager)
{

    TRACE("[CALL] Fred::Request::Manager::create()");
    return new ManagerImpl(
      _domain_manager,
      _contact_manager,
      _nsset_manager,
      _keyset_manager,
      _mailer_manager,
      _doc_manager,
      _messages_manager
    );
}

}
}
