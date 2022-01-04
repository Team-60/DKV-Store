#include <string>
#include <vector>

struct SMShard {
  int lower;
  int upper;

  const std::pair<SMShard, SMShard> subtract(const SMShard& a, const SMShard& b) {
    // a - b
    std::pair<SMShard, SMShard> answer;
    answer.first = {.lower= -1, .upper= -1};
    answer.second = answer.first;

    if (a.upper < b.lower || b.upper < a.lower) {
      // no overlap - return just {a, -1}
      answer.first = a;
      return answer;
    }

    if (a.lower >= b.lower && a.upper <= b.upper) {
      // total overlap - return {-1, -1}
      return answer;
    }
    
    if (a.lower < b.lower && a.upper > b.upper) {
      // b is strictly inside a -- return two range results
      answer.first = {.lower=a.lower, .upper=b.lower - 1};
      answer.second = {.lower=b.upper + 1, .upper=a.upper};
      return answer;
    }

    if (a.lower < b.lower) {
      answer.first = {.lower=a.lower, .upper=b.lower - 1};
      return answer;
    }

    if (b.upper < a.upper) {
      answer.first = {.lower=b.upper + 1, .upper=a.upper};
      return answer;
    }
    assert(false);
  }
};

struct SMConfigEntry {
  std::vector<SMShard> shards;
  std::string vs_addr;
};
