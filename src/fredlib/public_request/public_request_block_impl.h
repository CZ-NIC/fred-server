#ifndef PUBLIC_REQUEST_BLOCK_IMPL_H_
#define PUBLIC_REQUEST_BLOCK_IMPL_H_

#include "public_request_impl.h"

#define SERVER_TRANSFER_PROHIBITED 3
#define SERVER_UPDATE_PROHIBITED 4


namespace Fred {
namespace PublicRequest {



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

class BlockTransferRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = !checkState(getObject(i).id,SERVER_UPDATE_PROHIBITED) &&
      queryBlockRequest(getObject(i).id,0,"3",false);
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
        if (!checkState(getObject(i).id, SERVER_UPDATE_PROHIBITED) &&
              queryBlockRequest(getObject(i).id, getId(), "3", false)) {
            insertNewStateRequest(getId(),getObject(i).id, 3);
        } else throw REQUEST_BLOCKED();

      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";
      conn.exec(q);
    }
    }
};

class BlockUpdateRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i = 0; res && i < getObjectSize(); i++)
     res = !checkState(getObject(i).id, SERVER_UPDATE_PROHIBITED) &&
      queryBlockRequest(getObject(i).id, 0, "3,4", false);
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
        if (!checkState(getObject(i).id, SERVER_UPDATE_PROHIBITED) &&
              queryBlockRequest(getObject(i).id, getId(), "3,4", false)) {
            insertNewStateRequest(getId(),getObject(i).id, SERVER_TRANSFER_PROHIBITED);
            insertNewStateRequest(getId(),getObject(i).id, SERVER_UPDATE_PROHIBITED);
        } else throw REQUEST_BLOCKED();
      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";
      conn.exec(q);
    }
    }
};

class UnBlockTransferRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i=0; res && i<getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id,0,"3",true);
    return res;
  }
  virtual short blockType() const {
    return 2;
  }
  virtual short blockAction() const {
    return 2;
  }
    virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
        if (!queryBlockRequest(getObject(i).id, getId(), "3", true))
        throw REQUEST_BLOCKED();
      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";
      conn.exec(q);
    }
    }
};

class UnBlockUpdateRequestPIFImpl : public BlockUnblockRequestPIFImpl {
public:
  virtual bool check() const {
    bool res = true;
    for (unsigned i = 0; res && i < getObjectSize(); i++)
     res = queryBlockRequest(getObject(i).id, 0, "3,4", true);
    return res;
  }
  virtual short blockType() const {
    return 2;
  }
  virtual short blockAction() const {
    return 1;
  }
    virtual void processAction(bool check) {
    Database::Connection conn = Database::Manager::acquire();
    for (unsigned i = 0; i < getObjectSize(); i++) {
        if (!queryBlockRequest(getObject(i).id, getId(), "3,4", true))
        throw REQUEST_BLOCKED();
      Database::Query q;
      q.buffer() << "SELECT update_object_states(" << getObject(i).id << ")";
      conn.exec(q);
    }
    }
};

class BlockTransferRequestPIFPostImpl : public BlockTransferRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 2;
  }
};

class BlockUpdateRequestPIFPostImpl : public BlockUpdateRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 4;
  }
};

class UnBlockTransferRequestPIFPostImpl : public UnBlockTransferRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 3;
  }
};

class UnBlockUpdateRequestPIFPostImpl : public UnBlockUpdateRequestPIFImpl {
public:
  /// return proper type for PDF letter template
  virtual unsigned getPDFType() const {
    return 5;
  }
};


}
}

#endif /* PUBLIC_REQUEST_BLOCK_IMPL_H_ */


