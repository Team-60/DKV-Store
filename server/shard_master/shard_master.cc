#include "shard_master.h"

grpc::Status ShardMaster::Query(grpc::ServerContext* context, const Empty* request, QueryResponse* response) {
  return grpc::Status::OK;
}

grpc::Status ShardMaster::Move(grpc::ServerContext* context, const MoveRequest* request, Empty* response) {
  return grpc::Status::OK;
}

grpc::Status ShardMaster::Join(grpc::ServerContext* context, const JoinRequest* request, JoinResponse* response) {
  return grpc::Status::OK;
}

grpc::Status ShardMaster::Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) {
  return grpc::Status::OK;
}
  
