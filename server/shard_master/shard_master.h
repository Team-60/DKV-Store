#include <iostream>
#include <string>

#include <grpcpp/grpcpp.h>
#include "shard_master.grpc.pb.h"

using google::protobuf::Empty;

class ShardMaster final : public ShardMasterService::Service {

  public:
    ShardMaster () {
    }
    ~ShardMaster () {
    }

    grpc::Status Query(grpc::ServerContext* context, const Empty* request, QueryResponse* response) override;

    grpc::Status Move(grpc::ServerContext* context, const MoveRequest* request, Empty* response) override;

    grpc::Status Join(grpc::ServerContext* context, const JoinRequest* request, JoinResponse* response) override;

    grpc::Status Leave(grpc::ServerContext* context, const LeaveRequest* request, Empty* response) override;
  
};
