#include "shard_master.h"

void RunServer() {
  std::string server_address("127.0.0.1:8081");

  ShardMaster shard_master;

  ::grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, ::grpc::InsecureServerCredentials());

  builder.RegisterService(&shard_master);
  std::unique_ptr<::grpc::Server> server = builder.BuildAndStart();

  std::cout << "Listening on : " << server_address << std::endl;
  server->Wait();
}


int main() {
  RunServer();
  return 0;
}
