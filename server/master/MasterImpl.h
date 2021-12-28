#include <iostream>
#include <string>
#include <vector>
#include <map>

#include <grpcpp/grpcpp.h>

#include "master.grpc.pb.h"

class MasterImpl final : public Master::Service {

  public:
    grpc::Status Get(grpc::ServerContext* context, const GetRequest* request, GetResponse* response) override {

      std::cout << "Get : key=" << request->key() << std::endl;

      if (keyValueMap.find(request->key()) == keyValueMap.end()) {
        response->set_key("");
        response->set_value("");
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "key not found");
      }

      response->set_key(request->key());
      response->set_value(keyValueMap[request->key()]);

      return grpc::Status::OK;
    }

    grpc::Status Put(grpc::ServerContext* context, const PutRequest* request, google::protobuf::Empty* response) override {

      std::cout << "Put : key=" << request->key() << " Value=" << request->value() << std::endl;

      if (keyValueMap.find(request->key()) != keyValueMap.end()) {
        return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "key already exists");
      }

      keyValueMap[request->key()] = request->value();

      return grpc::Status::OK;
    }

    grpc::Status Delete(grpc::ServerContext* context, const DeleteRequest* request, google::protobuf::Empty* response) override {
      std::cout << "Delete : key=" << request->key() << " Value=" << request->value() << std::endl;

      if (keyValueMap.find(request->key()) == keyValueMap.end()) {
        return grpc::Status(grpc::StatusCode::NOT_FOUND, "key not found");
      }

      keyValueMap.erase(request->key());

      return grpc::Status::OK;
    }

  private:
    std::map<std::string, std::string> keyValueMap;

};
