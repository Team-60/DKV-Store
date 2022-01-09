#ifndef VOLUME_SERVER
#define VOLUME_SERVER

#include <grpcpp/grpcpp.h>

#include <chrono>
#include <iostream>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <thread>

#include "concurrentqueue/concurrentqueue.h"
#include "leveldb/db.h"
#include "md5.h"
#include "shard_master.grpc.pb.h"
#include "utils.h"
#include "volume_server.grpc.pb.h"
#include "thread-pool/thread_pool.hpp"

using google::protobuf::Empty;

#define TICK_INTERVAL 100

const std::string SHARD_MASTER_ADDR = "127.0.0.1:8080";

namespace grpc_status_msg {
const std::string KEY_NOT_FOUND = "Key not found!";
const std::string KEY_EXISTS = "Key already exists!";
const std::string BAD_REQUEST = "Wrong volume server for key!";
}  // namespace grpc_status_msg

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
    assert(status.ok());

    // form mod_map
    this->formModMap();

    // ask shard-master to join
    this->requestJoin();

    // setup ticks
    std::thread([&]() -> void {
      while (1) {
        this->fetchSMConfig();
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_INTERVAL));
      }
    }).detach();
  }

  ~VolumeServer() { delete this->db; }

  grpc::Status Get(grpc::ServerContext* context, const GetRequest* request, GetResponse* response) override;

  grpc::Status Put(grpc::ServerContext* context, const PutRequest* request, Empty* response) override;

  grpc::Status Delete(grpc::ServerContext* context, const DeleteRequest* request, Empty* response) override;

 private:
  uint db_idx;
  std::string vs_addr;
  std::string db_name;
  leveldb::DB* db;
  // data members for volume server config (fetched from SM)
  std::unique_ptr<ShardMasterService::Stub> sm_stub_;
  std::mutex config_mtx;  // for "config" exclusion while reading and writing
  uint config_num;
  const uint NUM_CHUNKS = 1000;  // in accordance with NUM_CHUNKS of shard-master
  std::vector<SMConfigEntry> config;
  SMConfigEntry my_config;
  // data members for move utils
  std::map<uint, std::set<std::string>> mod_map;  // maps shards to respective keys, IMP keep it consistent with leveldb
  std::mutex mod_map_mtx;
  moodycamel::ConcurrentQueue<std::string> move_queue;

  void formModMap();
  void requestJoin();
  void fetchSMConfig();
  bool isMyKey(const std::string& key);

  void printCurrentConfig() {
    std::cout << "VS" << this->db_idx << ") Current config\n";
    for (int entry = 0; entry < (int)this->config.size(); ++entry) {
      std::cout << this->config[entry].vs_addr << ": ";
      auto& shards = this->config[entry].shards;
      for (int shard_idx = 0; shard_idx < (int)shards.size(); ++shard_idx) {
        std::cout << "{" << shards[shard_idx].lower << ", " << shards[shard_idx].upper << "} ";
      }
      std::cout << '\n';
    }
    std::cout << std::endl;
  }
};

#endif  // VOLUME_SERVER
