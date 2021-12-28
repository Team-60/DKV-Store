#include <grpcpp/grpcpp.h>
#include "master.grpc.pb.h"

class Client {
  std::unique_ptr<master::Stub> stub_;
  public: 
    Client(std::shared_ptr<grpc::Channel> channel) : stub_(master::NewStub(channel)) {}

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
};

int main() {

  std::string server_address("127.0.0.1:8080");
  Client client(grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials()));
  std::cout << "Response : " << client.Get("Random Key") << std::endl;

  return 0;
}

