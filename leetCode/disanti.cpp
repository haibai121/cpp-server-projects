#include <vector>
#include <set>
using namespace std;

class Solution {
public:
    int longestConsecutive(vector<int>& nums) {
        set<int> numbers(nums.begin(), nums.end());

        int current_length = 1;
        int max_length = 1;     

        auto current_it = numbers.begin();
        auto next_it = std::next(current_it);

        while (next_it != numbers.end()) {
            if (*next_it == *current_it + 1) {
                current_length++;
            } else {
                max_length = std::max(max_length, current_length);
                current_length = 1;
            }
            current_it = next_it;
            ++next_it;
        }
        max_length = std::max(max_length, current_length);

        return max_length;
    }
};