#include <iostream>
#include <string>
#include <sstream>

#include "leveldb/db.h"
#include <grpcpp/grpcpp.h>
#include "volume_server.grpc.pb.h"

class VolumeServer final : public VolumeServerService::Service {

  public:
    VolumeServer(int server_id_) : server_id(server_id_) {
      // db name
      std::ostringstream buffer;
      buffer << "db-volume-server-" << this->server_id;
      this->db_name = buffer.str();
      
      // create db
      leveldb::Options options;
      options.create_if_missing = true;

      leveldb::Status status = leveldb::DB::Open(options, this->db_name, &this->db);
      assert (status.ok());
    }

    ~VolumeServer() {
      delete this->db;
    }

    grpc::Status Get(grpc::ServerContext* context, const GetRequest* request, GetResponse* response) override;

    grpc::Status Put(grpc::ServerContext* context, const PutRequest* request, google::protobuf::Empty* response) override;

    grpc::Status Delete(grpc::ServerContext* context, const DeleteRequest* request, google::protobuf::Empty* response) override;

  private:
    int server_id;
    std::string db_name;
    leveldb::DB* db;

};
