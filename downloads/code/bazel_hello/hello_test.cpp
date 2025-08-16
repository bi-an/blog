#include "hello.h"
#include <gtest/gtest.h>

TEST(HelloTest, Basic) {
    EXPECT_NO_THROW(say_hello());
}
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
