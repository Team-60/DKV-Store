#include <unistd.h>
#include <cassert>
#include <map>
#include <optional>
#include <string>
#include <vector>

#include "../../test_utils/test_utils.h"

using namespace std;

int main() {
  char hostnamebuf[256] = "127.0.0.1";
  gethostname(hostnamebuf, 256);
  string hostname(hostnamebuf);

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

  // sleep to allow shardkvs to query and get initial config
  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);

  assert(test_put(skv_1, 600, "hi", true));
  assert(test_put(skv_1, 200, "hello", true));
  assert(test_get(skv_1, 600, "hi"));
  assert(test_get(skv_1, 200, "hello"));

  // now when a new server joins, "hi" should move to it while "hello" stays
  assert(test_join(shardmaster_addr, skv_2, true));
  m[skv_1].push_back({0, 500});
  m[skv_2].push_back({501, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // wait for the key to transfer
  std::this_thread::sleep_for(timespan);
  // key should now be gone from skv_1 and on skv_2
  assert(test_get(skv_1, 600, nullopt));
  assert(test_get(skv_1, 200, "hello"));
  assert(test_get(skv_2, 600, "hi"));
}
