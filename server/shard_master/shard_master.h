#include <iostream>
#include <string>
#include <vector>

#include <grpcpp/grpcpp.h>
#include "shard_master.grpc.pb.h"

#include "leveldb/db.h"

#include "utils.h"

using google::protobuf::Empty;

class ShardMaster final : public ShardMasterService::Service {

  public:
    ShardMaster () {
      leveldb::Options options;
      options.create_if_missing = true;

      leveldb::Status openDBStatus = leveldb::DB::Open(options, this->db_name, &this->db);

      if (!openDBStatus.ok()) {
        std::cerr << "Error: Can't open leveldb" << '\n';
        exit(-1);
      }

      // config
      this->sm_config.clear();

      // dummy data to test Query()
      SMConfigEntry smce;
      SMShard s1, s2;
      s1.lower = 0, s1.upper = 2;
      s2.lower = 4, s2.upper = 6;
      smce.vs_addr = "127.0.0.1:8081";
      smce.shards.push_back(s1);
      smce.shards.push_back(s2);
      this->sm_config.push_back(smce);
    }

    ~ShardMaster () {
      delete this->db;
    }

    grpc::Status Query(grpc::ServerContext* context, const Empty* request, QueryResponse* response) override;

    grpc::Status Move(grpc::ServerContext* context, const MoveRequest* request, Empty* response) override;

    grpc::Status Join(grpc::ServerContext* context, const JoinRequest* request, JoinResponse* response) override;

    grpc::Status Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) override;

  private:
    const std::string db_name = "db-shard-master";
    leveldb::DB* db;

    std::vector<SMConfigEntry> sm_config;

};
