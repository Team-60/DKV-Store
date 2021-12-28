
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>

#include "master.grpc.pb.h"

class MasterImpl final : public master::Service {

  ::grpc::Status Get(::grpc::ServerContext* context, const GetRequest* request, GetResponse* response) override {
    response->set_key(request->key());
    response->set_value(std::string("This is working"));
    return ::grpc::Status::OK;
  }

};
