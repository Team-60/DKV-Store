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
  map<string, vector<SMShard>> m;

  assert(test_join(shardmaster_addr, skv_1, true));
  assert(test_join(shardmaster_addr, skv_2, true));
  assert(test_join(shardmaster_addr, skv_3, true));

  m[skv_1].push_back({0, 332});
  m[skv_2].push_back({333, 665});
  m[skv_3].push_back({666, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  assert(test_leave(shardmaster_addr, {skv_1}, true));
  m[skv_2].push_back({0, 499});
  m[skv_3].push_back({500, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  assert(test_join(shardmaster_addr, skv_1, true));
  m[skv_2].push_back({0, 332});
  m[skv_3].push_back({333, 665});
  m[skv_1].push_back({666, 1000});
  assert(test_query(shardmaster_addr, m));

  return 0;
}
