#include <unistd.h>

#include <cassert>
#include <optional>
#include <string>

#include "../../test_utils/test_utils.h"

using namespace std;

int main() {
  std::string hostname = "127.0.0.1";

  const string shardmaster_addr = hostname + ":8080";
  const string skv_1 = hostname + ":8081";
  const string skv_2 = hostname + ":8082";
  const string skv_3 = hostname + ":8083";

  start_shardmaster(shardmaster_addr);
  start_simple_shardkv(skv_1, 1);

  assert(test_get(skv_1, 0, nullopt));
  assert(test_get(skv_1, 2, nullopt));
  assert(test_put(skv_1, 0, "cs131!", true));
  assert(test_get(skv_1, 0, "cs131!"));

  assert(test_leave(shardmaster_addr, skv_1, true));

  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);
  start_simple_shardkv(skv_2, 2);

  assert(test_get(skv_2, 1, nullopt));
  assert(test_get(skv_2, 2, nullopt));
  assert(test_put(skv_2, 1, "huh?", true));
  assert(test_get(skv_2, 1, "huh?"));
  assert(test_put(skv_2, 1, "huh!", false));
  assert(test_get(skv_2, 1, "huh?"));
  assert(test_delete(skv_2, 1, "huh?", true));

  assert(test_leave(shardmaster_addr, skv_2, true));
  std::this_thread::sleep_for(timespan);
  start_simple_shardkv(skv_3, 3);

  assert(test_get(skv_3, 2, nullopt));
  assert(test_put(skv_3, 2, "abcd", true));
  assert(test_get(skv_3, 2, "abcd"));

  assert(test_leave(shardmaster_addr, skv_3, true));

  return 0;
}
