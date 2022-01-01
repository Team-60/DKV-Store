#include "volume_server.h"


grpc::Status VolumeServer::Get(grpc::ServerContext* context, const GetRequest* request, GetResponse* response) {
  std::cout << "Get : key=" << request->key() << std::endl;

  std::string value;
  leveldb::Status s = this->db->Get(leveldb::ReadOptions(), request->key(), &value);
  if (!s.ok()) {
    response->set_key("");
    response->set_value("");
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "key not found");
  }

  response->set_key(request->key());
  response->set_value(value);
  return grpc::Status::OK;
}


grpc::Status VolumeServer::Put(grpc::ServerContext* context, const PutRequest* request, google::protobuf::Empty* response) {
  std::cout << "Put : key=" << request->key() << " Value=" << request->value() << std::endl;

  std::string value;
  leveldb::Status s = this->db->Get(leveldb::ReadOptions(), request->key(), &value);
  if (s.ok()) {
    return grpc::Status(grpc::StatusCode::ALREADY_EXISTS, "key already exists");
  }

  this->db->Put(leveldb::WriteOptions(), request->key(), request->value());
  return grpc::Status::OK;
}

grpc::Status VolumeServer::Delete(grpc::ServerContext* context, const DeleteRequest* request, google::protobuf::Empty* response) {
  std::cout << "Delete : key=" << request->key() << " Value=" << request->value() << std::endl;

  std::string value;
  leveldb::Status s = this->db->Get(leveldb::ReadOptions(), request->key(), &value);
  if (!s.ok()) {
    return grpc::Status(grpc::StatusCode::NOT_FOUND, "key not found");
  }

  this->db->Delete(leveldb::WriteOptions(), request->key());
  return grpc::Status::OK;
}
