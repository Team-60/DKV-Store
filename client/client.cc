#include <grpcpp/grpcpp.h>

#include <map>

#include "md5.h"
#include "shard_master.grpc.pb.h"
#include "utils.h"
#include "volume_server.grpc.pb.h"

using google::protobuf::Empty;

const std::string SHARD_MASTER_ADDR = "127.0.0.1:8080";

namespace query_status {
const std::string KEY_NOT_EXISTS = "Volume server for corresponding key doesn't exist!";
const std::string RPC_FAIL = "RPC Failed!";
const std::string RPC_FAIL_RESET = "RPC Failed! Resetting config";
const std::string OK = "Success!";
}  // namespace query_status

class Client {
  std::map<std::string, std::unique_ptr<VolumeServerService::Stub>> vs_stub_;
  std::unique_ptr<ShardMasterService::Stub> sm_stub_;

 public:
  Client(std::shared_ptr<grpc::Channel> channel) : sm_stub_(ShardMasterService::NewStub(channel)), config_num(0) {}

  // volume-server
  std::string Get(const std::string& key) {
    this->Query();
    std::string vs_addr = this->getVSAddr(key);
    if (vs_addr.empty()) return query_status::KEY_NOT_EXISTS;
    this->createChannelIfNotExists(vs_addr);

    GetRequest request;
    request.set_key(key);

    GetResponse response;
    grpc::ClientContext context;
    grpc::Status status = this->vs_stub_[vs_addr]->Get(&context, request, &response);

    if (status.ok()) {
      return response.value();
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return query_status::RPC_FAIL;
    }
  }

  // volume-server
  std::string Put(const std::string& key, const std::string& value) {
    this->Query();
    std::string vs_addr = this->getVSAddr(key);
    if (vs_addr.empty()) return query_status::KEY_NOT_EXISTS;
    this->createChannelIfNotExists(vs_addr);

    PutRequest request;
    request.set_key(key);
    request.set_value(value);

    Empty response;
    grpc::ClientContext context;
    grpc::Status status = this->vs_stub_[vs_addr]->Put(&context, request, &response);

    if (status.ok()) {
      return query_status::OK;
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return query_status::RPC_FAIL;
    }
  }

  // volume-server
  std::string Delete(const std::string& key, const std::string& value) {
    this->Query();
    std::string vs_addr = this->getVSAddr(key);
    if (vs_addr.empty()) return query_status::KEY_NOT_EXISTS;
    this->createChannelIfNotExists(vs_addr);

    DeleteRequest request;
    request.set_key(key);
    request.set_value(value);

    Empty response;
    grpc::ClientContext context;
    grpc::Status status = this->vs_stub_[vs_addr]->Delete(&context, request, &response);

    if (status.ok()) {
      return query_status::OK;
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return query_status::RPC_FAIL;
    }
  }

  // shard-master
  std::string Query() {
    // if config_num mismatch, query config
    std::cout << "* Client querying shard master; config_num=" << this->config_num << std::endl;

    Empty request;
    QueryConfigNumResponse response;
    grpc::ClientContext context;
    grpc::Status status = this->sm_stub_->QueryConfigNum(&context, request, &response);
    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return query_status::RPC_FAIL;
    }

    // mismatch
    if (this->config_num < response.config_num()) {
      // fetch new config
      this->mappings.clear_config();
      Empty request_;
      grpc::ClientContext context_;
      grpc::Status status_ = sm_stub_->Query(&context_, request_, &this->mappings);
      if (!status_.ok()) {
        std::cout << status_.error_code() << ": " << status_.error_message() << std::endl;
        this->config_num = 0;
        return query_status::RPC_FAIL_RESET;
      }
      std::cout << "* Client: Updating config! " << this->config_num << "->" << response.config_num() << std::endl;
      this->config_num = response.config_num();
      this->printMappings();
    }

    return query_status::OK;
  }

  // shard-master
  std::string Move(const std::pair<uint, uint>& shard, const std::string& vs_addr) {
    MoveRequest request;
    request.set_server(vs_addr);
    Shard* req_shard = request.mutable_shard();
    req_shard->set_lower(shard.first);
    req_shard->set_upper(shard.second);

    Empty response;
    grpc::ClientContext context;
    grpc::Status status = sm_stub_->Move(&context, request, &response);
    if (!status.ok()) {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return query_status::RPC_FAIL;
    }

    return query_status::OK;
  }

 private:
  QueryResponse mappings;
  uint config_num;
  const uint num_chunks = 1000;

  void printMappings() {
    std::cout << "> Client mappings\n";
    std::cout << "- config_num " << this->config_num << '\n';
    std::cout << "- num_chunks " << this->num_chunks << '\n';
    auto config = this->mappings.config();
    for (int entry = 0; entry < config.size(); ++entry) {
      std::cout << config[entry].server_addr() << ": ";
      auto shards = config[entry].shards();
      for (int shard_idx = 0; shard_idx < shards.size(); ++shard_idx) {
        std::cout << "{" << shards[shard_idx].lower() << ", " << shards[shard_idx].upper() << "} ";
      }
      std::cout << '\n';
    }
    std::cout << std::endl;
  }

  std::string getVSAddr(const std::string& key) {
    // gets volume-server addr
    std::string hash = md5(key);
    std::cout << "> hash: " << key << " " << hash << std::endl;
    uint hash_int = get_hash_uint(hash);
    std::cout << "> hash_int: " << hash << " " << hash_int << std::endl;
    auto config = this->mappings.config();
    uint shard_mod = hash_int % this->num_chunks;
    std::cout << "> shard_mod: " << shard_mod << std::endl;
    // find corresponding volume server
    for (int entry = 0; entry < config.size(); ++entry) {
      auto shards = config[entry].shards();
      for (int shard_idx = 0; shard_idx < shards.size(); ++shard_idx) {
        if (shards[shard_idx].lower() <= shard_mod && shard_mod <= shards[shard_idx].upper()) return config[entry].server_addr();
      }
    }
    // not found
    return "";
  }

  void createChannelIfNotExists(const std::string& vs_addr) {
    if (this->vs_stub_.find(vs_addr) == this->vs_stub_.end()) {
      std::shared_ptr<grpc::Channel> new_channel = grpc::CreateChannel(vs_addr, grpc::InsecureChannelCredentials());
      this->vs_stub_[vs_addr] = VolumeServerService::NewStub(new_channel);
      std::cout << "* New gRPC channel created for vs_addr: " << vs_addr << std::endl;
    }
  }
};

int main(int argc, char* argv[]) {
  std::string shard_master_addr = (argc > 1) ? argv[1] : SHARD_MASTER_ADDR;
  Client client(grpc::CreateChannel(shard_master_addr, grpc::InsecureChannelCredentials()));

  // simple non-exhaustive sanity check

  // key not found
  std::cout << "Response:-\n"
            << client.Get("key1") << '\n'
            << std::endl;
  // success
  std::cout << "Response:-\n"
            << client.Put("key1", "value1") << '\n'
            << std::endl;
  // success
  std::cout << "Response:-\n"
            << client.Get("key1") << '\n'
            << std::endl;
  // key already exists
  std::cout << "Response:-\n"
            << client.Put("key1", "value1") << '\n'
            << std::endl;
  // success
  std::cout << "Response:-\n"
            << client.Put("key2", "value2") << '\n'
            << std::endl;
  // key not found
  std::cout << "Response:-\n"
            << client.Delete("key3", "value3") << '\n'
            << std::endl;
  // success
  std::cout << "Response:-\n"
            << client.Delete("key2", "value2") << '\n'
            << std::endl;
  // key not found
  std::cout << "Response:-\n"
            << client.Get("key2") << '\n'
            << std::endl;
  // try move key
  std::cout << "Response:-\n"
            << client.Move({0, 400}, "127.0.0.1:8081") << '\n'
            << std::endl;

  std::cout << std::endl;
  return 0;
}
