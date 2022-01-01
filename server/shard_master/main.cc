#include <string>

#include "shard_master.cc"

void RunServer(std::string server_address) {
  ShardMaster shard_master({"127.0.0.1:8080", "127:0.0.2:8080"});

  ::grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, ::grpc::InsecureServerCredentials());

  builder.RegisterService(&shard_master);
  std::unique_ptr<::grpc::Server> server = builder.BuildAndStart();

  std::cout << "* Shardmaster: Listening on " << server_address << std::endl;
  server->Wait();
}


int main(int argc, char* argv[]) {
  std::string addr = (argc > 1) ? argv[1]: "127.0.0.1:8080";
  RunServer(addr);
  return 0;
}
