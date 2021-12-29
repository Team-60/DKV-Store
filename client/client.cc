#include <grpcpp/grpcpp.h>
#include "master.grpc.pb.h"

class Client {
  std::unique_ptr<Master::Stub> stub_;
  
  public: 
    Client(std::shared_ptr<grpc::Channel> channel) : stub_(Master::NewStub(channel)) {}

    std::string Get(const std::string& key) {
      GetRequest request;
      request.set_key(key);
      
      GetResponse response;

      grpc::ClientContext context;

      grpc::Status status = stub_->Get(&context, request, &response);

      if (status.ok()) {
        return response.value();
      }else {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        return "RPC Failed";
      }
    }

    std::string Put(const std::string& key, const std::string& value) {
      PutRequest request;
      request.set_key(key);
      request.set_value(value);

      google::protobuf::Empty response;

      grpc::ClientContext context;

      grpc::Status status = stub_->Put(&context, request, &response);

      if (status.ok()) {
        return "Success";
      }else {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        return "RPC Failed";
      }
    }

    std::string Delete(const std::string& key, const std::string& value) {
      DeleteRequest request;
      request.set_key(key);
      request.set_value(value);

      google::protobuf::Empty response;

      grpc::ClientContext context;

      grpc::Status status = stub_->Delete(&context, request, &response);

      if (status.ok()) {
        return "Success";
      }else {
        std::cout << status.error_code() << ": " << status.error_message() << std::endl;
        return "RPC Failed";
      }
    }

};

int main() {
  std::string server_address("127.0.0.1:8080");
  Client client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));

  // simple non-exhaustive sanity check

  // key not found
  std::cout << "Response : " << client.Get("key1") << '\n' << std::endl;

  // success
  std::cout << "Response : " << client.Put("key1", "value1") << '\n'  << std::endl;

  // success
  std::cout << "Response : " << client.Get("key1") << '\n' << std::endl;

  // key already exists
  std::cout << "Response : " << client.Put("key1", "value1") << '\n' << std::endl;

  // success
  std::cout << "Response : " << client.Put("key2", "value2") << '\n' << std::endl;

  // key not found
  std::cout << "Response : " << client.Delete("key3", "value3") << '\n' << std::endl;

  // success
  std::cout << "Response : " << client.Delete("key2", "value2") << '\n' << std::endl;

  // key not found
  std::cout << "Response : " << client.Get("key2") << '\n' << std::endl;

  return 0;
}

