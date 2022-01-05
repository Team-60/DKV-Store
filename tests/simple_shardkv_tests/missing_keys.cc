#include <unistd.h>
#include <cassert>
#include <optional>
#include <string>

#include "../../test_utils/test_utils.h"

using namespace std;

int main() {
  std::string hostname = "127.0.0.1";

  string shardmaster_addr = hostname + ":8080";
  start_shardmaster(shardmaster_addr);

  const string skv_1 = hostname + ":8081";
  const string skv_2 = hostname + ":8082";
  const string skv_3 = hostname + ":8083";

  //start_simple_shardkvs({skv_1, skv_2, skv_3});
  start_simple_shardkv(skv_1, 1);

  assert(test_get(skv_1, 0, nullopt));
  assert(test_put(skv_1, 0, "cs15", true));
  assert(test_get(skv_1, 0, "cs15"));
  assert(test_delete(skv_1, 0, "cs15", true));
  assert(test_delete(skv_1, 0, "cs15", false));
  assert(test_put(skv_1, 30, "cs131", true));
  assert(test_get(skv_1, 30, "cs131"));

  assert(test_leave(shardmaster_addr, skv_1, true));

  std::chrono::milliseconds timespan(1000);
  std::this_thread::sleep_for(timespan);
  start_simple_shardkv(skv_2, 2);

  assert(test_get(skv_2, 1, nullopt));
  assert(test_put(skv_2, 1, "cs17", true));
  assert(test_get(skv_2, 1, "cs17"));
  assert(test_get(skv_2, 34, nullopt));
  assert(test_delete(skv_2, 34, "cs32", false));
  assert(test_get(skv_2, 34, nullopt));

  assert(test_leave(shardmaster_addr, skv_2, true));

  return 0;
}
