#include <string>
#include <vector>

struct SMShard {
  uint lower;
  uint upper;
};

struct SMConfigEntry {
  uint server_id;
  std::vector<SMShard> shards;
  std::string vs_addr;
};
