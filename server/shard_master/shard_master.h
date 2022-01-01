#include <iostream>
#include <string>

#include <grpcpp/grpcpp.h>
#include "shard_master.grpc.pb.h"

#include "leveldb/db.h"

using google::protobuf::Empty;

class ShardMaster final : public ShardMasterService::Service {

  public:
    ShardMaster (const std::vector<std::string>& vserver_addr) : numVServers(vserver_addr.size()) {
      
      leveldb::Options options;
      options.create_if_missing = true;

      leveldb::Status openDBStatus = leveldb::DB::Open(options, this->db_name, &this->db);

      if (!openDBStatus.ok()) {
        std::cerr << "Error : Can't open leveldb" << '\n';
        exit(-1);
      }

      // Seralize into db
      leveldb::Status putStatus = this->db->Put(leveldb::WriteOptions(), "numVServers", std::to_string(numVServers));
      assert(putStatus.ok());

      for (int i = 0; i < numVServers; ++i) {
        putStatus = this->db->Put(leveldb::WriteOptions(), std::to_string(i), vserver_addr[i]);
        assert(putStatus.ok());
      }

    }

    ShardMaster () {
      leveldb::Options options;
      options.create_if_missing = false;

      leveldb::Status openDBStatus = leveldb::DB::Open(options, this->db_name, &this->db);

      if (!openDBStatus.ok()) {
        std::cerr << "Error : Can't open leveldb" << '\n';
        exit(-1);
      }

      std::string numVServers_str;
      leveldb::Status getStatus = this->db->Get(leveldb::ReadOptions(), "numVServers", &numVServers_str);
      assert(getStatus.ok());
      numVServers = std::stoi(numVServers_str);

      vserver_addr.resize(numVServers);

      for (int i = 0; i < numVServers; ++i) {
        getStatus = this->db->Get(leveldb::ReadOptions(), std::to_string(i), &vserver_addr[i]);
        assert(getStatus.ok());
      }

      std::cout << "Shardmaster restarted with " << numVServers << " servers, with addresses:" << "\n";
      for (auto &addr : vserver_addr) {
        std::cout << addr << '\n';
      }
      std::cout << std::flush;
    }

    ~ShardMaster () {
    }

    grpc::Status Query(grpc::ServerContext* context, const Empty* request, QueryResponse* response) override;

    grpc::Status Move(grpc::ServerContext* context, const MoveRequest* request, Empty* response) override;

    grpc::Status Join(grpc::ServerContext* context, const JoinRequest* request, JoinResponse* response) override;

    grpc::Status Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) override;

  private:
    const std::string db_name = "db-shard-master";
    int numVServers;
    std::vector<std::string> vserver_addr;
    leveldb::DB* db;
};
