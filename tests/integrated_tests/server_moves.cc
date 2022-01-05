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
  string skv_3 = hostname + ":8083";

  start_shardkvs({skv_1, skv_2, skv_3}, shardmaster_addr);

  map<string, vector<SMShard>> m;

  // sleep to allow shardkvs to query and get initial config
  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);

  assert(test_put(skv_1, 200, "hello", true));
  assert(test_put(skv_1, 202, "wow!", true));
  assert(test_put(skv_2, 404, "hi", true));

  assert(test_get(skv_1, 200, "hello"));
  assert(test_get(skv_1, 202, "wow!"));
  assert(test_get(skv_2, 404, "hi"));

  // now we move keys onto skv_3
  assert(test_move(shardmaster_addr, skv_3, {201, 404}, true));

  m[skv_1].push_back({0, 200});
  m[skv_2].push_back({405, 667});
  m[skv_3].push_back({201, 404});
  m[skv_3].push_back({668, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // wait for the transfer
  std::this_thread::sleep_for(timespan);

  assert(test_get(skv_1, 200, "hello"));
  assert(test_get(skv_1, 202, nullopt));
  assert(test_get(skv_2, 404, nullopt));

  assert(test_get(skv_3, 202, "wow!"));
  assert(test_get(skv_3, 404, "hi"));
}
