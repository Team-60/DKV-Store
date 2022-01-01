#include <cstdlib>
#include <string>

#include "volume_server.cc"

void RunServer(std::string server_address, int server_id) {
  VolumeServer volume_server(server_id);

  ::grpc::ServerBuilder builder;
  builder.AddListeningPort(server_address, ::grpc::InsecureServerCredentials());

  builder.RegisterService(&volume_server);
  std::unique_ptr<::grpc::Server> server = builder.BuildAndStart();

  std::cout << "* VolumeServer " << server_id << ": Listening on " << server_address << std::endl;
  server->Wait();
}


int main(int argc, char* argv[]) {
  std::string server_address = (argc > 1) ? argv[1]: "127.0.0.1:8081";
  int server_id = (argc > 2) ? atoi(argv[2]): 1;
  RunServer(server_address, server_id);
  return 0;
}
