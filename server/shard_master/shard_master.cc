#include "shard_master.h"

grpc::Status ShardMaster::Query(grpc::ServerContext* context, const Empty* request, QueryResponse* response) {
  std::cout << "* Shardmaster: Query called" << std::endl;
  response->clear_config();
  
  // prepare response
  for (const SMConfigEntry &sm_config_entry: this->sm_config) {
    ConfigEntry* config_entry = response->add_config();
    config_entry->set_server_addr(sm_config_entry.vs_addr);
    config_entry->clear_shards();
    for (const SMShard &sm_shard: sm_config_entry.shards) {
      Shard* shard = config_entry->add_shards();
      shard->set_lower(sm_shard.lower);
      shard->set_upper(sm_shard.upper);
    }
  }

  return grpc::Status::OK;
}

grpc::Status ShardMaster::QueryConfigNum(grpc::ServerContext* context, const Empty* request, QueryConfigNumResponse* response) {
  std::cout << "* Shardmaster: Query config called - " << this->config_num << std::endl; // obviously comment afterwards
  
  response->set_config_num(this->config_num);
  return grpc::Status::OK;
};

grpc::Status ShardMaster::Move(grpc::ServerContext* context, const MoveRequest* request, Empty* response) {

  return grpc::Status::OK;
}

grpc::Status ShardMaster::Join(grpc::ServerContext* context, const JoinRequest* request, JoinResponse* response) { 

  int new_id = find_unassigned_vs_id();

  SMConfigEntry new_configEntry;
  new_configEntry.vs_addr = request->server_addr();
  new_configEntry.server_id = new_id;

  mtx.lock();
  sm_config.push_back(new_configEntry);
  mtx.unlock();

  redistributeChunks();
  
  return grpc::Status::OK;
}

grpc::Status ShardMaster::Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) {

  mtx.lock();
  for (int i = 0; i < sm_config.size(); ++i) {
    if (sm_config[i].vs_addr == request->server_addr()) {
      sm_config.erase(sm_config.begin() + i);
      break;
    }
  }
  mtx.unlock();

  redistributeChunks();

  return grpc::Status::OK;
}

void ShardMaster::redistributeChunks() {
  mtx.lock();
  config_num++;
  int num_vs = sm_config.size();

  for (int i = 0; i < num_vs; ++i) {
    sm_config[i].shards.clear();
    SMShard shard;
    shard.lower = (NUM_CHUNKS/num_vs) * i;

    if (i == num_vs - 1) {
      // in case NUM_CHUNKS % num_vs != 0 there can be unassigned chunks
      shard.upper = (NUM_CHUNKS);
    }else {
      shard.upper = (NUM_CHUNKS/num_vs) * (i + 1) - 1; 
    }
    sm_config[i].shards.push_back(shard);
  }
  mtx.unlock();
}

int ShardMaster::find_unassigned_vs_id() {
  mtx.lock();
  // min add is 1;
  int mex = 1;
  std::vector<int> ids;
  for (SMConfigEntry& configEntry : sm_config) {
    ids.push_back(configEntry.server_id);
  }
  sort(ids.begin(), ids.end());
  for (int& id : ids) {
    if (id != mex) 
      break;
    mex++;
  }
  mtx.unlock();
  return mex;
}




