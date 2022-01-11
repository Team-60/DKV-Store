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
  vector<int> keys(3);
  for (int i = 0; i < 10000; ++i) {
    std::string hash = md5(std::to_string(i));
    uint hash_int = get_hash_uint(hash);
    uint shard_mod = hash_int % 1000;
    if (shard_mod >= 0 && shard_mod <= 200) {
      keys[0] = i;
    }else if (shard_mod >= 405 && shard_mod <= 667) {
      keys[1] = i;
    }else if (shard_mod >= 201 && shard_mod <= 404) {
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

  auto keys = getKeys();

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

  assert(test_put(skv_1, keys[0], "hello", true));
  assert(test_put(skv_1, keys[2], "wow!", true));
  assert(test_put(skv_2, keys[1], "hi", true));

  assert(test_get(skv_1, keys[0], "hello"));
  assert(test_get(skv_1, keys[2], "wow!"));
  assert(test_get(skv_2, keys[1], "hi"));

  // now we move keys onto skv_3
  assert(test_move(shardmaster_addr, skv_3, {201, 404}, true));

  m[skv_1].push_back({0, 200});
  m[skv_2].push_back({405, 665});
  m[skv_3].push_back({201, 404});
  m[skv_3].push_back({666, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // wait for the transfer
  std::this_thread::sleep_for(timespan);

  assert(test_get(skv_1, keys[0], "hello"));
  assert(test_get(skv_1, keys[2], nullopt));
  assert(test_get(skv_2, keys[1], "hi"));

  assert(test_get(skv_3, keys[2], "wow!"));
  assert(test_get(skv_3, keys[1], nullopt));
}
