#include <random>
using namespace std;

random_device rd;
mt19937 generator(rd());

int rng(int a, int b) {
    uniform_int_distribution<int> distribution(a, b);
    return distribution(generator);
}

void test_rng() {
    vector<int> nums;
    int n = rng(1, 10);
    for (int i = 0; i < n; i ++ ) {
        nums.push_back(rng(-1000, 1000));
    }
    printf("n:%d\n", n);
    for (auto x : nums) printf("%d ", x);
}
