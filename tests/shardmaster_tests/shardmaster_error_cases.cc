#include <unistd.h>
#include <cassert>
#include <map>
#include <string>
#include <vector>

#include "../../test_utils/test_utils.h"

using namespace std;

int main() {
  std::string hostname = "127.0.0.1";

  string shardmaster_addr = hostname + ":8080";
  start_shardmaster(shardmaster_addr);

  string skv_1 = hostname + ":8081";
  string skv_2 = hostname + ":8082";
  string skv_3 = hostname + ":8083";
  string skv_4 = hostname + ":8084";
  map<string, vector<SMShard>> m;

  assert(test_join(shardmaster_addr, skv_1, true));
  m[skv_1].push_back({0, 1000});
  assert(test_query(shardmaster_addr, m));
  // duplicate join
  assert(test_join(shardmaster_addr, skv_1, false));
  assert(test_query(shardmaster_addr, m));

  // removing servers that don't exist
  assert(test_leave(shardmaster_addr, skv_2, false));
  assert(test_leave(shardmaster_addr, skv_3, false));
  assert(test_query(shardmaster_addr, m));

  // move that targets a server that doesn't exist
  assert(test_move(shardmaster_addr, skv_4, {50, 99}, false));
  assert(test_query(shardmaster_addr, m));

  return 0;
}
