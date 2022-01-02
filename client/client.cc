#include <grpcpp/grpcpp.h>

#include "volume_server.grpc.pb.h"
#include "shard_master.grpc.pb.h"

using google::protobuf::Empty;

class Client {
  std::unique_ptr<VolumeServerService::Stub> vs_stub_;
  std::unique_ptr<ShardMasterService::Stub> sm_stub_;
  
  public: 
    Client(std::shared_ptr<grpc::Channel> channel) : vs_stub_(VolumeServerService::NewStub(channel)), sm_stub_(ShardMasterService::NewStub(channel)) {}

    std::string Get(const std::string& key) { // volume-server
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

    std::string Put(const std::string& key, const std::string& value) { // volume-server
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

    std::string Delete(const std::string& key, const std::string& value) { // volume-server
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

    void Query() { // shard-master
      Empty request;

      QueryResponse response;
      grpc::ClientContext context;
      grpc::Status status = sm_stub_->Query(&context, request, &response);

      if (status.ok()) {
        std::cout << "Success";
      } else {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        std::cout << "RPC Failed";
      }
    }

};

int main(int argc, char* argv[]) {
  std::string shard_master_addr = (argc > 1) ? argv[1]: "127.0.0.1:8080";
  Client client(grpc::CreateChannel(shard_master_addr, grpc::InsecureChannelCredentials()));

  std::cout << "* Client querying shard master: " << std::endl;
  client.Query();

  // // simple non-exhaustive sanity check

  // // key not found
  // std::cout << "Response:- " << client.Get("key1") << '\n' << std::endl;
  // // success
  // std::cout << "Response:- " << client.Put("key1", "value1") << '\n'  << std::endl;
  // // success
  // std::cout << "Response:- " << client.Get("key1") << '\n' << std::endl;
  // // key already exists
  // std::cout << "Response:- " << client.Put("key1", "value1") << '\n' << std::endl;
  // // success
  // std::cout << "Response:- " << client.Put("key2", "value2") << '\n' << std::endl;
  // // key not found
  // std::cout << "Response:- " << client.Delete("key3", "value3") << '\n' << std::endl;
  // // success
  // std::cout << "Response:- " << client.Delete("key2", "value2") << '\n' << std::endl;
  // // key not found
  // std::cout << "Response:- " << client.Get("key2") << '\n' << std::endl;

  return 0;
}

