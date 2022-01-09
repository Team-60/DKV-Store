#include <unistd.h>

#include <cassert>
#include <map>
#include <optional>
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

  start_shardkvs({skv_1, skv_2}, shardmaster_addr);

  map<string, vector<SMShard>> m;

  assert(test_join(shardmaster_addr, skv_1, true));
  m[skv_1].push_back({0, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  assert(test_join(shardmaster_addr, skv_2, true));
  m[skv_1].push_back({0, 500});
  m[skv_2].push_back({501, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // sleep to allow shardkvs to query and get initial config
  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);

  assert(test_put(skv_1, 200, "hello", true));
  assert(test_put(skv_1, 202, "wow!", true));
  assert(test_put(skv_2, 600, "hi", true));

  assert(test_get(skv_1, 200, "hello"));
  assert(test_get(skv_1, 202, "wow!"));
  assert(test_get(skv_2, 600, "hi"));

  // now when skv 1 leaves, its keys should transfer to skv 2
  assert(test_leave(shardmaster_addr, skv_1, true));

  // wait for the transfer
  std::this_thread::sleep_for(timespan);

  // keys not on skv_1
  assert(test_get(skv_1, 200, nullopt));
  assert(test_get(skv_1, 202, nullopt));

  // all data on skv_2
  assert(test_get(skv_2, 200, "hello"));
  assert(test_get(skv_2, 202, "wow!"));
  assert(test_get(skv_2, 600, "hi"));

  // now if skv 1 rejoins, we expect key 600 to end up on it
  assert(test_join(shardmaster_addr, skv_1, true));

  // wait for the transfer
  std::this_thread::sleep_for(timespan);

  assert(test_get(skv_2, 200, "hello"));
  assert(test_get(skv_2, 202, "wow!"));
  assert(test_get(skv_1, 600, "hi"));
}
