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
  return grpc::Status::OK;
}

grpc::Status ShardMaster::Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) {
  return grpc::Status::OK;
}


  
