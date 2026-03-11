#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <map>
#include <set>
#include <string>

#include "include/lyrics.h"

using namespace std;
using namespace testing;

namespace {

map<string, set<string>> INDEX = {
    {"hello", {"Example", "School"}},
    {"there", {"Example"}},
    {"according", {"School"}},
    {"to", {"School"}},
    {"all", {"Example", "School", "Random"}},
    {"known", {"School"}},
    {"laws", {"Random"}},
    {"of", {"Random"}},
    {"aviation", {"Random"}},
    {"a", {"School", "Random"}},
};

TEST(FindQueryMatches, SingleTerm) {
  set<string> expected;

  expected = {"Example"};
  EXPECT_THAT(findQueryMatches(INDEX, "there"), ContainerEq(expected));

  expected = {"Example", "School"};
  EXPECT_THAT(findQueryMatches(INDEX, "hello"), ContainerEq(expected));
  EXPECT_THAT(findQueryMatches(INDEX, "Hello!"), ContainerEq(expected));
}

TEST(FindQueryMatches, Union) {
  set<string> expected;

  expected = {"School"};
  EXPECT_THAT(findQueryMatches(INDEX, "known to"), ContainerEq(expected));

  expected = {"Example", "School", "Random"};
  EXPECT_THAT(findQueryMatches(INDEX, "hello laws aviation"),
              ContainerEq(expected));
  EXPECT_THAT(findQueryMatches(INDEX, "Hello, Laws, AVIATION!"),
              ContainerEq(expected));
}

TEST(FindQueryMatches, Intersection) {
  set<string> expected;

  expected = {"Example"};
  EXPECT_THAT(findQueryMatches(INDEX, "all +there"), ContainerEq(expected));

  expected = {"Example", "School"};
  EXPECT_THAT(findQueryMatches(INDEX, "hello +all"), ContainerEq(expected));

  expected = {"School"};
  EXPECT_THAT(findQueryMatches(INDEX, "hello +all +to"), ContainerEq(expected));

  expected = {};
  EXPECT_THAT(findQueryMatches(INDEX, "hello +aviation"),
              ContainerEq(expected));
}

TEST(FindQueryMatches, Difference) {
  set<string> expected;

  expected = {"Example", "School"};
  EXPECT_THAT(findQueryMatches(INDEX, "all -laws"), ContainerEq(expected));

  expected = {"Random"};
  EXPECT_THAT(findQueryMatches(INDEX, "all -hello"), ContainerEq(expected));

  expected = {"Example"};
  EXPECT_THAT(findQueryMatches(INDEX, "all -of -a"), ContainerEq(expected));

  expected = {};
  EXPECT_THAT(findQueryMatches(INDEX, "known -to"), ContainerEq(expected));

  expected = {};
  EXPECT_THAT(findQueryMatches(INDEX, "to -all -a"), ContainerEq(expected));
}

TEST(FindQueryMatches, LongCombinedQueries) {
  set<string> expected;

  expected = {"Example"};
  EXPECT_THAT(findQueryMatches(INDEX, "there laws aviation -to +hello"),
              ContainerEq(expected));

  expected = {};
  EXPECT_THAT(findQueryMatches(INDEX, "all -all +all"), ContainerEq(expected));

  expected = {"Example", "School", "Random"};
  EXPECT_THAT(findQueryMatches(INDEX, "all -all +all all"),
              ContainerEq(expected));
}

}  // namespace
