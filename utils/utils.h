#include <string>
#include <vector>

struct SMShard {
  uint lower;
  uint upper;
};

struct SMConfigEntry {
  std::vector<SMShard> shards;
  std::string vs_addr;
};