#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include "include/lyrics.h"

using namespace std;
using namespace testing;

TEST(FakeTest, PleaseDeleteOnceYouWriteSome) {
  EXPECT_THAT(1, Eq(1));
}
TEST(CleanToken, PunctBothEnds) {
  EXPECT_THAT(cleanToken("...Love!!!"), StrEq("love"));
}

TEST(CleanToken, MiddlePunctOnly) {
  EXPECT_THAT(cleanToken("co-operate"), StrEq("co-operate"));
}

TEST(CleanToken, DigitsOnlyToEmpty) {
  EXPECT_THAT(cleanToken("12345"), StrEq(""));
}

TEST(CleanToken, MixedDigitsKeep) {
  EXPECT_THAT(cleanToken("1.abc.1"), StrEq("1.abc.1"));
}
