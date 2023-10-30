// Measure the distribution of the history hash function that
// hashes a 64-bit integer to a 16-bit integer.
#include <iostream>
#include <cstdint>
#include <array>
#include <random>

using namespace std;

auto hash_64bit (const uint64_t bits) -> uint16_t
{
    array<uint16_t, 4> parts;

    parts[0] = (bits >> 0ULL) & 0xffff;
    parts[1] = (bits >> 16ULL) & 0xffff;
    parts[2] = (bits >> 32ULL) & 0xffff;
    parts[3] = (bits >> 48ULL) & 0xffff;

    return parts[0] * 41 +
           parts[1] * 3 +
           parts[2] * 31 +
           parts[3] * 7;
}

array<int, 0x10000> counts;
array<int, 0x10000> diffs;

int main()
{
    random_device device;
    mt19937 gen(device());
    uniform_int_distribution<uint64_t> distro (1, 0xffffFFFFffffFFFFULL);

    const int trials = 40000000;

    for (int i = 0; i < trials; i++) {
        auto result = hash_64bit (distro(gen));
        counts[result]++;
    }

    int expected = trials / 0x10000;
    int threshold = trials / 100000;

    std::cout << "Expected for " << trials << " trials: " << expected << "\n";
    std::cout << "Anomalies at numbers greater than " << threshold << " differences:" << "\n";

    for (int i = 0; i < 0xffff; i++) {
        auto count = counts[i];
        auto diff = count - expected;
        if (abs(diff) > threshold) {
            std::cout << i << ":" << " count: " << count << " expected: " << expected
                             << " diff: " << diff << "\n";
        } else {
            std::cout << i << " count: " << count << "\n";
        }
    }
}
