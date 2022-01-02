#include <iostream>
#include <string>

#include <grpcpp/grpcpp.h>
#include "shard_master.grpc.pb.h"

#include "leveldb/db.h"

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

      this->num_vservers = 0;
      this->vserver_addr.clear();
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
    int num_vservers;
    std::vector<std::string> vserver_addr;
    leveldb::DB* db;

};
