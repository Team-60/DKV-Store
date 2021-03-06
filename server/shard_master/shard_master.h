#ifndef SHARD_MASTER
#define SHARD_MASTER

#include <grpcpp/grpcpp.h>

#include <iostream>
#include <string>
#include <vector>

#include "leveldb/db.h"
#include "shard_master.grpc.pb.h"
#include "utils.h"

using google::protobuf::Empty;

namespace grpc_status_msg {
const std::string VS_NOT_FOUND = "Server doesn't exist!";
const std::string VS_EXISTS = "Server already exists!";
}  // namespace grpc_status_msg

class ShardMaster final : public ShardMasterService::Service {
 public:
  ShardMaster() {
    leveldb::Options options;
    options.create_if_missing = true;

    leveldb::Status openDBStatus = leveldb::DB::Open(options, this->db_name, &this->db);

    if (!openDBStatus.ok()) {
      std::cerr << "Error: Can't open leveldb" << '\n';
      exit(-1);
    }

    // config
    this->config_num = 0;  // INITIAL CONFIGURATION IS 0
    this->sm_config.clear();
  }

  ~ShardMaster() { delete this->db; }

  grpc::Status Query(grpc::ServerContext* context, const Empty* request, QueryResponse* response) override;

  grpc::Status QueryConfigNum(grpc::ServerContext* context, const Empty* request, QueryConfigNumResponse* response) override;

  grpc::Status Move(grpc::ServerContext* context, const MoveRequest* request, Empty* response) override;

  grpc::Status Join(grpc::ServerContext* context, const JoinRequest* request, Empty* response) override;

  grpc::Status Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) override;

 private:
  const std::string db_name = "db-shard-master";
  leveldb::DB* db;
  // config
  std::vector<SMConfigEntry> sm_config;
  uint config_num;
  const uint NUM_CHUNKS = 1000;
  std::mutex mtx;

  void redistributeChunks();
};

#endif  //SHARD_MASTER
