#include <unistd.h>

#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../test_utils/test_utils.h"
#include "utils.h"
#include "md5.h"

using namespace std;

int main() {
  std::string hostname = "127.0.0.1";

  string shardmaster_addr = hostname + ":8080";
  start_shardmaster(shardmaster_addr);

  string skv_1 = hostname + ":8081";
  string skv_2 = hostname + ":8082";

  map<string, vector<SMShard>> m;

  start_shardkv(skv_1, 1, shardmaster_addr);
  m[skv_1].push_back({0, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // sleep to allow shardkvs to query and get initial config
  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);

  assert(test_put(skv_1, 600, "hi", true));
  assert(test_put(skv_1, 100, "hello", true));
  assert(test_get(skv_1, 600, "hi"));
  assert(test_get(skv_1, 100, "hello"));

  // now when a new server joins, "hi" should move to it while "hello" stays
  start_shardkv(skv_2, 2, shardmaster_addr);
  m[skv_1].push_back({0, 499});
  m[skv_2].push_back({500, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // as hash of 600 >= 500, and hash of 100 < 500
  // wait for the key to transfer
  std::this_thread::sleep_for(timespan);
  //key should now be gone from skv_1 and on skv_2
  assert(test_get(skv_1, 600, nullopt));
  assert(test_get(skv_1, 100, "hello"));
  assert(test_get(skv_2, 600, "hi"));
}
