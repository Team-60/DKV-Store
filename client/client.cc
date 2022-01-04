#include <grpcpp/grpcpp.h>

#include "md5.cc"
#include "shard_master.grpc.pb.h"
#include "utils.h"
#include "volume_server.grpc.pb.h"

using google::protobuf::Empty;

const std::string SHARD_MASTER_ADDR = "127.0.0.1:8080";

class Client {
  std::unique_ptr<VolumeServerService::Stub> vs_stub_;
  std::unique_ptr<ShardMasterService::Stub> sm_stub_;

 public:
  QueryResponse mappings;

  Client(std::shared_ptr<grpc::Channel> channel) : vs_stub_(VolumeServerService::NewStub(channel)), sm_stub_(ShardMasterService::NewStub(channel)) {}

  // volume-server
  std::string Get(const std::string& key) {
    GetRequest request;
    request.set_key(key);

    GetResponse response;
    grpc::ClientContext context;
    grpc::Status status = vs_stub_->Get(&context, request, &response);

    if (status.ok()) {
      return response.value();
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC Failed";
    }
  }

  // volume-server
  std::string Put(const std::string& key, const std::string& value) {
    PutRequest request;
    request.set_key(key);
    request.set_value(value);

    Empty response;
    grpc::ClientContext context;
    grpc::Status status = vs_stub_->Put(&context, request, &response);

    if (status.ok()) {
      return "Success";
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC Failed";
    }
  }

  // volume-server
  std::string Delete(const std::string& key, const std::string& value) {
    DeleteRequest request;
    request.set_key(key);
    request.set_value(value);

    Empty response;
    grpc::ClientContext context;
    grpc::Status status = vs_stub_->Delete(&context, request, &response);

    if (status.ok()) {
      return "Success";
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC Failed";
    }
  }

  // shard-master
  std::string Query() {
    Empty request;

    grpc::ClientContext context;
    grpc::Status status = sm_stub_->Query(&context, request, &this->mappings);

    if (status.ok()) {
      return "Success";
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC Failed";
    }
  }

  void printMappings() {
    std::cout << "> Client mappings\n";
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
};

int main(int argc, char* argv[]) {
  std::string shard_master_addr = (argc > 1) ? argv[1] : SHARD_MASTER_ADDR;
  Client client(grpc::CreateChannel(shard_master_addr, grpc::InsecureChannelCredentials()));

  std::cout << "* Client querying shard master " << client.Query() << std::endl;
  client.printMappings();

  // md5 check
  std::cout << "> md5 of 'key-1': " << md5("key-1") << std::endl;

  // // simple non-exhaustive sanity check

  // // key not found
  // std::cout << "Response:- " << client.Get("key1") << '\n' << std::endl;
  // // success
  // std::cout << "Response:- " << client.Put("key1", "value1") << '\n'  <<
  // std::endl;
  // // success
  // std::cout << "Response:- " << client.Get("key1") << '\n' << std::endl;
  // // key already exists
  // std::cout << "Response:- " << client.Put("key1", "value1") << '\n' <<
  // std::endl;
  // // success
  // std::cout << "Response:- " << client.Put("key2", "value2") << '\n' <<
  // std::endl;
  // // key not found
  // std::cout << "Response:- " << client.Delete("key3", "value3") << '\n' <<
  // std::endl;
  // // success
  // std::cout << "Response:- " << client.Delete("key2", "value2") << '\n' <<
  // std::endl;
  // // key not found
  // std::cout << "Response:- " << client.Get("key2") << '\n' << std::endl;

  std::cout << std::endl;
  return 0;
}
