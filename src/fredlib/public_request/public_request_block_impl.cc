#include "public_request_impl.h"
#include "public_request_block_impl.h"
#include "src/fredlib/object_states.h"
#include "factory.h"
#include "util.h"
#include "types/stringify.h"



namespace Fred {
namespace PublicRequest {

FACTORY_MODULE_INIT_DEFI(block)

class BlockUnblockRequestPIFImpl : public PublicRequestImpl {
public:
  virtual std::string getTemplateName() const {
    return "request_block";
  }
  virtual short blockType() const = 0;
  virtual short blockAction() const = 0;
  virtual void fillTemplateParams(Mailer::Parameters& params) const {
    params["reqdate"] = stringify(getCreateTime().date());
    params["reqid"] = stringify(getId());
    if (getObjectSize()) {
      params["type"] = stringify(getObject(0).type);
      params["handle"] = getObject(0).handle;
    }
    params["otype"] = stringify(blockType());
    params["rtype"] = stringify(blockAction());
  }
};

class BlockTransferRequestPIFImpl
        : public BlockUnblockRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, BlockTransferRequestPIFImpl>
{
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = !object_has_state(getObject(i).id, ObjectState::SERVER_UPDATE_PROHIBITED) &&
      queryBlockRequest(getObject(i).id,0
          , Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
          ,false);
    return res;
  }
  virtual short blockType() const {
    return 1;
  }
  virtual short blockAction() const {
    return 2;
  }
    virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
        if (!object_has_state(getObject(i).id, ObjectState::SERVER_UPDATE_PROHIBITED) &&
              queryBlockRequest(getObject(i).id, getId()
                  , Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
                  , false))
        {
            insertNewStateRequest(getId(),getObject(i).id
                , ObjectState::SERVER_TRANSFER_PROHIBITED);
        } else throw REQUEST_BLOCKED();

      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";
      conn.exec(q);
    }
    }

    static std::string registration_name()
    {
        return PRT_BLOCK_TRANSFER_EMAIL_PIF;
    }
};

class BlockUpdateRequestPIFImpl
        : public BlockUnblockRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, BlockUpdateRequestPIFImpl>
{
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i = 0; res && i < getObjectSize(); i++)
     res = !object_has_state(getObject(i).id, ObjectState::SERVER_UPDATE_PROHIBITED) &&
      queryBlockRequest(getObject(i).id, 0
          , Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
              (ObjectState::SERVER_UPDATE_PROHIBITED)
          , false);
    return res;
  }
  virtual short blockType() const {
    return 1;
  }
  virtual short blockAction() const {
    return 1;
  }
    virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
        if (!object_has_state(getObject(i).id, ObjectState::SERVER_UPDATE_PROHIBITED) &&
              queryBlockRequest(getObject(i).id, getId()
                  , Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
                      (ObjectState::SERVER_UPDATE_PROHIBITED)
                  , false))
        {
            insertNewStateRequest(getId(),getObject(i).id
                    , ObjectState::SERVER_TRANSFER_PROHIBITED);
            insertNewStateRequest(getId(),getObject(i).id
                    , ObjectState::SERVER_UPDATE_PROHIBITED);
        } else throw REQUEST_BLOCKED();
      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";
      conn.exec(q);
    }
    }

    static std::string registration_name()
    {
        return PRT_BLOCK_CHANGES_EMAIL_PIF;
    }
};

class UnBlockTransferRequestPIFImpl
        : public BlockUnblockRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, UnBlockTransferRequestPIFImpl>
{
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id,0
         ,Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
         ,true);
    return res;
  }
  virtual short blockType() const {
    return 2;
  }
  virtual short blockAction() const {
    return 2;
  }
    virtual void processAction(bool check)
    {
        Database::Connection conn = Database::Manager::acquire();
        for (unsigned i = 0; i < getObjectSize(); i++)
        {
            if (!queryBlockRequest(getObject(i).id, getId()
                , Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
                , true))
            throw REQUEST_BLOCKED();
            Fred::cancel_object_state(getObject(i).id, ObjectState::SERVER_TRANSFER_PROHIBITED);
            Fred::update_object_states(getObject(i).id);
        }
    }

    static std::string registration_name()
    {
        return PRT_UNBLOCK_TRANSFER_EMAIL_PIF;
    }
};

class UnBlockUpdateRequestPIFImpl
        : public BlockUnblockRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, UnBlockUpdateRequestPIFImpl>
{
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i = 0; res && i < getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id, 0
         , Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
             (ObjectState::SERVER_UPDATE_PROHIBITED)
         , true);
    return res;
  }
  virtual short blockType() const {
    return 2;
  }
  virtual short blockAction() const {
    return 1;
  }
    virtual void processAction(bool check)
    {
        Database::Connection conn = Database::Manager::acquire();
        for (unsigned i = 0; i < getObjectSize(); i++)
        {
            if (!queryBlockRequest(getObject(i).id, getId()
                , Util::vector_of<std::string>(ObjectState::SERVER_TRANSFER_PROHIBITED)
                    (ObjectState::SERVER_UPDATE_PROHIBITED)
                , true))
            throw REQUEST_BLOCKED();

            Fred::cancel_object_state(getObject(i).id, ObjectState::SERVER_TRANSFER_PROHIBITED);
            Fred::cancel_object_state(getObject(i).id, ObjectState::SERVER_UPDATE_PROHIBITED);
            Fred::update_object_states(getObject(i).id);
        }
    }

    static std::string registration_name()
    {
        return PRT_UNBLOCK_CHANGES_EMAIL_PIF;
    }
};

class BlockTransferRequestPIFPostImpl
        : public BlockTransferRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, BlockTransferRequestPIFPostImpl>
{
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 2;
  }

  static std::string registration_name()
  {
      return PRT_BLOCK_TRANSFER_POST_PIF;
  }
};

class BlockUpdateRequestPIFPostImpl
        : public BlockUpdateRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, BlockUpdateRequestPIFPostImpl>
{
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 4;
  }

  static std::string registration_name()
  {
      return PRT_BLOCK_CHANGES_POST_PIF;
  }
};

class UnBlockTransferRequestPIFPostImpl
        : public UnBlockTransferRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, UnBlockTransferRequestPIFPostImpl>
{
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 3;
  }

  static std::string registration_name()
  {
      return PRT_UNBLOCK_TRANSFER_POST_PIF;
  }
};

class UnBlockUpdateRequestPIFPostImpl
        : public UnBlockUpdateRequestPIFImpl,
          public Util::FactoryAutoRegister<PublicRequest, UnBlockUpdateRequestPIFPostImpl>
{
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 5;
  }

  static std::string registration_name()
  {
      return PRT_UNBLOCK_CHANGES_POST_PIF;
  }
};


}
}

