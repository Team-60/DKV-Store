#ifndef SHARDING_TEST_UTILS_H
#define SHARDING_TEST_UTILS_H

#include <grpcpp/grpcpp.h>
#include <unistd.h>
#include <sys/wait.h>
#include <cassert>
#include <optional>
#include <string>
#include <thread>
#include <vector>
#include <signal.h>
#include "utils.h"

using Addrs = std::vector<std::string>;

#define RETRIES 10

/**
 *
 * @tparam Service type of service we're starting
 * @tparam Args types of arguments to the ctor of Service
 * @param addr address we run our service on
 * @param args args to Service ctor
 *
 * This function runs a specified service on the specfied address - it is
 * blocking
 */
template <typename Service, typename... Args>
void spawn_service(const std::string& addr, Args&&... args) {
  ::grpc::ServerBuilder builder;
  builder.AddListeningPort(addr, ::grpc::InsecureServerCredentials());
  Service service(std::forward<Args>(args)...);
  builder.RegisterService(&service);
  std::unique_ptr<::grpc::Server> server = builder.BuildAndStart();
  std::cout << "* started on " << addr << std::endl;
  server->Wait();
}

// runs spawn_service in a detached thread so the caller doesn't block
template <typename Service, typename... Args>
void spawn_service_in_thread(const std::string& addr, Args&&... args) {
  std::thread thr(spawn_service<Service, Args...>, addr,
                  std::forward<Args>(args)...);
  thr.detach();
  // sleep to allow service to start
  std::chrono::milliseconds timespan(300);
  std::this_thread::sleep_for(timespan);
}

template <typename Service, typename... Args>
pid_t spawn_service_in_proc(const std::string& addr, Args&&... args) {
  pid_t pid = fork();
  assert(pid != -1);
  if (!pid) {
    spawn_service<Service, Args...>(addr, std::forward<Args>(args)...);
    exit(0);
  }
  return pid;
}

void start_simple_shardkvs(const Addrs& addrs);

void start_simple_shardkv(const std::string& addr, const uint db_idx);

void start_shardkvs(const Addrs& addrs, const std::string& shardmaster_addr);

std::vector<pid_t> start_shardkvs_proc(const Addrs& addrs,
                                       const std::string& shardmaster_addr);

void start_shardkv(const std::string& addr, const uint db_idx,
                   const std::string& shardmaster_addr);

pid_t start_shardkv_proc(const std::string& addr,
                         const std::string& shardmaster_addr);

void start_shardmaster(const std::string& addr);

// testing functions for simple shardkv and shardkv
bool test_get(const std::string& addr, unsigned int key,
              const std::optional<std::string>& value);

bool test_put(const std::string& addr, unsigned int key,
              const std::string& value, bool success);

bool test_delete(const std::string& addr, unsigned int key,
                 const std::string& value, bool success);

// testing functions for shardmaster
bool test_join(const std::string& shardmaster_addr, const std::string& addr,
               bool success);

bool test_leave(const std::string& shardmaster_addr, const std::string& addr,
                bool success);

bool test_move(const std::string& shardmaster_addr, const std::string& addr,
               const SMShard& shard, bool success);

bool test_query(const std::string& shardmaster_addr,
                const std::map<std::string, std::vector<SMShard>>& m);

void cleanup_children(const std::vector<pid_t>& pids);

#endif  // SHARDING_TEST_UTILS_H
