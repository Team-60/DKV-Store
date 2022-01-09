#include "volume_server.h"

grpc::Status VolumeServer::Get(grpc::ServerContext* context, const GetRequest* request, GetResponse* response) {
  std::cout << "VS" << this->db_idx << ") Get: key=" << request->key() << std::endl;

  if (!isMyKey(request->key())) {
    response->set_key("");
    response->set_value("");
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, grpc_status_msg::BAD_REQUEST);
  }

  std::string value;
  leveldb::Status s = this->db->Get(leveldb::ReadOptions(), request->key(), &value);
  if (!s.ok()) {
    response->set_key("");
    response->set_value("");
    return grpc::Status(grpc::StatusCode::NOT_FOUND, grpc_status_msg::KEY_NOT_FOUND);
  }

  response->set_key(request->key());
  response->set_value(value);
  return grpc::Status::OK;
}

grpc::Status VolumeServer::Put(grpc::ServerContext* context, const PutRequest* request, google::protobuf::Empty* response) {
  std::cout << "VS" << this->db_idx << ") Put: key=" << request->key() << " value=" << request->value() << std::endl;

  if (!isMyKey(request->key())) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, grpc_status_msg::BAD_REQUEST);
  }

  std::string value;
  leveldb::Status s = this->db->Get(leveldb::ReadOptions(), request->key(), &value);
  if (s.ok()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, grpc_status_msg::KEY_EXISTS);
  }

  this->db->Put(leveldb::WriteOptions(), request->key(), request->value());
  return grpc::Status::OK;
}

grpc::Status VolumeServer::Delete(grpc::ServerContext* context, const DeleteRequest* request, google::protobuf::Empty* response) {
  std::cout << "VS" << this->db_idx << ") Delete: key=" << request->key() << " value=" << request->value() << std::endl;

  if (!isMyKey(request->key())) {
    return grpc::Status(grpc::StatusCode::INVALID_ARGUMENT, grpc_status_msg::BAD_REQUEST);
  }

  std::string value;
  leveldb::Status s = this->db->Get(leveldb::ReadOptions(), request->key(), &value);
  if (!s.ok()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, grpc_status_msg::KEY_NOT_FOUND);
  }

  this->db->Delete(leveldb::WriteOptions(), request->key());
  return grpc::Status::OK;
}

void VolumeServer::requestJoin() {
  // initialises config number, config
  this->config.clear();
  this->config_num = 0;

  JoinRequest request;
  grpc::ClientContext context;
  Empty response;
  request.set_server_addr(vs_addr);

  grpc::Status status = this->sm_stub_->Join(&context, request, &response);
  if (!status.ok()) {
    std::cerr << status.error_message() << std::endl;
    exit(-1);
  }
}

void VolumeServer::fetchSMConfig() {
  // periodically fetches config from shard-master & incorporates changes (if
  // any)
  Empty request;
  QueryConfigNumResponse response;
  grpc::ClientContext context;
  grpc::Status status = sm_stub_->QueryConfigNum(&context, request, &response);

  if (!status.ok()) {
    std::cerr << status.error_message() << std::endl;
    exit(-1);
  }

  if (this->config_num < response.config_num()) {
    // fetch config & update
    Empty request_;
    QueryResponse new_config_response;
    grpc::ClientContext context_;
    grpc::Status status_ = this->sm_stub_->Query(&context_, request_, &new_config_response);
    assert(status.ok());

    this->mtx.lock();  // lock
    std::cout << "VS" << this->db_idx << ") Updating config! " << this->config_num << "->" << response.config_num() << ".\n";
    // update config number
    this->config_num = response.config_num();
    // update config
    this->config.clear();
    auto new_config = new_config_response.config();
    for (int entry = 0; entry < new_config.size(); ++entry) {
      SMConfigEntry smce;
      smce.vs_addr = new_config[entry].server_addr();
      auto shards = new_config[entry].shards();
      for (int shard_idx = 0; shard_idx < shards.size(); ++shard_idx) {
        SMShard nshard;
        nshard.lower = shards[shard_idx].lower(), nshard.upper = shards[shard_idx].upper();
        smce.shards.push_back(nshard);
      }
      if (this->vs_addr == smce.vs_addr) {
        this->my_config = smce;
      }
      this->config.push_back(smce);
    }
    this->printCurrentConfig();
    this->mtx.unlock();  // unlock
  }
}

bool VolumeServer::isMyKey(const std::string& key) {
  std::string hash = md5(key);
  uint hash_int = get_hash_uint(hash);
  uint shard_mod = hash_int % this->NUM_CHUNKS;

  for (SMShard& shard : my_config.shards) {
    if (shard.lower <= shard_mod && shard_mod <= shard.upper) {
      return true;
    }
  }
  return false;
}



