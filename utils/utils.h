#ifndef UTILS
#define UTILS

#include <iostream>
#include <string>
#include <vector>

// ---------------------------------- SHARD MASTER CONFIG UTILS ----------------------------------

struct SMShard {
  uint lower;
  uint upper;

  const std::vector<SMShard> subtract(const SMShard& a, const SMShard& b) {
    // a - b
    std::vector<SMShard> answer;

    if (a.upper < b.lower || b.upper < a.lower) {
      // no overlap - return just {a}
      answer.push_back(a);
      return answer;
    }

    if (a.lower >= b.lower && a.upper <= b.upper) {
      // total overlap - return {}
      return answer;
    }

    if (a.lower < b.lower && a.upper > b.upper) {
      // b is strictly inside a -- return two range results
      answer.push_back(SMShard{.lower = a.lower, .upper = b.lower - 1});
      answer.push_back(SMShard{.lower = b.upper + 1, .upper = a.upper});
      return answer;
    }

    if (a.lower < b.lower) {
      answer.push_back(SMShard{.lower = a.lower, .upper = b.lower - 1});
      return answer;
    }

    if (b.upper < a.upper) {
      answer.push_back(SMShard{.lower = b.upper + 1, .upper = a.upper});
      return answer;
    }

    assert(false);
  }

  bool operator==(const SMShard& rhs) const {
    return lower == rhs.lower && upper == rhs.upper;
  }
};

struct SMConfigEntry {
  std::vector<SMShard> shards;
  std::string vs_addr;
};

// ---------------------------------- HASH UTILS ----------------------------------

inline uint get_hash_uint(const std::string& hash) {
  // converts md5 hash to an unsigned int
  uint seg1 = std::stoul(hash.substr(0, 8), nullptr, 16);
  uint seg2 = std::stoul(hash.substr(8, 8), nullptr, 16);
  uint seg3 = std::stoul(hash.substr(16, 8), nullptr, 16);
  uint seg4 = std::stoul(hash.substr(24, 8), nullptr, 16);

  uint hash_int = seg1 ^ seg2 ^ seg3 ^ seg4;
  return hash_int;
}

#endif  // UTILS
