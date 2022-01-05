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

  const string skv_1 = hostname + ":8080";
  const string skv_2 = hostname + ":8081";
  const string skv_3 = hostname + ":8082";

  start_simple_shardkvs({skv_1, skv_2, skv_3});

  assert(test_get(skv_1, 0, nullopt));
  assert(test_get(skv_2, 1, nullopt));
  assert(test_get(skv_3, 2, nullopt));

  assert(test_put(skv_3, 2, "abcd", true));
  assert(test_get(skv_1, 2, nullopt));
  assert(test_get(skv_2, 2, nullopt));
  assert(test_get(skv_3, 2, "abcd"));

  assert(test_put(skv_1, 0, "cs131!", true));
  assert(test_get(skv_1, 0, "cs131!"));

  assert(test_append(skv_2, 1, "huh?", true));
  assert(test_get(skv_2, 1, "huh?"));
  assert(test_put(skv_2, 1, "huh!", true));
  assert(test_get(skv_2, 1, "huh!"));
  assert(test_append(skv_2, 1, "?", true));
  assert(test_get(skv_2, 1, "huh!?"));

  return 0;
}
