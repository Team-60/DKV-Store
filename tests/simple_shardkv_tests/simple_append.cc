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

  const string skv_addr = hostname + ":8080";
  start_simple_shardkv(skv_addr, 0);

  assert(test_get(skv_addr, 1, nullopt));
  assert(test_append(skv_addr, 1, "abcd", true));
  assert(test_get(skv_addr, 1, "abcd"));
  assert(test_append(skv_addr, 1, "!", true));
  assert(test_get(skv_addr, 1, "abcd!"));
  assert(test_put(skv_addr, 1, "BBBBB", true));
  assert(test_get(skv_addr, 1, "BBBBB"));

  return 0;
}
