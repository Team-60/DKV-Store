#include <unistd.h>
#include <cassert>
#include <optional>
#include <string>

#include "../../test_utils/test_utils.h"

using namespace std;

int main() {
  char hostnamebuf[256] = "127.0.0.1";
  gethostname(hostnamebuf, 256);
  string hostname(hostnamebuf);

  const string skv_1 = hostname + ":8081";
  const string skv_2 = hostname + ":8082";
  const string skv_3 = hostname + ":8083";

  start_simple_shardkvs({skv_1, skv_2, skv_3});

  assert(test_get(skv_1, 0, nullopt));
  assert(test_get(skv_2, 1, nullopt));
  assert(test_get(skv_3, 2, nullopt));

  assert(test_put(skv_1, 0, "cs15", true));
  assert(test_put(skv_2, 1, "cs17", true));
  assert(test_put(skv_3, 2, "cs19", true));

  assert(test_get(skv_1, 0, "cs15"));
  assert(test_get(skv_2, 1, "cs17"));
  assert(test_get(skv_3, 2, "cs19"));

  assert(test_get(skv_1, 30, nullopt));
  assert(test_get(skv_2, 34, nullopt));
  assert(test_get(skv_3, 41, nullopt));

  assert(test_append(skv_1, 30, "cs131", true));
  assert(test_append(skv_2, 34, "cs32", true));
  assert(test_append(skv_3, 41, "cs33", true));

  assert(test_get(skv_1, 30, "cs131"));
  assert(test_get(skv_2, 34, "cs32"));
  assert(test_get(skv_3, 41, "cs33"));

  return 0;
}
