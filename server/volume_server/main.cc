#include "volume_server.h"

void RunServer() {
  std::string server_address("127.0.0.1:8080");

  VolumeServer volume_server(1);

  ::grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, ::grpc::InsecureServerCredentials());

  builder.RegisterService(&volume_server);
  std::unique_ptr<::grpc::Server> server = builder.BuildAndStart();

  std::cout << "Listening on : " << server_address << std::endl;
  server->Wait();
}


int main() {
  RunServer();
  return 0;
}
