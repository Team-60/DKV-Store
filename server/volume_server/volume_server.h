#include <iostream>
#include <string>
#include <sstream>
#include <chrono>
#include <thread>
#include <mutex>

#include <grpcpp/grpcpp.h>
#include "volume_server.grpc.pb.h"
#include "shard_master.grpc.pb.h"

#include "leveldb/db.h"

#include "utils.h"

using google::protobuf::Empty;

#define TICK_INTERVAL 100

const std::string SHARD_MASTER_ADDR = "127.0.0.1:8080";

class VolumeServer final : public VolumeServerService::Service {

  public:
    VolumeServer(uint db_idx, std::string vs_addr, std::shared_ptr<grpc::Channel> channel) : db_idx(db_idx), vs_addr(vs_addr), sm_stub_(ShardMasterService::NewStub(channel)) {
      // db name
      std::ostringstream buffer;
      buffer << "db-volume-server-" << this->db_idx;
      this->db_name = buffer.str();
      
      // create db
      leveldb::Options options;
      options.create_if_missing = true;

      leveldb::Status status = leveldb::DB::Open(options, this->db_name, &this->db);
      assert (status.ok());

      // ask shard-master to join
      this->requestJoin();

      // setup ticks
      std::thread([&]() -> void {
        while(1) {
          this->fetchSMConfig();
          std::this_thread::sleep_for(std::chrono::milliseconds(TICK_INTERVAL));
        }
      }).detach();
    }

    ~VolumeServer() {
      delete this->db;
    }

    grpc::Status Get(grpc::ServerContext* context, const GetRequest* request, GetResponse* response) override;

    grpc::Status Put(grpc::ServerContext* context, const PutRequest* request, Empty* response) override;

    grpc::Status Delete(grpc::ServerContext* context, const DeleteRequest* request, Empty* response) override;

    void requestJoin();

    void fetchSMConfig() {
      Empty request;

      QueryConfigNumResponse response;
      grpc::ClientContext context;
      grpc::Status status = sm_stub_->QueryConfigNum(&context, request, &response);
      assert (status.ok());

      if (this->config_num < response.config_num()) {
        // fetch config & update
        Empty request_;
        QueryResponse new_config_response;
        grpc::ClientContext context_;
        grpc::Status status_ = sm_stub_->Query(&context_, request_, &new_config_response);
        assert (status.ok());
        
        mtx.lock(); // lock
        std::cout << "VS" << this->db_idx << ") Updating config! " << this->config_num << "->" << response.config_num() << ".\n";
        // update config number
        this->config_num = response.config_num();
        // update config
        this->config.clear();
        auto new_config = new_config_response.config();
        for (int entry = 0; entry < new_config.size(); ++entry) {
          SMConfigEntry smce;
          smce.vs_addr = new_config[entry].server_addr();
          auto shards = new_config[entry].shards();
          for (int shard_idx = 0; shard_idx < shards.size(); ++ shard_idx) {
            SMShard nshard;
            nshard.lower = shards[shard_idx].lower(), nshard.upper = shards[shard_idx].upper();
            smce.shards.push_back(nshard);
          }
          this->config.push_back(smce);
        }
        mtx.unlock(); // unlock
        this->printCurrentConfig();
      }
    }

  private:
    uint db_idx;
    std::string vs_addr;
    std::string db_name;
    leveldb::DB* db;
    // data members for volume server config (fetched from SM)
    std::unique_ptr<ShardMasterService::Stub> sm_stub_;
    std::mutex mtx; // for "config" exclusion while reading and writing
    uint config_num;
    std::vector<SMConfigEntry> config;

    void printCurrentConfig() {
      std::cout << "VS" << this->db_idx << ") Current config\n";
      for (int entry = 0; entry < (int) this->config.size(); ++entry) {
        std::cout << this->config[entry].vs_addr << ": ";
        auto &shards = this->config[entry].shards;
        for (int shard_idx = 0; shard_idx < (int) shards.size(); ++ shard_idx) {
          std::cout << "{" << shards[shard_idx].lower << ", " << shards[shard_idx].upper << "} ";
        }
        std::cout << '\n';
      }
      std::cout << std::endl;
    }

};
