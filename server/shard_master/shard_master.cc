#include "shard_master.h"

grpc::Status ShardMaster::Query(grpc::ServerContext* context, const Empty* request, QueryResponse* response) {
  std::cout << "* Shardmaster: Query called - " << this->config_num << std::endl;
  response->clear_config();

  // prepare response
  for (const SMConfigEntry& sm_config_entry : this->sm_config) {
    ConfigEntry* config_entry = response->add_config();
    config_entry->set_server_addr(sm_config_entry.vs_addr);
    config_entry->clear_shards();
    for (const SMShard& sm_shard : sm_config_entry.shards) {
      Shard* shard = config_entry->add_shards();
      shard->set_lower(sm_shard.lower);
      shard->set_upper(sm_shard.upper);
    }
  }

  return grpc::Status::OK;
}

grpc::Status ShardMaster::QueryConfigNum(grpc::ServerContext* context, const Empty* request, QueryConfigNumResponse* response) {
  // std::cout << "* Shardmaster: Query config called - " << this->config_num << std::endl;  // obviously comment afterwards

  response->set_config_num(this->config_num);
  return grpc::Status::OK;
};

grpc::Status ShardMaster::Move(grpc::ServerContext* context, const MoveRequest* request, Empty* response) {
  std::cout << "* Shardmaster: Move called - " << this->config_num << std::endl;

  bool serverExists = false;
  for (SMConfigEntry& config: this->sm_config) {
    if (config.vs_addr == request->server()) {
      serverExists = true;
      break;
    }
  }

  if (!serverExists) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "server doesn't exists");
  }

  auto move_vs_addr = request->server();
  auto shard = request->shard();
  SMShard move_shard;
  move_shard.lower = shard.lower(), move_shard.upper = shard.upper();

  this->mtx.lock();
  for (SMConfigEntry& config : this->sm_config) {
    std::vector<SMShard> new_shards;
    for (SMShard& shard : config.shards) {
      // shard - move_shard
      std::vector<SMShard> result = shard.subtract(shard, move_shard);
      if (result.size() > 0) {
        new_shards.push_back(result[0]);
      }
      if (result.size() > 1) {
        new_shards.push_back(result[1]);
      }
    }
    if (config.vs_addr == move_vs_addr) {
      new_shards.push_back(move_shard);
    }
    config.shards = new_shards;
  }
  this->config_num++;
  this->mtx.unlock();

  return grpc::Status::OK;
}

grpc::Status ShardMaster::Join(grpc::ServerContext* context, const JoinRequest* request, Empty* response) {
  std::cout << "* Shardmaster: Join called - " << this->config_num << "; vs_addr: " << request->server_addr() << std::endl;

  // check if already exists
  for (SMConfigEntry& config: this->sm_config) {
    if (config.vs_addr == request->server_addr()) {
      return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "server already exists");
    }
  }

  SMConfigEntry new_configEntry;
  new_configEntry.vs_addr = request->server_addr();

  this->mtx.lock();
  this->sm_config.push_back(new_configEntry);
  this->mtx.unlock();

  this->redistributeChunks();

  return grpc::Status::OK;
}

grpc::Status ShardMaster::Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) {
  std::cout << "* Shardmaster: Leave called - " << this->config_num << "; vs_addr: " << request->server_addr() << std::endl;

  bool serverExists = false;
  this->mtx.lock();
  for (int i = 0; i < this->sm_config.size(); ++i) {
    if (this->sm_config[i].vs_addr == request->server_addr()) {
      this->sm_config.erase(this->sm_config.begin() + i);
      serverExists = true;
      break;
    }
  }
  this->mtx.unlock();

  if (serverExists) {
    this->redistributeChunks();
  }else {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "server not found");
  }

  return grpc::Status::OK;
}

void ShardMaster::redistributeChunks() {
  this->mtx.lock();
  this->config_num++;
  int num_vs = this->sm_config.size();
  for (int i = 0; i < num_vs; ++i) {
    this->sm_config[i].shards.clear();
    SMShard shard;
    shard.lower = (this->NUM_CHUNKS / num_vs) * i;
    if (i == num_vs - 1) {
      // in case NUM_CHUNKS % num_vs != 0 there can be unassigned chunks
      shard.upper = this->NUM_CHUNKS;
    } else {
      shard.upper = (this->NUM_CHUNKS / num_vs) * (i + 1) - 1;
    }
    this->sm_config[i].shards.push_back(shard);
  }
  this->mtx.unlock();
}
