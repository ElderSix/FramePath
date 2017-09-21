#include "testtool.hpp"

using namespace frame_path;

TEST(hello, world) {
    ASSERT_EQ(1, 2);
    ASSERT_LE(1, 2);
    ASSERT_LT(1, 2);
    ASSERT_GE(1, 2);
    ASSERT_GT(1, 2);
}

int main() {
    test::run_all_tests();
    return 0;
}