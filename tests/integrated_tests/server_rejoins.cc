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

vector<int> getKeys() {
  vector<int> keys(3, -1);
  for (int i = 0; i < 10000; ++i) {
    std::string hash = md5(std::to_string(i));
    uint hash_int = get_hash_uint(hash);
    uint shard_mod = hash_int % 1000;
    if (shard_mod >= 0 && shard_mod < 500 && keys[0] != -1) {
      keys[0] = i;
    }else if (shard_mod < 500) {
      keys[1] = i;
    }else {
      keys[2] = i;
    }
  }
  for (auto key : keys) {
    std::cout << key << " ";
  }
  std::cout << std::endl;
  return keys;
}

int main() {
  std::string hostname = "127.0.0.1";

  string shardmaster_addr = hostname + ":8080";

  vector<int> keys = getKeys();

  start_shardmaster(shardmaster_addr);

  string skv_1 = hostname + ":8081";
  string skv_2 = hostname + ":8082";

  map<string, vector<SMShard>> m;

  start_shardkv(skv_1, 1, shardmaster_addr);
  m[skv_1].push_back({0, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  start_shardkv(skv_2, 2, shardmaster_addr);
  m[skv_1].push_back({0, 499});
  m[skv_2].push_back({500, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // sleep to allow shardkvs to query and get initial config
  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);

  assert(test_put(skv_1, keys[0], "hello", true));
  assert(test_put(skv_1, keys[1], "wow!", true));
  assert(test_put(skv_2, keys[2], "hi", true));

  assert(test_get(skv_1, keys[0], "hello"));
  assert(test_get(skv_1, keys[1], "wow!"));
  assert(test_get(skv_2, keys[2], "hi"));

  // now when skv 1 leaves, its keys should transfer to skv 2
  assert(test_leave(shardmaster_addr, skv_1, true));

  // wait for the transfer
  std::this_thread::sleep_for(timespan);

  // keys not on skv_1
  assert(test_get(skv_1, keys[0], nullopt));
  assert(test_get(skv_1, keys[1], nullopt));

  // all data on skv_2
  assert(test_get(skv_2, keys[0], "hello"));
  assert(test_get(skv_2, keys[1], "wow!"));
  assert(test_get(skv_2, keys[2], "hi"));

  // now if skv 1 rejoins, we expect key keys[2] to end up on it
  assert(test_join(shardmaster_addr, skv_1, true));

  // wait for the transfer
  std::this_thread::sleep_for(timespan);

  assert(test_get(skv_2, keys[0], "hello"));
  assert(test_get(skv_2, keys[1], "wow!"));
  assert(test_get(skv_1, keys[2], "hi"));
}
