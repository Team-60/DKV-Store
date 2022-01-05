#include <unistd.h>
#include <cassert>
#include <map>
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
  string skv_3 = hostname + ":8083";
  string skv_4 = hostname + ":8084";
  string skv_5 = hostname + ":8085";

  map<string, vector<SMShard>> m;

  assert(test_join(shardmaster_addr, skv_1, true));
  assert(test_join(shardmaster_addr, skv_2, true));
  assert(test_join(shardmaster_addr, skv_3, true));
  assert(test_join(shardmaster_addr, skv_4, true));
  assert(test_join(shardmaster_addr, skv_5, true));

  m[skv_1].push_back({0, 200});
  m[skv_2].push_back({201, 400});
  m[skv_3].push_back({401, 600});
  m[skv_4].push_back({601, 800});
  m[skv_5].push_back({801, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // move a shard completely off a server
  assert(test_move(shardmaster_addr, skv_3, {0, 200}, true));
  m[skv_2].push_back({201, 400});
  m[skv_3].push_back({0, 200});
  m[skv_3].push_back({401, 600});
  m[skv_4].push_back({601, 800});
  m[skv_5].push_back({801, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // now move a shard that spans multiple servers
  assert(test_move(shardmaster_addr, skv_1, {150, 802}, true));
  m[skv_1].push_back({150, 802});
  m[skv_3].push_back({0, 149});
  m[skv_5].push_back({803, 1000});
  assert(test_query(shardmaster_addr, m));
  m.clear();

  // now if we have servers leave, we should rebalance
  assert(test_leave(shardmaster_addr, skv_1, true));
  assert(test_leave(shardmaster_addr, skv_2, true));
  assert(test_leave(shardmaster_addr, skv_5, true));
  m[skv_3].push_back({0, 500});
  m[skv_4].push_back({501, 1000});
  assert(test_query(shardmaster_addr, m));

  return 0;
}
